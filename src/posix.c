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

    if (!argv0)
        return mrb_nil_value();

    const char *progname = strrchr(argv0, '/');

    if (progname)
        progname++;
    else
        progname = argv0;

    return mrb_str_new_cstr(mrb, progname);
}

pid_t
spawnv(const char *path, char *const argv[])
{
  posix_spawn_file_actions_t action;
  posix_spawn_file_actions_init(&action);
  posix_spawn_file_actions_adddup2 (&action, 0, 0);
  posix_spawn_file_actions_adddup2 (&action, 1, 1);
  posix_spawn_file_actions_adddup2 (&action, 2, 2);

  pid_t pid;

  if (posix_spawn(&pid, path, &action, NULL, argv, NULL) != 0)
      return -1;

  posix_spawn_file_actions_destroy(&action);

  return pid;
}

pid_t
spawnve(const char *path, char *const argv[], char *const envp[], int envc)
{
  posix_spawn_file_actions_t action;
  posix_spawn_file_actions_init(&action);
  posix_spawn_file_actions_adddup2 (&action, 0, 0);
  posix_spawn_file_actions_adddup2 (&action, 1, 1);
  posix_spawn_file_actions_adddup2 (&action, 2, 2);

  pid_t pid;

  if (posix_spawn(&pid, path, &action, NULL, argv, envp) != 0)
      return -1;

  posix_spawn_file_actions_destroy(&action);
  
  return pid;
}
