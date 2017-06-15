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

//TODO entfernen, nur für debug
#include <stdio.h>

#include <stdlib.h>
#include <strings.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

#ifndef PATH_ENV
# define PATH_ENV "Path"
#endif
#ifndef PATH_SEP
# define PATH_SEP ";"
#endif

static char*
dln_find_1(const char *fname, const char *path, char *buf, size_t size, int exe_flag);

char*
dln_find_exe_r(const char *fname, const char *path, char *buf, size_t size)
{
    if (!path) {
        path = getenv(PATH_ENV);
        if (path) path = strdup(path);
    }
#if defined(__APPLE__) || defined(__linux__)
    if (!path) {
        path =
            "/usr/local/bin" PATH_SEP
            "/usr/ucb" PATH_SEP
            "/usr/bin" PATH_SEP
            "/bin" PATH_SEP
            ".";
    }
#endif


    buf = dln_find_1(fname, path, buf, size, 1);

    return buf;
}

static char *
dln_find_1(const char *fname, const char *path, char *fbuf, size_t size, int exe_flag)
{
    register const char *dp;
    register const char *ep;
    register char *bp;
    struct stat st;
    size_t i, fnlen, fspace;

#if !defined(__APPLE__) && !defined(__linux__)
    static const char extension[][5] = {
        ".exe", ".com", ".cmd", ".bat"
    };
    size_t j;
    int is_abs = 0, has_path = 0;
    const char *ext = 0;
#endif

    const char *p = fname;

    if(!path)
      return NULL;  

#define RETURN_IF(expr) if (expr) return (char *)fname;

    RETURN_IF(!fname);

    fnlen = strlen(fname);

    if (fnlen >= size)
        return NULL;

#if !defined(__APPLE__) && !defined(__linux__)
# ifndef CharNext
# define CharNext(p) ((p)+1)
# endif
# ifdef DOSISH_DRIVE_LETTER
    if (((p[0] | 0x20) - 'a') < 26  && p[1] == ':') {
        p += 2;
        is_abs = 1;
    }
# endif
    switch (*p) {
      case '/': case '\\':
        is_abs = 1;
        p++;
    }
    has_path = is_abs;
    while (*p) {
        switch (*p) {
            case '/': case '\\':
                has_path = 1;
                ext = 0;
                p++;
                break;
            case '.':
                ext = p;
                p++;
                break;
            default:
                p = CharNext(p);
        }
    }
    if (ext) {
        for (j = 0; strcasecmp(ext, extension[j]); ) {
            if (++j == sizeof(extension) / sizeof(extension[0])) {
                ext = 0;
                break;
            }
        }
    }
    ep = bp = 0;

    if (!exe_flag) {
        RETURN_IF(is_abs);
    }
    else if (has_path) {
        RETURN_IF(ext);
        i = p - fname;
        if (i + 1 > size) goto toolong;
        fspace = size - i - 1;
        bp = fbuf;
        ep = p;
        memcpy(fbuf, fname, i + 1);
        goto needs_extension;
    }
    p = fname;
#endif

    if (*p == '.' && *++p == '.') ++p;
    RETURN_IF(*p == '/');
    RETURN_IF(exe_flag && strchr(fname, '/'));

#undef RETURN_IF

    for (dp = path;; dp = ++ep) {
        register size_t l;

        /* extract a component */
        ep = strchr(dp, PATH_SEP[0]);
        if (ep == NULL){
            ep = dp+strlen(dp);
        }
        /* find the length of that component */
        l = ep - dp;
        bp = fbuf;
        fspace = size - 2;
        if (l > 0) {
            /*
            **  If the length of the component is zero length,
            **  start from the current directory.  If the
            **  component begins with "~", start from the
            **  user's $HOME environment variable.  Otherwise
            **  take the path literally.
            */
            if (*dp == '~' && (l == 1 ||
#if !defined(__APPLE__) && !defined(__linux__)
                       dp[1] == '\\' ||
#endif
                       dp[1] == '/')) {

                char *home;

                home = getenv("HOME");
                if (home != NULL) {
                    i = strlen(home);
                    if (fspace < i)
                    goto toolong;
                    fspace -= i;
                    memcpy(bp, home, i);
                    bp += i;
                }
                dp++;
                l--;
            }
            if (l > 0) {
                if (fspace < l)
                    goto toolong;
                fspace -= l;
                memcpy(bp, dp, l);
                bp += l;
            }

            /* add a "/" between directory and filename */
            if (ep[-1] != '/')
                *bp++ = '/';
        }

        /* now append the file name */
        i = fnlen;
        if (fspace < i) {
            toolong:
            goto next;
        }
        fspace -= i;
        memcpy(bp, fname, i + 1);

#if !defined(__APPLE__) && !defined(__linux__)
        if (exe_flag && !ext) {
            needs_extension:
            for (j = 0; j < sizeof(extension) / sizeof(extension[0]); j++) {
                if (fspace < strlen(extension[j])) {
                    continue;
                }
                strcpy(bp + i, extension[j]);
                if (access(fbuf, X_OK) == 0){
                    return fbuf;
                }
            }
            goto next;
        }
#endif

        if (stat(fbuf, &st) == 0 && S_ISREG(st.st_mode)) {
            if (exe_flag == 0){
                return fbuf;
            }
        /* looking for executable */
          if (access(fbuf, X_OK) == 0){
             return fbuf;
           }
        }
        next:
        /* if not, and no other alternatives, life is bleak */
        if (*ep == '\0') {
            return NULL;
        }
        /* otherwise try the next component in the search path */
    }
}
