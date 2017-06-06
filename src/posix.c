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

#include <spawn.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>

mrb_value
mrb_argv0(mrb_state *mrb)
{
    const char *argv0 = getenv("_");

    if (!argv0)
        return mrb_nil_value();

    return mrb_str_new_cstr(mrb,argv0);
}

mrb_value
mrb_progname(mrb_state *mrb)
{
    const char *argv0 = getenv("_");
    const char *progname;

    if (!argv0)
        return mrb_nil_value();

    progname = strrchr(argv0, '/');

    if (progname)
        progname++;
    else
        progname = argv0;

    return mrb_str_new_cstr(mrb, progname);
}

pid_t
spawnv(const char *path, char *const argv[], mrb_value in, mrb_value out, mrb_value err)
{
    pid_t pid;
    posix_spawn_file_actions_t action;

    posix_spawn_file_actions_init(&action);


    if(mrb_fixnum_p(in)){
      posix_spawn_file_actions_adddup2 (&action, mrb_fixnum(in), 0);
    }

    if(mrb_fixnum_p(out)){
      posix_spawn_file_actions_adddup2 (&action, mrb_fixnum(out), 1);
    }

    if(mrb_fixnum_p(err)){
      posix_spawn_file_actions_adddup2 (&action, mrb_fixnum(err), 2);
    }

    if (posix_spawn(&pid, path, &action, NULL, argv, NULL) != 0)
        return -1;


    posix_spawn_file_actions_destroy(&action);

    return pid;
}

pid_t
spawnve(const char *path, char *const argv[], char *const envp[], mrb_value in, mrb_value out, mrb_value err)
{
    pid_t pid;
    posix_spawn_file_actions_t action;

    posix_spawn_file_actions_init(&action);

    if(mrb_fixnum_p(in)){
      posix_spawn_file_actions_adddup2 (&action, mrb_fixnum(in), 0);
    }

    if(mrb_fixnum_p(out)){
      posix_spawn_file_actions_adddup2 (&action, mrb_fixnum(out), 1);
    }

    if(mrb_fixnum_p(err)){
      posix_spawn_file_actions_adddup2 (&action, mrb_fixnum(err), 2);
    }

    if (posix_spawn(&pid, path, &action, NULL, argv, envp) != 0)
        return -1;

    posix_spawn_file_actions_destroy(&action);

    return pid;
}
