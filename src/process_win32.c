/*
** process.c -
*/

#ifdef _WIN32

#include "mruby.h"

mrb_value
mrb_f_kill(mrb_state *mrb, mrb_value klass)
{
  return mrb_nil_value();
}

static mrb_value
mrb_f_fork(mrb_state *mrb, mrb_value klass)
{
  return mrb_nil_value();
}

static mrb_value
mrb_f_waitpid(mrb_state *mrb, mrb_value klass)
{
  return mrb_nil_value();
}

mrb_value
mrb_f_sleep(mrb_state *mrb, mrb_value klass)
{
  return mrb_nil_value();
}

mrb_value
mrb_f_system(mrb_state *mrb, mrb_value klass)
{
  return mrb_nil_value();
}

mrb_value
mrb_f_exit(mrb_state *mrb, mrb_value klass)
{
  return mrb_nil_value();
}

mrb_value
mrb_f_exit_bang(mrb_state *mrb, mrb_value klass)
{
  return mrb_nil_value();
}

static mrb_value
mrb_f_exit_common(mrb_state *mrb, int bang)
{
  return mrb_nil_value();
}

mrb_value
mrb_f_pid(mrb_state *mrb, mrb_value klass)
{
  return mrb_nil_value();
}

mrb_value
mrb_f_ppid(mrb_state *mrb, mrb_value klass)
{
  return mrb_nil_value();
}

static mrb_value
mrb_procstat_new(mrb_state *mrb, mrb_int pid, mrb_int status)
{
  return mrb_nil_value();
}

static mrb_value
mrb_procstat_coredump(mrb_state *mrb, mrb_value self)
{
return mrb_nil_value();
}

static mrb_value
mrb_procstat_exitstatus(mrb_state *mrb, mrb_value self)
{
  return mrb_nil_value();
}

static mrb_value
mrb_procstat_exited(mrb_state *mrb, mrb_value self)
{
  return mrb_nil_value();
}

static mrb_value
mrb_procstat_signaled(mrb_state *mrb, mrb_value self)
{
  return mrb_nil_value();
}

static mrb_value
mrb_procstat_stopped(mrb_state *mrb, mrb_value self)
{
  return mrb_nil_value();
}

static mrb_value
mrb_procstat_stopsig(mrb_state *mrb, mrb_value self)
{
  return mrb_nil_value();
}

static mrb_value
mrb_procstat_termsig(mrb_state *mrb, mrb_value self)
{
  return mrb_nil_value();
}

#endif
