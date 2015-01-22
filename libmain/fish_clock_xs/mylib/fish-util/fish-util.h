#define __FISH_UTIL_H 1

#include <stdbool.h>
#include <stdio.h>

void spr(const char* format, int size, ...);
char* spr_(const char* format, int size, ...);

void fish_warn(const char* s);
void warnf(const char* format, ...);

void say(const char* s);
void sayf(const char* format, ...);
void info(const char* s);
void infof(const char* format, ...);
FILE* sysr(const char* cmd);
FILE* sysw(const char* cmd);
int sysx(const char* cmd);
int sysxf(const char* orig, ...);
int sysclose(FILE* f);

int sig(int signum, void* func);
void autoflush();
void benchmark();
long int stoie(const char* s, int *err);
long int stoi(const char* s);
char* str(int length);
char* strs(int length);

char* R_(const char* s);
char* BR_(const char* s);
char* G_(const char* s);
char* BG_(const char* s);
char* Y_(const char* s);
char* BY_(const char* s);
char* B_(const char* s);
char* BB_(const char* s);
char* CY_(const char* s);
char* BCY_(const char* s);
char* M_(const char* s);
char* BM_(const char* s);

void R(const char* s);
void BR(const char* s);
void G(const char* s);
void BG(const char* s);
void Y(const char* s);
void BY(const char* s);
void B(const char* s);
void BB(const char* s);
void CY(const char* s);
void BCY(const char* s);
void M(const char* s);
void BM(const char* s);

void disable_colors();

void sys_die(bool b);
void sys_verbose(bool b);

char *_s, *_t, *_u, *_v, *_w;
void fish_();

double time_hires();

const char* perr();
const char* perr_arg(int err);

//int int_length(int i);
//int errorf(const char *format, ...);
//int error(const char* s);

