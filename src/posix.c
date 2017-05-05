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

#include <spawn.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>


#include <sys/wait.h>


int
spawnv(pid_t *pid, const char *path, char *const argv[])
{
  int status;
  status = posix_spawn(pid, path, NULL, NULL, argv, NULL);
  if (status == 0) {
      printf("Child pid: %i\n", *pid);
      if (waitpid(*pid, &status, 0) != -1) {
          printf("Child exited with status %i\n", status);
      } else {
          perror("waitpid");
      }
  } else {
      printf("posix_spawn: %s\n", strerror(status));
  }
    return status;
}

int
spawnve(pid_t *pid, const char * path, char *const argv[], char *const envp[])
{
  int status;
  status = posix_spawn(pid, path, NULL, NULL, argv, envp);
  if (status == 0) {
      printf("Child pid: %i\n", *pid);
      if (waitpid(*pid, &status, 0) != -1) {
          printf("Child exited with status %i\n", status);
      } else {
          perror("waitpid");
      }
  } else {
      printf("posix_spawn: %s\n", strerror(status));
  }
    return status;
}
