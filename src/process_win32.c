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
