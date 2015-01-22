#define _GNU_SOURCE 1

// Be careful, sys() can get truncated.
#define FISH_CMD_LENGTH 1000
#define FISH_ERR_LENGTH 100
#define FISH_INFO_LENGTH 500
#define FISH_WARN_LENGTH 500

#define FISH_STATIC_STR_LENGTH 100

// length of (longest) color escape
#define FISH_COLOR_LENGTH 5
#define FISH_COLOR_LENGTH_RESET 4

#include <signal.h>
// errno
#include <errno.h>

#include <sys/timeb.h>

// LONG_MIN
#include <limits.h>

#include <string.h>

#include <stdlib.h>
// varargs
#include <stdarg.h>

// also has va_list stuff
//#include <glib.h>

#include <stdio.h>
#include <sys/time.h>
#include <math.h>

#include <assert.h>

// offsetof
#include <stddef.h>

#include <sys/socket.h>
#include <sys/un.h>

#include "fish-util.h"

char* strng;
char* strng2;
char* strng3;

char *_s, *_t, *_u, *_v, *_w;
int NUM_STATIC_STRINGS = 5;
int _static_str_idx = -1;
bool _static_str_initted = false;
char** _static_strs[5];

char* COL[] = {
    // reset
    "[0m",
    // red
    "[31m",
    // bright red
    "[91m",
    // green
    "[32m",
    // bright green
    "[92m",
    // yellow
    "[33m",
    // bright yellow
    "[93m",
    // blue
    "[34m",
    // bright blue
    "[94m",
    // cyan
    "[36m",
    // bright cyan
    "[96m",
    // magenta
    "[35m",
    // bright magenta
    "[95m",

};

enum COLORS {
    RED=1,
    BRIGHT_RED,
    GREEN,
    BRIGHT_GREEN,
    YELLOW,
    BRIGHT_YELLOW,
    BLUE,
    BRIGHT_BLUE,
    CYAN,
    BRIGHT_CYAN,
    MAGENTA,
    BRIGHT_MAGENTA,
};

int _disable_colors = 0;
bool _die = false;
bool _verbose = true;

char** _get_static_str_ptr();

void disable_colors() {
    _disable_colors = 1;
}

// string with all nulls (can't use strlen).
char* str(int length) {
    assert(length > 0);
    char* s = malloc(length * sizeof(char));
    memset(s, '\0', length);
    return s;
}

// Is null-terminated, even if size too small.
void spr(const char* format, int size /*with null*/, ...) {
    char* s = str(size);
    va_list arglist;
    va_start( arglist, size );
    vsnprintf(s, size, format, arglist);
    va_end( arglist );

    int l = strlen(s);
    if (l > FISH_STATIC_STR_LENGTH - 1) l = FISH_STATIC_STR_LENGTH - 1;
    char** ptr = _get_static_str_ptr();
    memcpy(*ptr, s, l + 1);
    free(s);
}

char* spr_(const char* format, int size /*with null*/, ...) {
    char* s = str(size);
    va_list arglist;
    va_start( arglist, size );
    vsnprintf(s, size, format, arglist);
    va_end( arglist );
    return s;
}

// string with spaces and one null.
char* strs(int length) {
    assert(length > 0);
    char* s = malloc(length * sizeof(char));
    memset(s, ' ', length);
    s[-1] = '\0';
    return s;
}

char* _color(const char* s, int idx) {
    char* t = str(strlen(s) + 1 + 4 + 5);
    char* a = _disable_colors ? "" : COL[idx];
    char* b = _disable_colors ? "" : COL[0];
    sprintf(t, "%s%s%s", a, s, b );
    return t;
}

void _static_str_init() {
    _static_strs[0] = &_s;
    _static_strs[1] = &_t;
    _static_strs[2] = &_u;
    _static_strs[3] = &_v;
    _static_strs[4] = &_w;

    int i;
    for (i = 0; i < NUM_STATIC_STRINGS; i++) {
        *_static_strs[i] = malloc(sizeof(char) * FISH_STATIC_STR_LENGTH);
    }

}

void fish_() {
    if (! _static_str_initted) {
        _static_str_initted = true;
        _static_str_init();
    }

    _static_str_idx = -1;
    int i;
    for (i = 0; i < NUM_STATIC_STRINGS; i++) {
        memset(*_static_strs[i], '\0', FISH_STATIC_STR_LENGTH);
    }
}

