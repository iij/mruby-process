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
#include "mruby/class.h"
#include "mruby/string.h"
#include "mruby/variable.h"
#include "mruby/error.h"

#include "process.h"
#include "signal.c"

static mrb_value mrb_f_exit_common(mrb_state *mrb, int bang);
static int mrb_waitpid(int pid, int flags, int *st);
static void mrb_process_set_childstat_gv(mrb_state *mrb, mrb_value childstat);
static void mrb_process_set_pid_gv(mrb_state *mrb);

mrb_value
mrb_f_exit(mrb_state *mrb, mrb_value klass)
{
  return mrb_f_exit_common(mrb, 0);
}

mrb_value
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
      status  = mrb_convert_type(mrb, status, MRB_TT_FIXNUM, "Integer", "to_i");
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

mrb_value
mrb_f_pid(mrb_state *mrb, mrb_value klass)
{
  return mrb_fixnum_value((mrb_int)getpid());
}

mrb_value
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
mrb_f_waitpid(mrb_state *mrb, mrb_value klass)
{
  mrb_int pid, flags;
  int status;

  mrb_get_args(mrb, "i|i", &pid, &flags);

  if ((pid = mrb_waitpid(pid, flags, &status)) < 0)
    mrb_sys_fail(mrb, "waitpid failed");

  if (!pid && (flags & WNOHANG)) {
    mrb_gv_set(mrb, mrb_intern_lit(mrb, "$?"), mrb_nil_value());
    return mrb_nil_value();
  }

  // mrb_gv_set(mrb, mrb_intern_lit(mrb, "$?"), mrb_procstat_new(mrb, pid, status));
  return mrb_fixnum_value(pid);
}

static int
mrb_waitpid(int pid, int flags, int *st)
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
  mrb_value b, result;
  int pid;

  mrb_get_args(mrb, "&", &b);

  switch (pid = fork()) {
  case 0:
    mrb_process_set_pid_gv(mrb);
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
}

static void
mrb_process_set_childstat_gv(mrb_state *mrb, mrb_value childstat)
{
  mrb_gv_set(mrb, mrb_intern_lit(mrb, "$?"), childstat);
  mrb_gv_set(mrb, mrb_intern_lit(mrb, "$CHILD_STATUS"), childstat);
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
  struct RClass *p, *ps, *s, *k;

  k = mrb->kernel_module;
  mrb_define_method(mrb, k, "exit",   mrb_f_exit,      MRB_ARGS_OPT(1));
  mrb_define_method(mrb, k, "exit!",  mrb_f_exit_bang, MRB_ARGS_OPT(1));
  mrb_define_method(mrb, k, "fork",   mrb_f_fork,      MRB_ARGS_NONE());
  // mrb_define_method(mrb, k, "system", mrb_f_system,    MRB_ARGS_ANY());

  s = mrb_define_module(mrb, "Signal");
  mrb_define_class_method(mrb, s, "signame", mrb_sig_signame, MRB_ARGS_REQ(1));
  mrb_define_class_method(mrb, s, "list",    mrb_sig_list,    MRB_ARGS_NONE());

  p = mrb_define_module(mrb, "Process");
  mrb_define_class_method(mrb, p, "kill",    mrb_f_kill,    MRB_ARGS_ANY());
  mrb_define_class_method(mrb, p, "fork",    mrb_f_fork,    MRB_ARGS_NONE());
  mrb_define_class_method(mrb, p, "waitpid", mrb_f_waitpid, MRB_ARGS_ANY());
  mrb_define_class_method(mrb, p, "pid",     mrb_f_pid,     MRB_ARGS_NONE());
  mrb_define_class_method(mrb, p, "ppid",    mrb_f_ppid,    MRB_ARGS_NONE());

  // ps = mrb_define_class_under(mrb, p, "Status", mrb->object_class);
  // mrb_define_method(mrb, ps, "coredump?", mrb_procstat_coredump, MRB_ARGS_NONE());
  // mrb_define_method(mrb, ps, "exited?", mrb_procstat_exited, MRB_ARGS_NONE());
  // mrb_define_method(mrb, ps, "exitstatus", mrb_procstat_exitstatus, MRB_ARGS_NONE());
  // mrb_define_method(mrb, ps, "signaled?", mrb_procstat_signaled, MRB_ARGS_NONE());
  // mrb_define_method(mrb, ps, "stopped?", mrb_procstat_stopped, MRB_ARGS_NONE());
  // mrb_define_method(mrb, ps, "stopsig", mrb_procstat_stopsig, MRB_ARGS_NONE());
  // mrb_define_method(mrb, ps, "termsig", mrb_procstat_termsig, MRB_ARGS_NONE());

  mrb_define_const(mrb, p, "WNOHANG",   mrb_fixnum_value(WNOHANG));
  mrb_define_const(mrb, p, "WUNTRACED", mrb_fixnum_value(WUNTRACED));

  mrb_process_set_childstat_gv(mrb, mrb_nil_value());
  mrb_process_set_pid_gv(mrb);
}

void
mrb_mruby_process_gem_final(mrb_state *mrb)
{

}
