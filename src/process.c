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

#include "process.h"

#include "mruby.h"
#include "mruby/variable.h"
#include "mruby/array.h"
#include "mruby/error.h"

#include "internal.c"
#include "status.c"
#include "signal.c"

static mrb_value mrb_f_exit_common(mrb_state *mrb, int bang);
static int mrb_waitpid(int pid, int *st, int flags);
static void mrb_process_set_pid_gv(mrb_state *mrb);

static mrb_value
mrb_proc_argv0(mrb_state *mrb, mrb_value klass)
{
  return mrb_argv0(mrb);
}

static mrb_value
mrb_proc_progname(mrb_state *mrb)
{
  return mrb_funcall(mrb, mrb_progname(mrb), "freeze", 0);
}

static mrb_value
mrb_f_abort(mrb_state *mrb, mrb_value klass)
{
  mrb_value error;
  int n;

  n = mrb_get_args(mrb, "|S", &error);

  if (n != 0) {
    fprintf(stderr, "%s\n", mrb_str_to_cstr(mrb, error));
  }

  return mrb_f_exit_common(mrb, 1);
}

static mrb_value
mrb_f_exit(mrb_state *mrb, mrb_value klass)
{
  return mrb_f_exit_common(mrb, 0);
}

static mrb_value
mrb_f_exit_bang(mrb_state *mrb, mrb_value klass)
{
  return mrb_f_exit_common(mrb, 1);
}

static mrb_value
mrb_f_exit_common(mrb_state *mrb, int bang)
{
  mrb_value status;
  int istatus, n;

  n = mrb_get_args(mrb, "|o", &status);
  if (n == 0) {
    status = (bang) ? mrb_false_value() : mrb_true_value();
  }

  switch (mrb_type(status)) {
    case MRB_TT_TRUE:
      istatus = EXIT_SUCCESS;
      break;

    case MRB_TT_FALSE:
      istatus = EXIT_FAILURE;
      break;

    default:
      status  = mrb_to_int(mrb, status);
      istatus = mrb_fixnum(status);
  }

  if (bang) {
    _exit(istatus);
  } else {
    exit(istatus);
  }

  /* maybe not reached */
  return mrb_nil_value();
}

static mrb_value
mrb_f_pid(mrb_state *mrb, mrb_value klass)
{
  return mrb_fixnum_value((mrb_int)getpid());
}

static mrb_value
mrb_f_ppid(mrb_state *mrb, mrb_value klass)
{
  return mrb_fixnum_value((mrb_int)getppid());
}

static mrb_value
mrb_f_kill(mrb_state *mrb, mrb_value klass)
{
  mrb_int pid, argc;
  mrb_value *argv, sig;
  int signo, sent;
  const char *signm;

  mrb_get_args(mrb, "oi*", &sig, &pid, &argv, &argc);

  switch (mrb_type(sig)) {
    case MRB_TT_FIXNUM:
      signo = mrb_fixnum(sig);
      signm = signo2signm(signo);
      break;

    case MRB_TT_STRING:
      signm = RSTRING_PTR(sig);
      signo = signm2signo(signm);
      break;

    case MRB_TT_SYMBOL:
      signm = mrb_sym2name(mrb, mrb_symbol(sig));
      signo = signm2signo(signm);
      break;

    default:
      mrb_raisef(mrb, E_TYPE_ERROR, "bad signal type %S",
                 mrb_obj_value(mrb_class(mrb, sig)));
  }

  if (!signm) {
    mrb_raisef(mrb, E_ARGUMENT_ERROR, "unsupported signal %S",
               mrb_fixnum_value(signo));
  }

  if (strncmp(signame_prefix, signm, sizeof(signame_prefix)) == 0)
    signm += 3;

  if (strcmp(signm, signo2signm(signo)) > 0) {
    mrb_raisef(mrb, E_ARGUMENT_ERROR, "unsupported signal name `SIG%S'",
               mrb_str_new_cstr(mrb, signm));
  }

  sent = 0;
  if (kill(pid, signo) == -1)
    mrb_sys_fail(mrb, "no such process");
  sent++;

  while (argc-- > 0) {
    if (!mrb_fixnum_p(*argv)) {
      mrb_raisef(mrb, E_TYPE_ERROR, "no implicit conversion of %S into Integer",
                 mrb_obj_value(mrb_class(mrb, *argv)));
    }

    if (kill(mrb_fixnum(*argv), signo) == -1)
      mrb_sys_fail(mrb, "no such process");

    sent++;
    argv++;
  }

  return mrb_fixnum_value(sent);
}