char* R_(const char* s) {
    return _color(s, RED);
}
char* BR_(const char* s) {
    return _color(s, BRIGHT_RED);
}
char* G_(const char* s) {
    return _color(s, GREEN);
}
char* BG_(const char* s) {
    return _color(s, BRIGHT_GREEN);
}
char* Y_(const char* s) {
    return _color(s, YELLOW);
}
char* BY_(const char* s) {
    return _color(s, BRIGHT_YELLOW);
}
char* B_(const char* s) {
    return _color(s, BLUE);
}
char* BB_(const char* s) {
    return _color(s, BRIGHT_BLUE);
}
char* CY_(const char* s) {
    return _color(s, CYAN);
}
char* BCY_(const char* s) {
    return _color(s, BRIGHT_CYAN);
}
char* M_(const char* s) {
    return _color(s, MAGENTA);
}
char* BM_(const char* s) {
    return _color(s, BRIGHT_MAGENTA);
}

void _color_static(const char* c) {
    int l = strlen(c);
    if (l > FISH_STATIC_STR_LENGTH - 1) l = FISH_STATIC_STR_LENGTH - 1;
    char** ptr = _get_static_str_ptr();
    // include null
    memcpy(*ptr, c, l + 1);
}

char** _get_static_str_ptr() {
    _static_str_idx = (_static_str_idx+1) % NUM_STATIC_STRINGS;
    return _static_strs[_static_str_idx];
}

void R(const char* s) {
    char* c = _color(s, RED);
    _color_static(c);
    free(c);
}

void BR(const char* s) {
    char* c = _color(s, BRIGHT_RED);
    _color_static(c);
    free(c);
}

void G(const char* s) {
    char* c = _color(s, GREEN);
    _color_static(c);
    free(c);
}

void BG(const char* s) {
    char* c = _color(s, BRIGHT_GREEN);
    _color_static(c);
    free(c);
}

void Y(const char* s) {
    char* c = _color(s, YELLOW);
    _color_static(c);
    free(c);
}

void BY(const char* s) {
    char* c = _color(s, BRIGHT_YELLOW);
    _color_static(c);
    free(c);
}

void B(const char* s) {
    char* c = _color(s, BLUE);
    _color_static(c);
    free(c);
}

void BB(const char* s) {
    char* c = _color(s, BRIGHT_BLUE);
    _color_static(c);
    free(c);
}

void CY(const char* s) {
    char* c = _color(s, CYAN);
    _color_static(c);
    free(c);
}

void BCY(const char* s) {
    char* c = _color(s, BRIGHT_CYAN);
    _color_static(c);
    free(c);
}

void M(const char* s) {
    char* c = _color(s, MAGENTA);
    _color_static(c);
    free(c);
}

void BM(const char* s) {
    char* c = _color(s, BRIGHT_MAGENTA);
    _color_static(c);
    free(c);
}

const char* perr() {
    char* st = str(100); 
    strerror_r(errno, st, 100); // discard return
    return st;
}

const char* perr_arg(int err) {
    char* st = str(100); 
    strerror_r(err, st, 100); // discard return
    return st;
}

char* add_nl(const char *orig) {
    // not including null
    int len = strlen(orig);
    char* new = malloc(sizeof(char) * (len + 2));
    memset(new, '\0', len + 2);
    // no null
    strncpy(new, orig, len);
    // adds null
    strcat(new, "\n");
    return new;
}

void sayf(const char *format, ...) {
    char* new = add_nl(format);
    va_list arglist;
    va_start( arglist, format );
    vprintf( new, arglist );
    va_end( arglist );
    free(new);
}

void say(const char *s) {
    char* new = add_nl(s);
    printf(new);
    free(new);
}

void infof( const char *format, ... ) {
    char* new = str(FISH_INFO_LENGTH);
    va_list arglist;
    va_start( arglist, format );
    vsnprintf( new, FISH_INFO_LENGTH, format, arglist );
    va_end( arglist );
    info(new);
    free(new);
}

void info(const char *s) {
    char* new = str(strlen(s) + 1 + FISH_COLOR_LENGTH + FISH_COLOR_LENGTH_RESET + 2 + 1);
    char* c = BB_("*");
    sprintf(new, "%s %s\n", c, s);
    printf(new);
    free(c);
    free(new);
}

void warnf(const char* format, ...) {
    char* new = str(FISH_WARN_LENGTH);
    va_list arglist;
    va_start( arglist, format );
    vsnprintf(new, FISH_WARN_LENGTH, format, arglist );
    va_end( arglist );
    fish_warn(new);
    free(new);
} 

void fish_warn(const char* s) {
    char* new = str(strlen(s) + 1 + FISH_COLOR_LENGTH + FISH_COLOR_LENGTH_RESET + 2 + 1);
    char* c = BR_("*");
    sprintf(new, "%s %s\n", c, s);
    fprintf(stderr, new);
    free(new);
    free(c);
} 

