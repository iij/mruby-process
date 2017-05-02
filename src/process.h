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

#ifndef PROCESS_H
#define PROCESS_H 1

#include <stdlib.h>
#include <signal.h>
#include <sys/types.h>

#ifdef _WIN32
# include <process.h>
#endif

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

extern void _exit(int status);
extern void exit(int status);

extern int getpid(void);
extern pid_t getppid(void);
extern pid_t waitpid(pid_t pid, int *stat_loc, int options);

extern int fork(void);
extern int execv(const char *path, char *const argv[]);
extern int execve(const char *filename, char *const argv[], char *const envp[]);
extern int kill(pid_t pid, int sig);

#endif /* PROCESS_H */
