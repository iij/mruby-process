/* MIT License
 *
 * This code was based on https://github.com/ruby/ruby/blob/trunk/signal.c
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
#include "mruby/hash.h"

#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#if defined(__BEOS__) || defined(__HAIKU__)
# undef SIGBUS
#endif

#ifndef NSIG
# define NSIG (_SIGMAX + 1)      /* For QNX */
#endif

#ifndef SIGINT                 /* For Win32 */
# define SIGINT 2
#endif
#ifndef SIGKILL                /* For Win32 */
# define SIGKILL 9
#endif

static const struct signals {
    const char *signm;
    int  signo;
} siglist [] = {
    {"EXIT", 0},
#ifdef SIGHUP
    {"HUP", SIGHUP},
#endif
    {"INT", SIGINT},
#ifdef SIGQUIT
    {"QUIT", SIGQUIT},
#endif
#ifdef SIGILL
    {"ILL", SIGILL},
#endif
#ifdef SIGTRAP
    {"TRAP", SIGTRAP},
#endif
#ifdef SIGABRT
    {"ABRT", SIGABRT},
#endif
#ifdef SIGIOT
    {"IOT", SIGIOT},
#endif
#ifdef SIGEMT
    {"EMT", SIGEMT},
#endif
#ifdef SIGFPE
    {"FPE", SIGFPE},
#endif
#ifdef SIGKILL
    {"KILL", SIGKILL},
#endif
#ifdef SIGBUS
    {"BUS", SIGBUS},
#endif
#ifdef SIGSEGV
    {"SEGV", SIGSEGV},
#endif
#ifdef SIGSYS
    {"SYS", SIGSYS},
#endif
#ifdef SIGPIPE
    {"PIPE", SIGPIPE},
#endif
#ifdef SIGALRM
    {"ALRM", SIGALRM},
#endif
#ifdef SIGTERM
    {"TERM", SIGTERM},
#endif
#ifdef SIGURG
    {"URG", SIGURG},
#endif
#ifdef SIGSTOP
    {"STOP", SIGSTOP},
#endif
#ifdef SIGTSTP
    {"TSTP", SIGTSTP},
#endif
#ifdef SIGCONT
    {"CONT", SIGCONT},
#endif
#ifdef SIGCHLD
    {"CHLD", SIGCHLD},
#endif
#ifdef SIGCLD
    {"CLD", SIGCLD},
#else
# ifdef SIGCHLD
    {"CLD", SIGCHLD},
# endif
#endif
#ifdef SIGTTIN
    {"TTIN", SIGTTIN},
#endif
#ifdef SIGTTOU
    {"TTOU", SIGTTOU},
#endif
#ifdef SIGIO
    {"IO", SIGIO},
#endif
#ifdef SIGXCPU
    {"XCPU", SIGXCPU},
#endif
#ifdef SIGXFSZ
    {"XFSZ", SIGXFSZ},
#endif
#ifdef SIGVTALRM
    {"VTALRM", SIGVTALRM},
#endif
#ifdef SIGPROF
    {"PROF", SIGPROF},
#endif
#ifdef SIGWINCH
    {"WINCH", SIGWINCH},
#endif
#ifdef SIGUSR1
    {"USR1", SIGUSR1},
#endif
#ifdef SIGUSR2
    {"USR2", SIGUSR2},
#endif
#ifdef SIGLOST
    {"LOST", SIGLOST},
#endif
#ifdef SIGMSG
    {"MSG", SIGMSG},
#endif
#ifdef SIGPWR
    {"PWR", SIGPWR},
#endif
#ifdef SIGPOLL
    {"POLL", SIGPOLL},
#endif
#ifdef SIGDANGER
    {"DANGER", SIGDANGER},
#endif
#ifdef SIGMIGRATE
    {"MIGRATE", SIGMIGRATE},
#endif
#ifdef SIGPRE
    {"PRE", SIGPRE},
#endif
#ifdef SIGGRANT
    {"GRANT", SIGGRANT},
#endif
#ifdef SIGRETRACT
    {"RETRACT", SIGRETRACT},
#endif
#ifdef SIGSOUND
    {"SOUND", SIGSOUND},
#endif
#ifdef SIGINFO
    {"INFO", SIGINFO},
#endif
    {NULL, 0}
};

static const char signame_prefix[3] = "SIG";

static int
signm2signo(const char *nm)
{
    const struct signals *sigs;

    for (sigs = siglist; sigs->signm; sigs++)

    if (strcmp(sigs->signm, nm) == 0)
        return sigs->signo;

    return 0;
}

static const char*
signo2signm(int no)
{
    const struct signals *sigs;

    for (sigs = siglist; sigs->signm; sigs++)

    if (sigs->signo == no)
        return sigs->signm;

    return 0;
}

/*
 * call-seq:
 *     Signal.signame(signo)  ->  string or nil
 *
 *  Convert signal number to signal name.
 *  Returns +nil+ if the signo is an invalid signal number.
 */
static mrb_value
mrb_sig_signame(mrb_state *mrb, mrb_value klass)
{
    mrb_value sigo;
    int signo;
    const char *signame;

    mrb_get_args(mrb, "i", &sigo);

    signo   = mrb_fixnum(sigo);
    signame = signo2signm(signo);

    if (!signame)
        return mrb_nil_value();

    return mrb_str_new_cstr(mrb, signame);
}

/*
 * call-seq:
 *   Signal.list -> a_hash
 *
 * Returns a list of signal names mapped to the corresponding
 * underlying signal numbers.
 *
 *   Signal.list   #=> {"EXIT"=>0, "HUP"=>1, "INT"=>2, "QUIT"=>3, "ILL"=>4, "TRAP"=>5, "IOT"=>6, "ABRT"=>6, "FPE"=>8, "KILL"=>9, "BUS"=>7, "SEGV"=>11, "SYS"=>31, "PIPE"=>13, "ALRM"=>14, "TERM"=>15, "URG"=>23, "STOP"=>19, "TSTP"=>20, "CONT"=>18, "CHLD"=>17, "CLD"=>17, "TTIN"=>21, "TTOU"=>22, "IO"=>29, "XCPU"=>24, "XFSZ"=>25, "VTALRM"=>26, "PROF"=>27, "WINCH"=>28, "USR1"=>10, "USR2"=>12, "PWR"=>30, "POLL"=>29}
 */
static mrb_value
mrb_sig_list(mrb_state *mrb, mrb_value klass)
{
    mrb_value h = mrb_hash_new(mrb);
    const struct signals *sigs;

    for (sigs = siglist; sigs->signm; sigs++) {
        mrb_value sig, signo;

        sig   = mrb_str_new_cstr(mrb, sigs->signm);
        signo = mrb_fixnum_value(sigs->signo);

        mrb_hash_set(mrb, h, sig, signo);
    }

    return h;
}
