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

#ifndef MRB_PROCESS_H
#define MRB_PROCESS_H 1

#if defined(__cplusplus)
extern "C" {
#endif

#include "mruby.h"
#include <signal.h>
#include <sys/types.h>

#ifndef WNOHANG
# define WNOHANG -1
#endif

#ifndef WUNTRACED
# define WUNTRACED 0
#endif

#ifndef SIGINT
# define SIGINT 2
#endif

#ifndef SIGKILL
# define SIGKILL 9
#endif

#ifndef WIFEXITED
# define WIFEXITED(w)    (((w) & 0xff) == 0)
#endif

#ifndef WIFSIGNALED
# define WIFSIGNALED(w)  (((w) & 0x7f) > 0 && (((w) & 0x7f) < 0x7f))
#endif

#ifndef WIFSTOPPED
# define WIFSTOPPED(w)   (((w) & 0xff) == 0x7f)
#endif

#ifndef WEXITSTATUS
# define WEXITSTATUS(w)  (((w) >> 8) & 0xff)
#endif

#ifndef WTERMSIG
# define WTERMSIG(w)     ((w) & 0x7f)
#endif

#ifndef WSTOPSIG
# define WSTOPSIG        WEXITSTATUS
#endif

mrb_value mrb_progname(mrb_state *mrb);
mrb_value mrb_argv0(mrb_state *mrb);

mrb_value mrb_last_status_get(mrb_state *mrb);
void mrb_last_status_set(mrb_state *mrb, pid_t pid, mrb_int status);
void mrb_last_status_clear(mrb_state *mrb);

void _exit(int status);
void exit(int status);

int   getpid(void);
pid_t getppid(void);
pid_t waitpid(pid_t pid, int *stat_loc, int options);

int fork(void);
pid_t spawnv(const char *path, char *const argv[]);
pid_t spawnve(const char *path, char *const argv[], char *const envp[], int envc);
int execv(const char *path, char *const argv[]);
int execve(const char *path, char *const argv[], char *const envp[]);
int kill(pid_t pid, int sig);

#if defined(__cplusplus)
} /* extern "C" { */
#endif
#endif /* MRB_PROCESS_H */
