/*
** process.c - 
**
** See Copyright Notice in mruby.h
*/

#include "mruby.h"
#include "mruby/array.h"
#include "mruby/class.h"
#include "mruby/string.h"
#include "mruby/variable.h"
#include "mruby/proc.h"
#include "error.h"

#include <sys/types.h>
#ifndef _WIN32
#include <sys/time.h>
#include <sys/wait.h>
#include <sys/select.h>
#else
#include <windows.h>
#endif
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#ifndef _MSC_VER
#include <unistd.h>
#endif
#include <errno.h>

#ifdef _WIN32
static const char*
emsg(DWORD err)
{
  static char buf[256];
  if (err == 0) return "succeeded";
  FormatMessage(
    FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
    NULL,
    err,
    MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US),
    buf,
    sizeof buf,
    NULL);
  return buf;
}

#ifndef SIGKILL
#define SIGKILL 9
#endif

int
kill(int pid, int sig)
{
  HANDLE handle;
  switch (sig) {
    case SIGTERM:
    case SIGKILL:
    case SIGINT:
      handle = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, pid);
      if (!TerminateProcess(handle, 1))
        return -1;
    default:
      return -1;
  }
  return 0;
}

unsigned int
sleep(unsigned int seconds)
{
  Sleep(seconds * 1000);
  return seconds;
}

#endif

mrb_value
mrb_f_kill(mrb_state *mrb, mrb_value klass)
{
  mrb_int pid;
  mrb_value *argv, sigo;
  int argc, sent, signo = 0;

  mrb_get_args(mrb, "oi*", &sigo, &pid, &argv, &argc);
  if (mrb_fixnum_p(sigo)) {
    signo = mrb_fixnum(sigo);
  } else {
    mrb_raisef(mrb, E_TYPE_ERROR, "bad signal type %s",
    	       mrb_obj_classname(mrb, sigo));
  }

  sent = 0;
  if (kill(pid, signo) == -1)
    mrb_sys_fail(mrb, "kill");
  sent++;

  while (argc-- > 0) {
    if (!mrb_fixnum_p(*argv)) {
      mrb_raisef(mrb, E_TYPE_ERROR, "wrong argument type %s (expected Fixnum)",
      	         mrb_obj_classname(mrb, *argv));
    }
    if (kill(mrb_fixnum(*argv), signo) == -1)
      mrb_sys_fail(mrb, "kill");
    sent++;
    argv++;
  }
  return mrb_fixnum_value(sent);
}

static mrb_value
mrb_f_fork(mrb_state *mrb, mrb_value klass)
{
#ifndef _WIN32
  mrb_value b, result;
  int pid;

  mrb_get_args(mrb, "&", &b);

  switch (pid = fork()) {
  case 0:
    if (!mrb_nil_p(b)) {
      mrb_yield(mrb, b, result);
      _exit(0);
    }
    return mrb_nil_value();

  case -1:
    mrb_sys_fail(mrb, "fork failed");
    return mrb_nil_value();

  default:
    return mrb_fixnum_value(pid);
  }
#else
   mrb_raise(mrb, E_RUNTIME_ERROR, emsg(ERROR_CALL_NOT_IMPLEMENTED));
   return mrb_nil_value();
#endif
}

static int
mrb_waitpid(int pid, int flags, int *st)
{
  int result;

#ifndef _WIN32
 retry:
  result = waitpid(pid, st, flags);
  if (result < 0) {
    if (errno == EINTR) {
      goto retry;
    }
    return -1;
  }
#else
  HANDLE handle = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, pid);
  result = WaitForSingleObject(handle, INFINITE) == WAIT_OBJECT_0 ? 0 : -1;
#endif

  return result;
}

static mrb_value
mrb_f_waitpid(mrb_state *mrb, mrb_value klass)
{
  mrb_int pid, flags = 0;
  int status;

  mrb_get_args(mrb, "i|i", &pid, &flags);

  if ((pid = mrb_waitpid(pid, flags, &status)) < 0)
    mrb_sys_fail(mrb, "waitpid failed");

  return mrb_fixnum_value(pid);
}