static mrb_value
mrb_f_spawn(mrb_state *mrb, mrb_value klass)
{
  struct mrb_execarg *eargp = mrb_execarg_new(mrb);
  int pid;


  // if (eargp->envp)
    //execve(eargp->filename, eargp->argv, eargp->envp);
  // else
    pid = spawnv(P_NOWAIT, eargp->filename, eargp->argv);

  if (pid == -1)
    mrb_sys_fail(mrb, "spawn failed");

  return mrb_fixnum_value(pid);
}

static mrb_value
mrb_f_wait(mrb_state *mrb, mrb_value klass)
{
  mrb_int pid, flags;
  int len, status;

  len = mrb_get_args(mrb, "|ii", &pid, &flags);

  if (len == 0) pid   = -1;
  if (len == 1) flags = 0;

  if ((pid = mrb_waitpid(pid, &status, flags)) < 0)
    mrb_sys_fail(mrb, "waitpid failed");
    
  if (!pid && (flags & WNOHANG)) {
    mrb_last_status_clear(mrb);
    return mrb_nil_value();
  }

  mrb_last_status_set(mrb, pid, status);
  return mrb_fixnum_value(pid);
}

static mrb_value
mrb_f_wait2(mrb_state *mrb, mrb_value klass)
{
  mrb_value pid = mrb_f_wait(mrb, klass);
  mrb_value st  = mrb_last_status_get(mrb);

  return mrb_assoc_new(mrb, pid, st);
}

static mrb_value
mrb_f_waitall(mrb_state *mrb, mrb_value klass)
{
  mrb_value result, st;
  pid_t pid;
  int status;

  result = mrb_ary_new(mrb);
  mrb_last_status_clear(mrb);

  for (pid = -1;;) {
    pid = mrb_waitpid(-1, &status, 0);

    if (pid == -1) {
      int e = errno;

      if (e == ECHILD)
        break;

      mrb_sys_fail(mrb, "waitall failed");
    }

    if (!pid)
      mrb_last_status_clear(mrb);
    else
      mrb_last_status_set(mrb, pid, status);

    st = mrb_last_status_get(mrb);
    mrb_ary_push(mrb, result, mrb_assoc_new(mrb, mrb_fixnum_value(pid), st));
  }

  return result;
}

static int
mrb_waitpid(int pid, int *st, int flags)
{
  int result;

retry:
  result = waitpid(pid, st, flags);
  if (result < 0) {
    if (errno == EINTR) {
      goto retry;
    }
    return -1;
  }

  return result;
}

static mrb_value
mrb_f_fork(mrb_state *mrb, mrb_value klass)
{
  mrb_value b;
  int pid;

  mrb_get_args(mrb, "&", &b);

  switch (pid = fork()) {
  case 0:
    mrb_process_set_pid_gv(mrb);
    if (!mrb_nil_p(b)) {
      mrb_yield(mrb, b, mrb_nil_value());
      _exit(0);
    }
    return mrb_nil_value();

  case -1:
    mrb_sys_fail(mrb, "fork failed");
    return mrb_nil_value();

  default:
    return mrb_fixnum_value(pid);
  }
}

static inline mrb_value
mrb_f_exec(mrb_state *mrb, mrb_value klass)
{


  free(eargp);
  mrb_sys_fail(mrb, "exec failed");

  return mrb_nil_value();
}

static void
mrb_process_set_pid_gv(mrb_state *mrb)
{
  mrb_value pid = mrb_fixnum_value((mrb_int)getpid());

  mrb_gv_set(mrb, mrb_intern_lit(mrb, "$$"), pid);
  mrb_gv_set(mrb, mrb_intern_lit(mrb, "$PID"), pid);
  mrb_gv_set(mrb, mrb_intern_lit(mrb, "$PROCESS_ID"), pid);
}

