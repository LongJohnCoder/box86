#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include <sys/syscall.h>   /* For SYS_xxx definitions */
#define _GNU_SOURCE         /* See feature_test_macros(7) */
#include <unistd.h>
#include <time.h>
#include <sys/mman.h>
#include <sys/select.h>
#include <sys/types.h>
#include <asm/stat.h>
#include <errno.h>

#include "debug.h"
#include "stack.h"
#include "x86emu.h"
#include "x86run.h"
#include "x86emu_private.h"
#include "x86run_private.h"
#include "x86primop.h"
#include "x86trace.h"
#include "myalign.h"

// Syscall table for x86 can be found here: http://shell-storm.org/shellcode/files/syscalls.html
typedef struct scwrap_s {
    int x86s;
    int nats;
    int nbpars;
} scwrap_t;

scwrap_t syscallwrap[] = {
    { 3, __NR_read, 3 },
    { 4, __NR_write, 3 },
    { 5, __NR_open, 3 },
    { 6, __NR_close, 1 },
    { 10, __NR_unlink, 1 },
#ifdef __NR_time
    { 13, __NR_time, 1 },
#endif
    { 20, __NR_getpid, 0 },
    { 33, __NR_access, 2 },
    { 38, __NR_rename, 2 },
    { 39, __NR_mkdir, 2 },
    { 54, __NR_ioctl, 5 },
    { 78, __NR_gettimeofday, 2 },
    { 85, __NR_readlink, 3 },
    { 91, __NR_munmap, 2 },
    { 125,__NR_mprotect, 3 },
    { 140,__NR__llseek, 5 },
#ifdef __NR_select
    { 142, __NR_select, 5 },
#endif
    { 143, __NR_flock,  2 },
    { 162, __NR_nanosleep, 2 },
    { 172, __NR_prctl, 5 },
    { 183, __NR_getcwd, 2 },
    { 186, __NR_sigaltstack, 2 },    // neeed wrap or something?
    { 191, __NR_ugetrlimit, 2 },
    { 192, __NR_mmap2, 6},
    //{ 195, __NR_stat64, 2 },  // need proprer wrap because of structure size change
    //{ 197, __NR_fstat64, 2 },  // need proprer wrap because of structure size change
    { 220, __NR_getdents64, 3 },
    { 224, __NR_gettid, 0 },
    { 240, __NR_futex, 6 },
    { 252, __NR_exit_group, 1 },
    //{ 270, __NR_tgkill, 3 },
};

struct mmap_arg_struct {
    unsigned long addr;
    unsigned long len;
    unsigned long prot;
    unsigned long flags;
    unsigned long fd;
    unsigned long offset;
};

#undef st_atime
#undef st_ctime
#undef st_mtime

