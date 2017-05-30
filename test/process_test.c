#include <sys/types.h>
#include <errno.h>
#include <unistd.h>

#if defined(_WIN32) || defined(_WIN64)
  #include <winsock.h>
  #include <io.h>
#else
  #include <sys/socket.h>

  #include <sys/un.h>
#endif

#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>

#include "mruby.h"
#include "mruby/array.h"
#include "mruby/error.h"
#include "mruby/string.h"
#include "mruby/variable.h"

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





void
mrb_mruby_process_gem_test(mrb_state* mrb)
{
  struct RClass *io_test = mrb_define_module(mrb, "WinTest");

  mrb_define_class_method(mrb, io_test, "sysopen", mrb_io_test_sysopen, MRB_ARGS_REQ(1));
}
