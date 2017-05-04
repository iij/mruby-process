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
#include "mruby/variable.h"

#include "process.h"

static mrb_value
mrb_pst_new(mrb_state *mrb, pid_t pid, mrb_int status)
{
    struct RClass *p, *s;
    p = mrb_module_get(mrb, "Process");
    s = mrb_class_get_under(mrb, p, "Status");

    return mrb_funcall(mrb, mrb_obj_value(s), "new", 2,
                       mrb_fixnum_value(pid), mrb_fixnum_value(status));
}

static int
mrb_pst_last_status_get(mrb_state *mrb, mrb_value self)
{
    return mrb_fixnum(mrb_iv_get(mrb, self, mrb_intern_lit(mrb, "@status")));
}

static void
mrb_pst_last_status_set(mrb_state *mrb, mrb_value pst)
{
  mrb_gv_set(mrb, mrb_intern_lit(mrb, "$?"), pst);
  mrb_gv_set(mrb, mrb_intern_lit(mrb, "$CHILD_STATUS"), pst);
}

void
mrb_last_status_set(mrb_state *mrb, pid_t pid, mrb_int status)
{
    mrb_pst_last_status_set(mrb, mrb_pst_new(mrb, pid, status));
}

mrb_value
mrb_last_status_get(mrb_state *mrb)
{
    return mrb_gv_get(mrb, mrb_intern_lit(mrb, "$?"));
}

void
mrb_last_status_clear(mrb_state *mrb)
{
    return mrb_pst_last_status_set(mrb, mrb_nil_value());
}

static mrb_value
mrb_pst_wcoredump(mrb_state *mrb, mrb_value self)
{
#ifdef WCOREDUMP
    int i = mrb_pst_last_status_get(mrb, self);
    return mrb_bool_value(WCOREDUMP(i));
#else
    return mrb_false_value();
#endif
}

static mrb_value
mrb_pst_wexitstatus(mrb_state *mrb, mrb_value self)
{
    int i = mrb_pst_last_status_get(mrb, self);

    if (WIFEXITED(i))
        return mrb_fixnum_value(WEXITSTATUS(i));

    return mrb_nil_value();
}

static mrb_value
mrb_pst_wifexited(mrb_state *mrb, mrb_value self)
{
    int i = mrb_pst_last_status_get(mrb, self);

    return mrb_bool_value(WIFEXITED(i));
}

static mrb_value
mrb_pst_wifsignaled(mrb_state *mrb, mrb_value self)
{
    int i = mrb_pst_last_status_get(mrb, self);

    return mrb_bool_value(WIFSIGNALED(i));
}

static mrb_value
mrb_pst_wifstopped(mrb_state *mrb, mrb_value self)
{
    int i = mrb_pst_last_status_get(mrb, self);

    return mrb_bool_value(WIFSTOPPED(i));
}

static mrb_value
mrb_pst_wstopsig(mrb_state *mrb, mrb_value self)
{
    int i = mrb_pst_last_status_get(mrb, self);

    if (WIFSTOPPED(i))
        return mrb_fixnum_value(WSTOPSIG(i));

    return mrb_nil_value();
}

static mrb_value
mrb_pst_wtermsig(mrb_state *mrb, mrb_value self)
{
    int i = mrb_pst_last_status_get(mrb, self);

    if (WIFSIGNALED(i)) {
        return mrb_fixnum_value(WTERMSIG(i));
    } else {
        return mrb_nil_value();
    }
}

void
mrb_mruby_process_gem_procstat_init(mrb_state *mrb)
{
    struct RClass *p, *s;

    p = mrb_module_get(mrb, "Process");
    s = mrb_define_class_under(mrb, p, "Status", mrb->object_class);

    mrb_define_method(mrb, s, "coredump?",  mrb_pst_wcoredump, MRB_ARGS_NONE());
    mrb_define_method(mrb, s, "exited?",    mrb_pst_wifexited, MRB_ARGS_NONE());
    mrb_define_method(mrb, s, "exitstatus", mrb_pst_wexitstatus, MRB_ARGS_NONE());
    mrb_define_method(mrb, s, "signaled?",  mrb_pst_wifsignaled, MRB_ARGS_NONE());
    mrb_define_method(mrb, s, "stopped?",   mrb_pst_wifstopped, MRB_ARGS_NONE());
    mrb_define_method(mrb, s, "stopsig",    mrb_pst_wstopsig, MRB_ARGS_NONE());
    mrb_define_method(mrb, s, "termsig",    mrb_pst_wtermsig, MRB_ARGS_NONE());

    mrb_last_status_clear(mrb);
}
