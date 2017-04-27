/* MIT License
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifdef _WIN32



#include "mruby.h"
#include "mruby/array.h"
#include "mruby/class.h"
#include "mruby/string.h"
#include "mruby/variable.h"
#include "mruby/proc.h"
#include "error.h"

#include <sys/types.h>
#include <windows.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#ifndef _MSC_VER
#include <unistd.h>
#endif
#include <errno.h>







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
  mrb_raise(mrb, E_RUNTIME_ERROR, emsg(ERROR_CALL_NOT_IMPLEMENTED));
  return mrb_nil_value();
}

static int
mrb_waitpid(int pid, int flags, int *st)
{
  int result;

  HANDLE handle = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, pid);
  result = WaitForSingleObject(handle, INFINITE) == WAIT_OBJECT_0 ? 0 : -1;

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
    n = tv.tv_sec * 1000 + tv.tv_usec / 1000;

    Sleep(n);

    if (n < 0)
      mrb_sys_fail(mrb, "mrb_f_sleep failed");
  } else {
    mrb_raise(mrb, E_ARGUMENT_ERROR, "wrong # of arguments");
  }

  end = time(0) - beg;

  return mrb_fixnum_value(end);
}

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

  if (ret != -1)
    return mrb_true_value();

  return mrb_false_value();
}

mrb_value
mrb_f_exit(mrb_state *mrb, mrb_value klass)
{
  //TODO not win spec
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
mrb_f_exit_bang(mrb_state *mrb, mrb_value klass)
{
  //TODO
  return mrb_nil_value();
}

static mrb_value
mrb_f_exit_common(mrb_state *mrb, int bang)
{
  //TODO
  return mrb_nil_value();
}

mrb_value
mrb_f_pid(mrb_state *mrb, mrb_value klass)
{
  //TODO windows spec?
  return mrb_fixnum_value((mrb_int)getpid());
}

mrb_value
mrb_f_ppid(mrb_state *mrb, mrb_value klass)
{
  //TODO
  return mrb_nil_value();
}

static mrb_value
mrb_procstat_new(mrb_state *mrb, mrb_int pid, mrb_int status)
{
  //TODO
  return mrb_nil_value();
}

static mrb_value
mrb_procstat_coredump(mrb_state *mrb, mrb_value self)
{
  //TODO
  return mrb_nil_value();
}

static mrb_value
mrb_procstat_exitstatus(mrb_state *mrb, mrb_value self)
{
  //TODO
  return mrb_nil_value();
}

static mrb_value
mrb_procstat_exited(mrb_state *mrb, mrb_value self)
{
  //TODO
  return mrb_nil_value();
}

static mrb_value
mrb_procstat_signaled(mrb_state *mrb, mrb_value self)
{
  //TODO
  return mrb_nil_value();
}

static mrb_value
mrb_procstat_stopped(mrb_state *mrb, mrb_value self)
{
  //TODO
  return mrb_nil_value();
}

static mrb_value
mrb_procstat_stopsig(mrb_state *mrb, mrb_value self)
{
  //TODO
  return mrb_nil_value();
}

static mrb_value
mrb_procstat_termsig(mrb_state *mrb, mrb_value self)
{
  //TODO
  return mrb_nil_value();
}

#endif