void
mrb_mruby_process_gem_init(mrb_state *mrb)
{
  struct RClass *p, *k;

  k = mrb->kernel_module;
  mrb_define_method(mrb, k, "abort", mrb_f_abort, MRB_ARGS_OPT(1));
  mrb_define_method(mrb, k, "exit",  mrb_f_exit,  MRB_ARGS_OPT(1));
  mrb_define_method(mrb, k, "exit!", mrb_f_exit_bang, MRB_ARGS_OPT(1));
  mrb_define_method(mrb, k, "exec",  mrb_f_exec,  MRB_ARGS_REQ(1)|MRB_ARGS_REST());
  mrb_define_method(mrb, k, "spawn",  mrb_f_spawn,  MRB_ARGS_REQ(1)|MRB_ARGS_REST());

  p = mrb_define_module(mrb, "Process");
  mrb_define_class_method(mrb, p, "argv0",    mrb_proc_argv0, MRB_ARGS_NONE());
  mrb_define_class_method(mrb, p, "abort",    mrb_f_abort,   MRB_ARGS_OPT(1));
  mrb_define_class_method(mrb, p, "exit",     mrb_f_exit,    MRB_ARGS_OPT(1));
  mrb_define_class_method(mrb, p, "exit!",    mrb_f_exit_bang, MRB_ARGS_OPT(1));
  mrb_define_class_method(mrb, p, "kill",     mrb_f_kill,    MRB_ARGS_REQ(2)|MRB_ARGS_REST());
  mrb_define_class_method(mrb, p, "exec",     mrb_f_exec,    MRB_ARGS_REQ(1)|MRB_ARGS_REST());
  mrb_define_class_method(mrb, p, "waitpid",  mrb_f_wait,    MRB_ARGS_OPT(2));
  mrb_define_class_method(mrb, p, "waitpid2", mrb_f_wait2,   MRB_ARGS_OPT(2));
  mrb_define_class_method(mrb, p, "wait",     mrb_f_wait,    MRB_ARGS_OPT(2));
  mrb_define_class_method(mrb, p, "wait2",    mrb_f_wait2,   MRB_ARGS_OPT(2));
  mrb_define_class_method(mrb, p, "waitall",  mrb_f_waitall, MRB_ARGS_NONE());
  mrb_define_class_method(mrb, p, "pid",      mrb_f_pid,     MRB_ARGS_NONE());
  mrb_define_class_method(mrb, p, "ppid",     mrb_f_ppid,    MRB_ARGS_NONE());
  mrb_define_class_method(mrb, p, "spawn",    mrb_f_spawn,    MRB_ARGS_REQ(1)|MRB_ARGS_REST());

#ifndef _WIN32
  mrb_define_method(mrb, k,       "fork",     mrb_f_fork,    MRB_ARGS_BLOCK());
  mrb_define_class_method(mrb, p, "fork",     mrb_f_fork,    MRB_ARGS_BLOCK());
#endif

#if defined(__APPLE__) || defined(__linux__)
  mrb_define_method(mrb, k,       "fork",     mrb_f_fork,    MRB_ARGS_BLOCK());
  mrb_define_class_method(mrb, p, "fork",     mrb_f_fork,    MRB_ARGS_BLOCK());
#endif

  mrb_define_const(mrb, p, "WNOHANG",   mrb_fixnum_value(WNOHANG));
  mrb_define_const(mrb, p, "WUNTRACED", mrb_fixnum_value(WUNTRACED));

  mrb_process_set_pid_gv(mrb);
  mrb_gv_set(mrb, mrb_intern_lit(mrb, "$0"),            mrb_proc_progname(mrb));
  mrb_gv_set(mrb, mrb_intern_lit(mrb, "$PROGRAM_NAME"), mrb_proc_progname(mrb));

  mrb_mruby_process_gem_signal_init(mrb);
  mrb_mruby_process_gem_procstat_init(mrb);
}

void
mrb_mruby_process_gem_final(mrb_state *mrb)
{

}