void sys_die(bool b) {
    _die = b;
}

void sys_verbose(bool b) {
    _verbose = b;
}

void _sys_say(const char* cmd) {
    char* new = str(strlen(cmd) + 1 + FISH_COLOR_LENGTH + FISH_COLOR_LENGTH_RESET + 2 + 1);
    char* c = G_("*");
    sprintf(new, "%s %s", c, cmd);
    say (new);
    free(new);
    free(c);
}

FILE* sysr(const char* cmd) {
    if (_verbose) _sys_say(cmd);
    FILE* f = popen(cmd, "r");
    // fork or pipe or alloc failed; other errors caught on close.
    if (f == NULL) warnf("Can't open cmd: %s (%s)", cmd, perr());
    return f;
}

FILE* sysw(const char* cmd) {
    if (_verbose) _sys_say(cmd);
    FILE* f = popen(cmd, "w");
    // fork or pipe or alloc failed; other errors caught on close.
    if (f == NULL) warnf("Can't open cmd: %s (%s)", cmd, perr());

    return f;
}

// 0 means good.
int sysclose(FILE* f) {
    int i = pclose(f);
    if (i != 0) {
        // Can't figure out how to make exit value into a useful message.
        // Just hope the command printed something to stderr.
        fish_warn("Error with cmd.");
        return -1;
    }
    return i;
}

// 0 means good
int sysxf(const char* orig, ...) {
    char* cmd = str(FISH_CMD_LENGTH);
    va_list arglist;
    va_start( arglist, orig );
    vsnprintf( cmd, FISH_CMD_LENGTH, orig, arglist );
    va_end( arglist );

    int c = sysx(cmd);
    free(cmd);
    return c;
}

// 0 means good
int sysx(const char* cmd) {
    FILE* f = sysr(cmd);

    int le = FISH_CMD_LENGTH;
    char* buf = str(le);
    while (fgets(buf, le, f) != NULL) {
    }
    free(buf);

    if (f != NULL) {
        int ret = sysclose(f);
        return ret;
    }
    else return 1;
}

int sig(int signum, void* func) {
    struct sigaction action;
    memset(&action, 0, sizeof(action));
    action.sa_handler = func;
    return sigaction(signum, &action, NULL);
}

void autoflush() {
    setvbuf(stdout, NULL, _IONBF, 0);
}

void benchmark(int flag) {

    static int sec;
    static int usec;

    struct timeval *tv = malloc(sizeof *tv); 

    if (flag == 0) {
        gettimeofday(tv, NULL);
        sec = tv->tv_sec;
        usec = tv->tv_usec;
    }
    else {
        gettimeofday(tv, NULL);

        int delta_s = tv->tv_sec - sec;
        int delta_us = tv->tv_usec - usec;
        if (delta_us < 0) {
            delta_s--;
            delta_us += 1e6;
        }
        float delta = delta_s + delta_us / 1e6;
        say(spr_("Benchmark: %f", 50, delta));

        sec = -1;
        usec = -1;
    }
}

// errno doesn't always work.
long int stoie(const char* s, int *err) {
    char *endptr;
    long int i = strtol(s, &endptr, 10);
    // nothing converted
    if (endptr == s) {
        *err = 1;
        return LONG_MIN;
    }
    else {
        return i;
    }
}

long int stoi(const char* s) {
    int err = 0;
    int i = stoie(s, &err);
    if (err) {
        // XX
        warnf("Can't convert string to int: %s", R_(s));
        return LONG_MIN;
    }
    return i;
}

int int_length(int i) {
    return 1 + (int) log10(i);
}

double time_hires() {
    struct timeb t;
    memset(&t, 0, sizeof(t));
    ftime(&t);

    time_t time = t.time;
    unsigned short millitm = t.millitm;

    double c = time + millitm / 1000.0;
    return c;
}

/*
int errorf(const char *format, ...) {
    char* new = str(FISH_ERR_LENGTH);
    va_list arglist;
    va_start( arglist, format );
    vsprintf(new, format, arglist );
    va_end( arglist );
    error(new);
    return -1;
}

int error(const char *s) {
    char* new = str(strlen(s) + 1 + FISH_COLOR_LENGTH + FISH_COLOR_LENGTH_RESET + 2 + 1);
    char* c = R_("*");
    sprintf(new, "%s %s\n", c, s);
    fprintf(stderr, new);
    free(new);
    free(c);
    exit(1);
    return -1;
}
*/
