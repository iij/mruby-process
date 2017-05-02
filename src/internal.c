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
#include "mruby/class.h"
#include "mruby/hash.h"
#include "mruby/array.h"
#include "mruby/string.h"

typedef struct mrb_execarg {
  char **envp;
  char *filename;
  char **argv;
} mrb_execarg;

static char* mrb_execarg_argv_to_cstr(mrb_state *mrb, mrb_value *argv, mrb_int len);
static void mrb_execarg_fill(mrb_state *mrb, mrb_value env, mrb_value *argv, mrb_int argc, struct mrb_execarg *eargp);

struct mrb_execarg*
mrb_execarg_new(mrb_state *mrb)
{
  mrb_int argc;
  mrb_value *argv, env;
  struct mrb_execarg *eargp;

  mrb_get_args(mrb, "o|*", &env, &argv, &argc);

  switch (mrb_type(env)) {
    case MRB_TT_HASH:
      mrb_get_args(mrb, "H|*", &env, &argv, &argc);
      break;

    case MRB_TT_STRING:
      mrb_get_args(mrb, "*", &argv, &argc);
      env = mrb_nil_value();
      break;

    default:
      mrb_raisef(mrb, E_TYPE_ERROR, "no implicit conversion of %S into String",
                 mrb_obj_value(mrb_class(mrb, env)));
  }

  eargp = malloc(sizeof(struct mrb_execarg));
  mrb_execarg_fill(mrb, env, argv, argc, eargp);

  return eargp;
}

static void
mrb_execarg_fill(mrb_state *mrb, mrb_value env, mrb_value *argv, mrb_int argc, struct mrb_execarg *eargp)
{
  int ai;
  char **result;

  ai     = mrb_gc_arena_save(mrb);
  result = (char **)mrb_malloc(mrb, sizeof(char *) * 4);

  // TODO: cross platform
  result[0] = strdup("/bin/sh");
  result[1] = strdup("-c");
  result[2] = mrb_execarg_argv_to_cstr(mrb, argv, argc);
  result[3] = NULL;

  eargp->envp     = NULL;
  eargp->filename = result[0];
  eargp->argv     = result;

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

  mrb_gc_arena_restore(mrb, ai);
}

static char*
mrb_execarg_argv_to_cstr(mrb_state *mrb, mrb_value *argv, mrb_int len)
{
  mrb_value str;
  int i, ai;

  if (len < 1)
    mrb_raise(mrb, E_ARGUMENT_ERROR, "must have at least 1 argument");

  ai  = mrb_gc_arena_save(mrb);
  str = mrb_str_new(mrb, "", 0);

  for (i = 0; i < len; i++) {
    if (i > 0)
      mrb_str_concat(mrb, str, mrb_str_new(mrb, " ", 1));
    mrb_str_concat(mrb, str, argv[i]);
  }

  mrb_gc_arena_restore(mrb, ai);

  return RSTRING_PTR(str);
}
