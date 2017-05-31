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

#if !defined(__APPLE__) && !defined(__linux__)

#include <sys/types.h>
#include <errno.h>
#include <unistd.h>

#include <winsock.h>
#include <io.h>

#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>


static mrb_value
mrb_io_test_sysopen(mrb_state *mrb, mrb_value self)
{
  mrb_value toReturn;
  DIR* dir = opendir("tmp");
  // FILE *fp;
  mrb_value file;
  HANDLE foo;

  if (dir)
  {
      /* Directory exists. */
      closedir(dir);
  }
  else if (ENOENT == errno)
  {
      mkdir("tmp");
  }
  else
  {
      /* opendir() failed for some other reason. */
  }


  mrb_get_args(mrb, "S", &file);

  // if( access( mrb_string_value_ptr(mrb,file), F_OK ) = -1 ) {
  //     fp = fopen(mrb_string_value_ptr(mrb,file) ,"a");
  //     fclose(fp);
  // }

  foo = (HANDLE)_get_osfhandle(fileno(fopen(mrb_string_value_ptr(mrb,file), "w")));
  return mrb_cptr_value(mrb,foo);
}

#endif


void
mrb_mruby_process_gem_test(mrb_state* mrb)
{
  struct RClass *io_test = mrb_define_module(mrb, "ProcessTest");
#if !defined(__APPLE__) && !defined(__linux__)
  mrb_define_class_method(mrb, io_test, "sysopen", mrb_io_test_sysopen, MRB_ARGS_REQ(1));
#endif
}