void EXPORT x86Syscall(x86emu_t *emu)
{
    RESET_FLAGS(emu);
    uint32_t s = R_EAX;
    printf_log(LOG_DEBUG, "%p: Calling syscall 0x%02X (%d) %p %p %p %p %p\n", (void*)R_EIP, s, s, (void*)R_EBX, (void*)R_ECX, (void*)R_EDX, (void*)R_ESI, (void*)R_EDI); 
    // check wrapper first
    int cnt = sizeof(syscallwrap) / sizeof(scwrap_t);
    for (int i=0; i<cnt; i++) {
        if(syscallwrap[i].x86s == s) {
            int sc = syscallwrap[i].nats;
            switch(syscallwrap[i].nbpars) {
                case 0: *(int32_t*)&R_EAX = syscall(sc); return;
                case 1: *(int32_t*)&R_EAX = syscall(sc, R_EBX); return;
                case 2: if(s==33) {printf_log(LOG_DUMP, " => sys_access(\"%s\", %d)\n", (char*)R_EBX, R_ECX);}; *(int32_t*)&R_EAX = syscall(sc, R_EBX, R_ECX); return;
                case 3: *(int32_t*)&R_EAX = syscall(sc, R_EBX, R_ECX, R_EDX); return;
                case 4: *(int32_t*)&R_EAX = syscall(sc, R_EBX, R_ECX, R_EDX, R_ESI); return;
                case 5: *(int32_t*)&R_EAX = syscall(sc, R_EBX, R_ECX, R_EDX, R_ESI, R_EDI); return;
                case 6: *(int32_t*)&R_EAX = syscall(sc, R_EBX, R_ECX, R_EDX, R_ESI, R_EDI, R_EBP); return;
                default:
                   printf_log(LOG_NONE, "ERROR, Unimplemented syscall wrapper (%d, %d)\n", s, syscallwrap[i].nbpars); 
                   emu->quit = 1;
                   return;
            }
        }
    }
    switch (s) {
        case 1: // sys_exit
            emu->quit = 1;
            R_EAX = R_EBX; // faking the syscall here, we don't want to really terminate the program now
            return;
#ifndef __NR_time
        case 13:    // sys_time (it's deprecated and remove on ARM EABI it seems)
            R_EAX = time(NULL);
            return;
#endif
        case 90:    // old_mmap
            {
                struct mmap_arg_struct *st = (struct mmap_arg_struct*)R_EBX;
                R_EAX = (uintptr_t)mmap((void*)st->addr, st->len, st->prot, st->flags, st->fd, st->offset);
            }
            return;
#ifndef __NR_select
        case 142:   // select
            R_EAX = select(R_EBX, (fd_set*)R_ECX, (fd_set*)R_EDX, (fd_set*)R_ESI, (struct timeval*)R_EDI);
            return;
#endif
        case 174: // sys_rt_sigaction
            printf_log(LOG_NONE, "Warning, Ignoring sys_rt_sigaction(0x%02X, %p, %p)\n", R_EBX, (void*)R_ECX, (void*)R_EDX);
            R_EAX = 0;
            return;
        case 195:
            {   
                struct stat64 st;
                unsigned int r = syscall(__NR_stat64, R_EBX, &st);
                UnalignStat64(&st, (void*)R_ECX);
                
                R_EAX = r;
            }
            return;
        case 197:
            {   
                struct stat64 st;
                unsigned int r = syscall(__NR_fstat64, R_EBX, &st);
                UnalignStat64(&st, (void*)R_ECX);
                
                R_EAX = r;
            }
            return;
        case 270:
            R_EAX = syscall(__NR_tgkill, R_EBX, R_ECX, R_EDX);
            return;
        default:
            printf_log(LOG_INFO, "Error: Unsupported Syscall 0x%02Xh (%d)\n", s, s);
            emu->quit = 1;
            emu->error |= ERR_UNIMPL;
    }
}

#define stack(n) (R_ESP+4+n)
#define i32(n)  *(int32_t*)stack(n)
#define u32(n)  *(uint32_t*)stack(n)
#define p(n)    *(void**)stack(n)

uint32_t EXPORT my_syscall(x86emu_t *emu)
{
    uint32_t s = u32(0);
    printf_log(LOG_DEBUG, "%p: Calling libc syscall 0x%02X (%d) %p %p %p %p %p\n", (void*)R_EIP, s, s, (void*)u32(4), (void*)u32(8), (void*)u32(12), (void*)u32(16), (void*)u32(20)); 
    // check wrapper first
    int cnt = sizeof(syscallwrap) / sizeof(scwrap_t);
    for (int i=0; i<cnt; i++) {
        if(syscallwrap[i].x86s == s) {
            int sc = syscallwrap[i].nats;
            switch(syscallwrap[i].nbpars) {
                case 0: return syscall(sc);
                case 1: return syscall(sc, u32(4));
                case 2: return syscall(sc, u32(4), u32(8));
                case 3: return syscall(sc, u32(4), u32(8), u32(12));
                case 4: return syscall(sc, u32(4), u32(8), u32(12), u32(16));
                case 5: return syscall(sc, u32(4), u32(8), u32(12), u32(16), u32(20));
                case 6: return syscall(sc, u32(4), u32(8), u32(12), u32(16), u32(20), u32(24));
                default:
                   printf_log(LOG_NONE, "ERROR, Unimplemented syscall wrapper (%d, %d)\n", s, syscallwrap[i].nbpars); 
                   emu->quit = 1;
                   return 0;
            }
        }
    }
    switch (s) {
        case 1: // __NR_exit
            emu->quit = 1;
            return u32(4); // faking the syscall here, we don't want to really terminate the program now
        case 270: //_NR_tgkill
            if(!u32(12)) {
                printf("tgkill(%u, %u, %u) => ", u32(4), u32(8), u32(12));
                uint32_t ret = (uint32_t)syscall(__NR_tgkill, u32(4), u32(8), u32(12));
                printf("%u\n", ret);
                return ret;
            } else {
                printf_log(LOG_INFO, "Warning: ignoring libc Syscall tgkill (%u, %u, %u)\n", u32(4), u32(8), u32(12));
            }
            return 0;
        default:
            printf_log(LOG_INFO, "Error: Unsupported libc Syscall 0x%02X (%d)\n", s, s);
            emu->quit = 1;
            emu->error |= ERR_UNIMPL;
    }
    return 0;
}