mrb_value
mrb_f_sleep(mrb_state *mrb, mrb_value klass)
{
  int argc;
  mrb_value *argv;
  time_t beg, end;

  beg = time(0);
  mrb_get_args(mrb, "*", &argv, &argc);
  if (argc == 0) {
    sleep((32767<<16)+32767);
  } else if(argc == 1) {
    struct timeval tv;
    int n;

    if (mrb_fixnum_p(argv[0])) {
      tv.tv_sec = mrb_fixnum(argv[0]);
      tv.tv_usec = 0;
    } else {
      tv.tv_sec = mrb_float(argv[0]);
      tv.tv_usec = (mrb_float(argv[0]) - tv.tv_sec) * 1000000.0;
    }

#ifdef _WIN32
    n = tv.tv_sec * 1000 + tv.tv_usec / 1000;
    Sleep(n);
#else
    n = select(0, 0, 0, 0, &tv);
#endif
    if (n < 0)
      mrb_sys_fail(mrb, "mrb_f_sleep failed");
  } else {
    mrb_raise(mrb, E_ARGUMENT_ERROR, "wrong # of arguments");
  }

  end = time(0) - beg;

  return mrb_fixnum_value(end);
}

#define RETSIGTYPE void

mrb_value
mrb_f_system(mrb_state *mrb, mrb_value klass)
{
  int ret;
  mrb_value *argv, pname;
  const char *path;
  int argc;
#ifdef SIGCHLD
  RETSIGTYPE (*chfunc)(int);
#endif

  fflush(stdout);
  fflush(stderr);

  mrb_get_args(mrb, "*", &argv, &argc);
  if (argc == 0) {
    mrb_raise(mrb, E_ARGUMENT_ERROR, "wrong number of arguments");
  }

  pname = argv[0];
#ifdef SIGCHLD
  chfunc = signal(SIGCHLD, SIG_DFL);
#endif
  path = mrb_string_value_cstr(mrb, &pname);
  ret = system(path);

#ifndef _WIN32
  if (WIFEXITED(ret) && WEXITSTATUS(ret) == 0) {
    return mrb_true_value();
  }
#else
  if (ret != -1)
    return mrb_true_value();
#endif

  return mrb_false_value();
}

mrb_value
mrb_f_exit(mrb_state *mrb, mrb_value klass)
{
  mrb_value status;
  int istatus;

  mrb_get_args(mrb, "|o", &status);
  if (!mrb_nil_p(status)) {
    if (mrb_type(status) == MRB_TT_TRUE)
      istatus = EXIT_SUCCESS;
    else {
      istatus = mrb_fixnum(status);
    }
  } else {
    istatus = EXIT_SUCCESS;
  }

  exit(istatus);
}

mrb_value
mrb_f_pid(mrb_state *mrb, mrb_value klass)
{
  return mrb_fixnum_value((mrb_int)getpid());
}

mrb_value
mrb_f_ppid(mrb_state *mrb, mrb_value klass)
{
#ifndef _WIN32
  return mrb_fixnum_value((mrb_int)getppid());
#else
  mrb_raise(mrb, E_RUNTIME_ERROR, emsg(ERROR_CALL_NOT_IMPLEMENTED));
  return mrb_nil_value();
#endif
}

void
mrb_mruby_process_win32_gem_init(mrb_state *mrb)
{
  struct RClass *p;

  mrb_define_method(mrb, mrb->kernel_module, "exit",   mrb_f_exit,   MRB_ARGS_OPT(1));
  mrb_define_method(mrb, mrb->kernel_module, "fork",   mrb_f_fork,   MRB_ARGS_NONE());
  mrb_define_method(mrb, mrb->kernel_module, "sleep",  mrb_f_sleep,  MRB_ARGS_ANY());
  mrb_define_method(mrb, mrb->kernel_module, "system", mrb_f_system, MRB_ARGS_ANY());

  p = mrb_define_module(mrb, "Process");
  mrb_define_class_method(mrb, p, "kill",    mrb_f_kill,    MRB_ARGS_ANY());
  mrb_define_class_method(mrb, p, "fork",    mrb_f_fork,    MRB_ARGS_NONE());
  mrb_define_class_method(mrb, p, "waitpid", mrb_f_waitpid, MRB_ARGS_ANY());
  mrb_define_class_method(mrb, p, "pid",     mrb_f_pid,     MRB_ARGS_NONE());
  mrb_define_class_method(mrb, p, "ppid",    mrb_f_ppid,    MRB_ARGS_NONE());

  mrb_gv_set(mrb, mrb_intern(mrb, "$$"), mrb_fixnum_value((mrb_int)getpid()));
}

void
mrb_mruby_process_win32_gem_final(mrb_state *mrb)
{
}
