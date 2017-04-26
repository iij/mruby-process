/*
** process.c -
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
#include "process_win32.c"
#else
#include "process_posix.c"
#endif

void
mrb_mruby_process_gem_init(mrb_state *mrb)
{
  struct RClass *p, *s;

  mrb_define_method(mrb, mrb->kernel_module, "exit",   mrb_f_exit,   MRB_ARGS_OPT(1));
  mrb_define_method(mrb, mrb->kernel_module, "exit!", mrb_f_exit_bang, MRB_ARGS_OPT(1));
  mrb_define_method(mrb, mrb->kernel_module, "fork",   mrb_f_fork,   MRB_ARGS_NONE());
  mrb_define_method(mrb, mrb->kernel_module, "sleep",  mrb_f_sleep,  MRB_ARGS_ANY());
  mrb_define_method(mrb, mrb->kernel_module, "system", mrb_f_system, MRB_ARGS_ANY());

  p = mrb_define_module(mrb, "Process");
  mrb_define_class_method(mrb, p, "kill",    mrb_f_kill,    MRB_ARGS_ANY());
  mrb_define_class_method(mrb, p, "fork",    mrb_f_fork,    MRB_ARGS_NONE());
  mrb_define_class_method(mrb, p, "waitpid", mrb_f_waitpid, MRB_ARGS_ANY());
  mrb_define_class_method(mrb, p, "pid",     mrb_f_pid,     MRB_ARGS_NONE());
  mrb_define_class_method(mrb, p, "ppid",    mrb_f_ppid,    MRB_ARGS_NONE());

  s = mrb_define_class_under(mrb, p, "Status", mrb->object_class);
  mrb_define_method(mrb, s, "coredump?", mrb_procstat_coredump, MRB_ARGS_NONE());
  mrb_define_method(mrb, s, "exited?", mrb_procstat_exited, MRB_ARGS_NONE());
  mrb_define_method(mrb, s, "exitstatus", mrb_procstat_exitstatus, MRB_ARGS_NONE());
  mrb_define_method(mrb, s, "signaled?", mrb_procstat_signaled, MRB_ARGS_NONE());
  mrb_define_method(mrb, s, "stopped?", mrb_procstat_stopped, MRB_ARGS_NONE());
  mrb_define_method(mrb, s, "stopsig", mrb_procstat_stopsig, MRB_ARGS_NONE());
  mrb_define_method(mrb, s, "termsig", mrb_procstat_termsig, MRB_ARGS_NONE());

  // mrb_define_const(mrb, p, "WNOHANG", mrb_fixnum_value(WNOHANG));
  // mrb_define_const(mrb, p, "WUNTRACED", mrb_fixnum_value(WUNTRACED));

  mrb_gv_set(mrb, mrb_intern_lit(mrb, "$$"), mrb_fixnum_value((mrb_int)getpid()));
  mrb_gv_set(mrb, mrb_intern_lit(mrb, "$?"), mrb_nil_value());
}

void
mrb_mruby_process_gem_final(mrb_state *mrb)
{
}
