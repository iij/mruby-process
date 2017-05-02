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
#include <string.h>

#include "mruby.h"
#include "mruby/hash.h"
#include "mruby/array.h"
#include "mruby/string.h"

typedef struct mrb_execarg {
  union {
    struct {
      char *shell_script;
    } sh;
    struct {
      char *command_name;
      char *command_abspath;
    } cmd;
  } invoke;
  unsigned use_shell : 1;
  char **envp;
  char **argv;
} mrb_execarg;

static int mrb_value_to_strv(mrb_state *mrb, mrb_value *array, mrb_int len, char **result);
static void mrb_process_set_childstat_gv(mrb_state *mrb, mrb_value childstat);
static void mrb_process_set_pid_gv(mrb_state *mrb);
static struct mrb_execarg* mrb_execarg_new(mrb_state *mrb, int accept_shell);
static void mrb_exec_fillarg(mrb_state *mrb, mrb_value env, mrb_value *argv, mrb_int argc, struct mrb_execarg *eargp);

static int
mrb_value_to_strv(mrb_state *mrb, mrb_value *array, mrb_int len, char **result)
{
  mrb_value strv;
  char *buf;
  int i;

  if (len < 1)
    mrb_raise(mrb, E_ARGUMENT_ERROR, "must have at least 1 argument");

  int ai = mrb_gc_arena_save(mrb);

  for (i = 0; i < len; i++) {
    strv = mrb_convert_type(mrb, array[i], MRB_TT_STRING, "String", "to_str");
    buf  = (char *)mrb_string_value_cstr(mrb, &strv);

    *result = buf;
    result++;
  }

  *result = NULL;
  result -= i;

  mrb_gc_arena_restore(mrb, ai);

  return 0;
}

static size_t
memsize_exec_arg(void)
{
  return sizeof(struct mrb_execarg);
}

static struct mrb_execarg*
mrb_execarg_new(mrb_state *mrb, int accept_shell)
{
  mrb_int argc;
  mrb_value *argv, env;
  struct mrb_execarg *eargp;

  mrb_get_args(mrb, "o|*", &env, &argv, &argc);

  if (mrb_hash_p(env))
    mrb_get_args(mrb, "H|*", &env, &argv, &argc);
  else
    mrb_get_args(mrb, "*", &argv, &argc);

  eargp = malloc(memsize_exec_arg());
  eargp->use_shell = accept_shell;
  mrb_exec_fillarg(mrb, env, argv, argc, eargp);

  return eargp;
}

static void
mrb_exec_fillarg(mrb_state *mrb, mrb_value env, mrb_value *argv, mrb_int argc, struct mrb_execarg *eargp)
{
  int ai = mrb_gc_arena_save(mrb);
  char **result;

  result = (char **)mrb_malloc(mrb, sizeof(char *) * (argc + 1));
  mrb_value_to_strv(mrb, argv, argc, result);

  if (mrb_test(env)) {
    mrb_int len;
    mrb_value keys;
    char **envp;
    int i;

    keys = mrb_hash_keys(mrb, env);
    len  = RARRAY_LEN(keys);
    envp = (char **)mrb_malloc(mrb, sizeof(char *) * (len + 1));

    for (i = 0; i < len; ++i) {
      mrb_value key  = mrb_ary_ref(mrb, keys, i);
      mrb_value val  = mrb_hash_get(mrb, env, key);
      mrb_value skey = mrb_symbol_p(key) ? mrb_sym2str(mrb, mrb_symbol(key)) : key;
      mrb_value sval = mrb_convert_type(mrb, val, MRB_TT_STRING, "String", "to_str");
      mrb_int slen   = RSTRING_LEN(skey) + RSTRING_LEN(sval) + 1;
      char str[slen];

      sprintf(str, "%s=%s",
        mrb_string_value_cstr(mrb, &skey),
        mrb_string_value_cstr(mrb, &sval));

      envp[i] = strdup(str);
    }

    envp[i]     = NULL;
    eargp->envp = envp;
  }

  eargp->use_shell = argc == 0;
  eargp->argv      = result;

  if (eargp->use_shell)
    eargp->invoke.sh.shell_script  = result[0];
  else
    eargp->invoke.cmd.command_name = result[0];

  mrb_gc_arena_restore(mrb, ai);
}
