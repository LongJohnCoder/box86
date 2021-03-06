#ifndef __DEBUG_H_
#define __DEBUG_H_

extern int box86_log;    // log level
extern int dlsym_error;  // log dlsym error
extern int trace_xmm;    // include XMM reg in trace?
#define LOG_NONE 0
#define LOG_INFO 1
#define LOG_DEBUG 2
#define LOG_DUMP 3

extern FILE* ftrace;

#define printf_log(L, ...) if(L<=box86_log) {fprintf(ftrace, __VA_ARGS__); fflush(ftrace);}

#define EXPORT __attribute__((visibility("default")))

#endif //__DEBUG_H_