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

/* License: Ruby's */
static struct ChildRecord {
  HANDLE hProcess;
  pid_t pid;
} ChildRecord[256];

/* License: Ruby's */
#define FOREACH_CHILD(v) do { \
  struct ChildRecord* v; \
  for (v = ChildRecord; v < ChildRecord + sizeof(ChildRecord) / sizeof(ChildRecord[0]); ++v)
#define END_FOREACH_CHILD } while (0)

static FARPROC get_proc_address(const char *module, const char *func, HANDLE *mh);
static struct ChildRecord *FindChildSlot(pid_t pid);

int
fork(void)
{
  return -1;
}

/* License: Ruby's */
pid_t
getppid(void)
{
  typedef long (WINAPI query_func)(HANDLE, int, void *, ULONG, ULONG *);
  static query_func *pNtQueryInformationProcess = (query_func *)-1;
  pid_t ppid = 0;

    if (pNtQueryInformationProcess == (query_func *)-1)
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
      if (!ret) {
        ppid = pbi.ParentProcessId;
      }
    }

    return ppid;
}

/* License: Artistic or GPL */
pid_t
waitpid(pid_t pid, int *stat_loc, int options)
{
  DWORD timeout;
  HANDLE hProc;
  struct ChildRecord* child;

  child = FindChildSlot(pid);
  if (child) {
    hProc = child->hProcess;
  }
  if (hProc == NULL || hProc == INVALID_HANDLE_VALUE) {
    return -1;
  }

  /* Artistic or GPL part start */
  if (options == WNOHANG) {
    timeout = 0;
  }
  else {
    timeout = INFINITE;
  }
  /* Artistic or GPL part end */

  return (WaitForSingleObject(hProc, timeout) == WAIT_OBJECT_0) ? pid : -1;
}

/* License: Ruby's */
int
kill(pid_t pid, int sig)
{
  pid_t ret = 0;
  DWORD ctrlEvent;
  HANDLE hProc;
  struct ChildRecord* child;

  if (pid < 0 || (pid == 0 && sig != SIGINT)) {
    return -1;
  }

  if ((unsigned int)pid == GetCurrentProcessId() &&
    (sig != 0 && sig != SIGKILL)) {
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
      if (pid != 0) {
        ctrlEvent = CTRL_BREAK_EVENT;
      }
      if (!GenerateConsoleCtrlEvent(ctrlEvent, (DWORD)pid)) {
        ret = -1;
      }
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
        DWORD status;
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
      break;
    }

    default:
      ret = -1;
  }

  return ret;
}

/* License: Ruby's */
static struct ChildRecord *
FindChildSlot(pid_t pid)
{
  FOREACH_CHILD(child) {
    if (child->pid == pid) {
      return child;
    }
  } END_FOREACH_CHILD;
  return NULL;
}

/* License: Ruby's */
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
spawnv(pid_t *pid, const char *path, char *const argv[])
{
  return 0; // TODO
}

int
spawnve(pid_t *pid, const char *path, char *const argv[], char *const envp[])
{
  return 0; // TODO
}
