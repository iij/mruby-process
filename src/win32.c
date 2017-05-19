/* MIT License
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "mruby.h"
#include "mruby/string.h"
#include "process.h"

#include <windows.h>
#include <process.h>

#include <tchar.h>
#include <stdio.h>
//#include <strsafe.h>

#define BUFSIZE 4096

#define MAXCHILDNUM 256 /* max num of child processes */

#ifndef P_OVERLAY
# define P_OVERLAY 2
#endif

#ifndef P_NOWAIT
# define P_NOWAIT 1
#endif

/* License: Ruby's */
static struct ChildRecord {
    HANDLE hProcess;
    pid_t pid;
} ChildRecord[MAXCHILDNUM];

/* License: Ruby's */
#define FOREACH_CHILD(v) do { \
    struct ChildRecord* v; \
    for (v = ChildRecord; v < ChildRecord + sizeof(ChildRecord) / sizeof(ChildRecord[0]); ++v)
#define END_FOREACH_CHILD } while (0)

static FARPROC get_proc_address(const char *module, const char *func, HANDLE *mh);
static pid_t poll_child_status(struct ChildRecord *child, int *stat_loc);
static struct ChildRecord *FindChildSlot(pid_t pid);
static struct ChildRecord *FindChildSlotByHandle(HANDLE h);
static struct ChildRecord *FindFreeChildSlot(void);
static void CloseChildHandle(struct ChildRecord *child);
static struct ChildRecord *CreateChild(const WCHAR *cmd, const WCHAR *prog, SECURITY_ATTRIBUTES *psa, HANDLE hInput, HANDLE hOutput, HANDLE hError, DWORD dwCreationFlags, LPVOID environment);
static pid_t child_result(struct ChildRecord *child, int mode);
static char* argv_to_str(char* const* argv);
static WCHAR* str_to_wstr(const char *utf8, int mlen);

mrb_value
mrb_argv0(mrb_state *mrb)
{
    TCHAR argv0[MAX_PATH + 1];

    GetModuleFileName(NULL, argv0, MAX_PATH + 1);

    return mrb_str_new_cstr(mrb, argv0);
}

mrb_value
mrb_progname(mrb_state *mrb)
{
    TCHAR argv0[MAX_PATH + 1];
    char *progname;

    GetModuleFileName(NULL, argv0, MAX_PATH + 1);

    progname = strrchr(argv0, '\\');

    if (progname)
        progname++;
    else
        progname = argv0;

    return mrb_str_new_cstr(mrb, progname);
}

int
fork(void)
{
    return -1;
}

pid_t
getppid(void)
{
    typedef long (WINAPI query_func)(HANDLE, int, void *, ULONG, ULONG *);
    static query_func *pNtQueryInformationProcess = (query_func *) - 1;
    pid_t ppid = 0;

    if (pNtQueryInformationProcess == (query_func *) - 1)
        pNtQueryInformationProcess = (query_func *)get_proc_address("ntdll.dll", "NtQueryInformationProcess", NULL);

    if (pNtQueryInformationProcess) {
        struct {
            long ExitStatus;
            void* PebBaseAddress;
            uintptr_t AffinityMask;
            uintptr_t BasePriority;
            uintptr_t UniqueProcessId;
            uintptr_t ParentProcessId;
        } pbi;

        ULONG len;
        long ret = pNtQueryInformationProcess(GetCurrentProcess(), 0, &pbi, sizeof(pbi), &len);

        if (!ret)
            ppid = pbi.ParentProcessId;
    }

    return ppid;
}

