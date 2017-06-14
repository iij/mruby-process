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

#include <stdlib.h>
#include <strings.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

#ifndef PATH_ENV
# define PATH_ENV "path"
#endif
#ifndef PATH_SEP
# define PATH_SEP ";"
#endif

static char *dln_find_1(const char *fname, const char *path, char *buf, size_t size, int exe_flag);
char* dln_find_exe_r(const char *fname, const char *path, char *buf, size_t size);
char* dln_find_file_r(const char *fname, const char *path, char *buf, size_t size);
char * stringReplace(char *search, char *replace, char *string);


char*
dln_find_exe_r(const char *fname, const char *path, char *buf, size_t size)
{
 	char *envpath = 0;
  char working_copy[80];


/* TODO This is a workaround to stop dln_fin_exe_r from returning
  e.g. "echo %MYVAR% > tmp/spawn.txt" AS ABSOLUTE PATH
  TO EXECUTABLE "echo %MYVAR% > tmp/spawn.txt" because that's total nonsense
  and a cause for errors
  On second thought, this would also eliminate any path with included whitespaces*/
  // if(strrchr(fname, ' '))
  //   return NULL;

  strcpy(working_copy,fname);


#if !defined(__APPLE__) || !defined(__linux__)
  if(!strstr(working_copy, ".exe"))
    strcat(working_copy, ".exe");
#endif

  	if (!path) {
    	path = getenv(PATH_ENV);
	  	if (path)
      		path = envpath = strdup(path);
  	}

  	if (!path) {
		path =
	    	"/usr/local/bin" PATH_SEP
	     	"/usr/ucb" PATH_SEP
	     	"/usr/bin" PATH_SEP
	     	"/bin" PATH_SEP
	     	".";
  	}

  	buf = dln_find_1(working_copy, path, buf, size, 1);

  	if (envpath)
  		free(envpath);
  // printf("first buf %s\n", buf);
// #if !defined(__APPLE__) && !defined(__linux__)
//     stringReplace("/", "\\", buf);
// #endif
  // printf("secon buf %s\n", buf);
	return buf;
}

char *
dln_find_file_r(const char *fname, const char *path, char *buf, size_t size)
{
    if (!path) path = ".";
    return dln_find_1(fname, path, buf, size, 0);
}

static char *
dln_find_1(const char *fname, const char *path, char *fbuf, size_t size, int exe_flag)
{
	register const char *dp;
    register const char *ep;
    register char *bp;
    struct stat st;
    size_t i, fnlen, fspace;

#ifdef DOSISH
    static const char extension[][5] = {
	EXECUTABLE_EXTS,
    };
    size_t j;
    int is_abs = 0, has_path = 0;
    const char *ext = 0;
#endif

    const char *p = fname;

#define RETURN_IF(expr) if (expr) return (char *)fname;

    RETURN_IF(!fname);

    fnlen = strlen(fname);

    if (fnlen >= size)
		return NULL;

#ifdef DOSISH
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
		for (j = 0; STRCASECMP(ext, extension[j]); ) {
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
		    **	If the length of the component is zero length,
		    **	start from the current directory.  If the
		    **	component begins with "~", start from the
		    **	user's $HOME environment variable.  Otherwise
		    **	take the path literally.
		    */
		    if (*dp == '~' && (l == 1 ||
#ifdef DOSISH
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

#ifdef DOSISH
		if (exe_flag && !ext) {
	  		needs_extension:
	    	for (j = 0; j < sizeof(extension) / sizeof(extension[0]); j++) {
				if (fspace < strlen(extension[j])) {
		    		PATHNAME_TOO_LONG();
		    		continue;
				}
				strlcpy(bp + i, extension[j], fspace);
				if (stat(fbuf, &st) == 0){
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
	    if (access(fbuf, X_OK) == 0) return fbuf;
		}
      	next:
	    /* if not, and no other alternatives, life is bleak */
		if (*ep == '\0') {
	    	return NULL;
		}
	    /* otherwise try the next component in the search path */
    }
}



char * stringReplace(char *search, char *replace, char *string) {
	char *tempString, *searchStart;
	int len=0;


	// preuefe ob Such-String vorhanden ist
	searchStart = strstr(string, search);
	if(searchStart == NULL) {
		return string;
	}

	// Speicher reservieren
	tempString = (char*) malloc(strlen(string) * sizeof(char));
	if(tempString == NULL) {
		return NULL;
	}

	// temporaere Kopie anlegen
	strcpy(tempString, string);

	// ersten Abschnitt in String setzen
	len = searchStart - string;
	string[len] = '\0';

	// zweiten Abschnitt anhaengen
	strcat(string, replace);

	// dritten Abschnitt anhaengen
	len += strlen(search);
	strcat(string, (char*)tempString+len);

	// Speicher freigeben
	free(tempString);

	return string;
}
