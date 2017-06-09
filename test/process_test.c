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

#if !defined(__APPLE__) && !defined(__linux__)

#include "mruby/string.h"
#include <windows.h>
#include <io.h>

static mrb_value
mrb_process_mock_sysopen(mrb_state *mrb, mrb_value self)
{
    mrb_int file_len, mod_len;
    char *file, *mod;
    HANDLE handle;

    mrb_get_args(mrb, "ss", &file, &file_len, &mod, &mod_len);

    handle = (HANDLE) _get_osfhandle(fileno(fopen(file, mod)));

    return mrb_cptr_value(mrb, handle);
}

#endif


void
mrb_mruby_process_gem_test(mrb_state* mrb)
{
#if !defined(__APPLE__) && !defined(__linux__)
    struct RClass *pt = mrb_define_module(mrb, "ProcessTest");
    mrb_define_class_method(mrb, pt, "sysopen", mrb_process_mock_sysopen, MRB_ARGS_REQ(2));
#endif
}