pid_t
waitpid(pid_t pid, int *stat_loc, int options)
{
    DWORD timeout;
    struct ChildRecord* child;
    int count, retried, ret;

    /* Artistic or GPL part start */
    if (options == WNOHANG)
        timeout = 0;
    else
        timeout = INFINITE;
    /* Artistic or GPL part end */

    if (pid == -1) {
        HANDLE targets[MAXCHILDNUM];
        struct ChildRecord* cause;

        count = 0;

        FOREACH_CHILD(child) {
            if (!child->pid || child->pid < 0) continue;
            if ((pid = poll_child_status(child, stat_loc))) return pid;
            targets[count++] = child->hProcess;
        } END_FOREACH_CHILD;

        if (!count)
            return -1;

        ret = WaitForMultipleObjects(count, targets, FALSE, timeout);
        if (ret == WAIT_TIMEOUT) return 0;
        if ((ret -= WAIT_OBJECT_0) == count) return -1;
        if (ret > count) return -1;

        cause = FindChildSlotByHandle(targets[ret]);
        if (!cause) return -1;

        return poll_child_status(cause, stat_loc);
    }
    else {
        child   = FindChildSlot(pid);
        retried = 0;

        if (!child || child->hProcess == INVALID_HANDLE_VALUE)
            return -1;

        while (!(pid = poll_child_status(child, stat_loc))) {
            /* wait... */
            ret = WaitForMultipleObjects(1, &child->hProcess, FALSE, timeout);

            if (ret == WAIT_OBJECT_0 + 1) return -1; /* maybe EINTR */
            if (ret != WAIT_OBJECT_0) {
                /* still active */
                if (options & WNOHANG) {
                    pid = 0;
                    break;
                }
                ++retried;
            }
        }

        if (pid == -1 && retried) pid = 0;
    }

    return pid;
}

int
kill(pid_t pid, int sig)
{
    pid_t ret = 0;
    DWORD ctrlEvent, status;
    HANDLE hProc;
    struct ChildRecord* child;

    if (pid < 0 || (pid == 0 && sig != SIGINT))
        return -1;

    if ((unsigned int)pid == GetCurrentProcessId() && (sig != 0 && sig != SIGKILL)) {
        ret = raise(sig);
        return ret;
    }

    switch (sig) {
        case 0:
            hProc = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, (DWORD)pid);

            if (hProc == NULL || hProc == INVALID_HANDLE_VALUE) {
                ret = -1;
            }
            else {
                CloseHandle(hProc);
            }

            break;

        case SIGINT:
            ctrlEvent = CTRL_C_EVENT;

            if (pid != 0) ctrlEvent = CTRL_BREAK_EVENT;
            if (!GenerateConsoleCtrlEvent(ctrlEvent, (DWORD)pid)) ret = -1;

            break;

        case SIGKILL:
            child = FindChildSlot(pid);

            if (child) {
                hProc = child->hProcess;
            }
            else {
                hProc = OpenProcess(PROCESS_TERMINATE | PROCESS_QUERY_INFORMATION, FALSE, (DWORD)pid);
            }

            if (hProc == NULL || hProc == INVALID_HANDLE_VALUE) {
                ret = -1;
            }
            else {
                if (!GetExitCodeProcess(hProc, &status)) {
                    ret = -1;
                }
                else if (status == STILL_ACTIVE) {
                    if (!TerminateProcess(hProc, 0)) {
                        ret = -1;
                    }
                }
                else {
                    ret = -1;
                }

                if (!child) {
                    CloseHandle(hProc);
                }
            }

            break;

    default:
        ret = -1;
    }

    return ret;
}

void PrintStringArray( char *s[], size_t n )
{
    for ( char **p = s; p < s + n; ++p )
    {
        puts( *p );
    }
    printf( "\n" );
}


pid_t
spawnve(const char *shell, char *const argv[], char *const envp[])
{

  LPTSTR lpszCurrentVariable;

  TCHAR chNewEnv[BUFSIZE];

  HANDLE input, output, error;

  input = GetStdHandle(STD_INPUT_HANDLE);
  output = GetStdHandle(STD_OUTPUT_HANDLE);
  error = GetStdHandle(STD_ERROR_HANDLE);


  lpszCurrentVariable = (LPTSTR) chNewEnv;

  int i = 0;
  char* env = envp[i];

  while (env != NULL) {
    if (FAILED(strcpy(lpszCurrentVariable, TEXT(env))))
      {
        printf("env-string copy failed\n");
        return FALSE;
      }

      lpszCurrentVariable += lstrlen(lpszCurrentVariable) + 1;

      i++;
      env = envp[i];
  }



  // Terminate the block with a NULL byte.

  *lpszCurrentVariable = (TCHAR)0;

  // Create the child process, specifying a new environment block.

  // SecureZeroMemory(&si, sizeof(STARTUPINFO));
  // si.cb = sizeof(STARTUPINFO);
  //  TODO eventuell relevant für environment?
  // #ifdef UNICODE
  // dwFlags = CREATE_UNICODE_ENVIRONMENT;
  // #endif


  WCHAR *wcmd, *wshell;
  pid_t ret = -1;
  char *cmd = argv_to_str(argv);
  char tCmd[strlen(cmd)];
  char tShell[strlen(shell)];
  strcpy(tCmd,cmd);
  strcpy(tShell,shell);

  wshell = str_to_wstr(tShell, strlen(tShell));
  wcmd   = str_to_wstr(tCmd, strlen(tCmd));

  ret = child_result(CreateChild(wshell, wcmd, NULL, input, output, error, 0, (LPVOID) chNewEnv), P_NOWAIT);

  free(wshell);
  free(wcmd);
  free(cmd);

  return ret;
}

pid_t
spawnv(const char *shell, char *const argv[])
{
    WCHAR *wcmd, *wshell;
    pid_t ret = -1;
    char *cmd = argv_to_str(argv);
    char tCmd[strlen(cmd)];
    char tShell[strlen(shell)];
    strcpy(tCmd,cmd);
    strcpy(tShell,shell);
    HANDLE input, output, error;

    input = GetStdHandle(STD_INPUT_HANDLE);
    output = GetStdHandle(STD_OUTPUT_HANDLE);
    error = GetStdHandle(STD_ERROR_HANDLE);



    wshell = str_to_wstr(tShell, strlen(tShell));
    wcmd   = str_to_wstr(tCmd, strlen(tCmd));

    ret = child_result(CreateChild(wshell, wcmd, NULL, input, output, error, 0, NULL), P_NOWAIT);

    free(wshell);
    free(wcmd);
    free(cmd);

    return ret;
}

static FARPROC
get_proc_address(const char *module, const char *func, HANDLE *mh)
{
    HANDLE h;
    FARPROC ptr;

    if (mh)
      h = LoadLibrary(module);
    else
      h = GetModuleHandle(module);
    if (!h)
      return NULL;

    ptr = GetProcAddress(h, func);
    if (mh) {
      if (ptr)
        *mh = h;
      else
        FreeLibrary(h);
    }
    return ptr;
}

static struct ChildRecord *
CreateChild(const WCHAR *shell, const WCHAR *cmd, SECURITY_ATTRIBUTES *psa,
        HANDLE hInput, HANDLE hOutput, HANDLE hError, DWORD dwCreationFlags, LPVOID env)
{
    BOOL fRet;
    STARTUPINFOW aStartupInfo;
    PROCESS_INFORMATION aProcessInformation;
    SECURITY_ATTRIBUTES sa;
    struct ChildRecord *child;

    if (!cmd && !shell)
        return NULL;

    child = FindFreeChildSlot();

    if (!child)
        return NULL;

    if (!psa) {
        sa.nLength              = sizeof (SECURITY_ATTRIBUTES);
        sa.lpSecurityDescriptor = NULL;
        sa.bInheritHandle       = TRUE;
        psa = &sa;
    }

    memset(&aStartupInfo, 0, sizeof(aStartupInfo));
    memset(&aProcessInformation, 0, sizeof(aProcessInformation));

    aStartupInfo.cb      = sizeof(aStartupInfo);
    aStartupInfo.dwFlags = STARTF_USESTDHANDLES;

    if (hInput) {
       aStartupInfo.hStdInput  = hInput;
    }
    else {
       aStartupInfo.hStdInput  = GetStdHandle(STD_INPUT_HANDLE);
    }

    if (hOutput) {
       aStartupInfo.hStdOutput = hOutput;
    }
    else {
       aStartupInfo.hStdOutput = GetStdHandle(STD_OUTPUT_HANDLE);
    }

    if (hError) {
       aStartupInfo.hStdError = hError;
    }
    else {
       aStartupInfo.hStdError = GetStdHandle(STD_ERROR_HANDLE);
    }

    dwCreationFlags |= NORMAL_PRIORITY_CLASS;

    if (lstrlenW(cmd) > 32767) {
        child->pid = 0;     /* release the slot */
        return NULL;
    }

    fRet = CreateProcessW(shell, (WCHAR*) cmd, psa, psa,
                          psa->bInheritHandle, dwCreationFlags, env, NULL,
                          &aStartupInfo, &aProcessInformation);

    if (!fRet) {
        child->pid = 0;     /* release the slot */
        return NULL;
    }

    CloseHandle(aProcessInformation.hThread);

    child->hProcess = aProcessInformation.hProcess;
    child->pid      = (pid_t)aProcessInformation.dwProcessId;

    return child;
}

static pid_t
child_result(struct ChildRecord *child, int mode)
{
    DWORD exitcode;

    if (!child)
        return -1;

    if (mode == P_OVERLAY) {
        WaitForSingleObject(child->hProcess, INFINITE);
        GetExitCodeProcess(child->hProcess, &exitcode);
        CloseChildHandle(child);
        _exit(exitcode);
    }

    return child->pid;
}

static pid_t
poll_child_status(struct ChildRecord *child, int *stat_loc)
{
    DWORD exitcode;

    if (!GetExitCodeProcess(child->hProcess, &exitcode)) {
        /* If an error occurred, return immediately. */
    error_exit:
        CloseChildHandle(child);
        return -1;
    }

    if (exitcode != STILL_ACTIVE) {
        pid_t pid;

        /* If already died, wait process's real termination. */
        if (WaitForSingleObject(child->hProcess, INFINITE) != WAIT_OBJECT_0) {
            goto error_exit;
        }

        pid = child->pid;
        CloseChildHandle(child);

        if (stat_loc)
            *stat_loc = exitcode << 8;

        return pid;
    }

    return 0;
}

static struct ChildRecord *
FindChildSlot(pid_t pid)
{
    FOREACH_CHILD(child) {
        if (pid == -1 || child->pid == pid)
            return child;
    } END_FOREACH_CHILD;

    return NULL;
}

static struct ChildRecord *
FindChildSlotByHandle(HANDLE h)
{
    FOREACH_CHILD(child) {
        if (child->hProcess == h)
            return child;
    } END_FOREACH_CHILD;

    return NULL;
}

static struct ChildRecord *
FindFreeChildSlot(void)
{
    FOREACH_CHILD(child) {
        if (!child->pid) {
            child->pid      = -1;
            child->hProcess = NULL;

            return child;
        }
    } END_FOREACH_CHILD;

    return NULL;
}

static void
CloseChildHandle(struct ChildRecord *child)
{
    HANDLE h        = child->hProcess;
    child->hProcess = NULL;
    child->pid      = 0;

    CloseHandle(h);
}

static char*
argv_to_str(char* const* argv)
{
    char args[8191];
    int i     = 0;
    char* arg = argv[i];

    while (arg != NULL) {
        if (i == 0)
            sprintf(args, "%s", arg);
        else
            sprintf(args, "%s %s", args, arg);

        i++;
        arg = argv[i];
    }

    return strdup(args);
}

static WCHAR*
str_to_wstr(const char *utf8, int mlen)
{
    int wlen = MultiByteToWideChar(CP_UTF8, 0, utf8, mlen, NULL, 0);
    wchar_t* utf16 = (wchar_t*)malloc((wlen+1) * sizeof(wchar_t));

    if (utf16 == NULL)
        return NULL;

    if (MultiByteToWideChar(CP_UTF8, 0, utf8, mlen, utf16, wlen) > 0)
        utf16[wlen] = 0;

    return utf16;
}
