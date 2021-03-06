#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define _GNU_SOURCE         /* See feature_test_macros(7) */
#include <dlfcn.h>

#include "wrappedlibs.h"

#include "debug.h"
#include "wrapper.h"
#include "bridge.h"
#include "library_private.h"
#include "x86emu.h"
#include "x86emu_private.h"
#include "box86context.h"
#include "librarian.h"

const char* libpthreadName = "libpthread.so.0";
#define LIBNAME libpthread


int my_pthread_create(x86emu_t *emu, void* t, void* attr, void* start_routine, void* arg); //implemented in thread.c
int my_pthread_key_create(x86emu_t* emu, void* key, void* dtor);
int my___pthread_key_create(x86emu_t* emu, void* key, void* dtor);
int my_pthread_once(x86emu_t* emu, void* once, void* cb);
int my_pthread_cond_broadcast(x86emu_t* emu, void* cond);
int my_pthread_cond_destroy(x86emu_t* emu, void* cond);
int my_pthread_cond_init(x86emu_t* emu, void* cond, void* attr);
int my_pthread_cond_signal(x86emu_t* emu, void* cond);
int my_pthread_cond_timedwait(x86emu_t* emu, void* cond, void* mutex, void* abstime);
int my_pthread_cond_wait(x86emu_t* emu, void* cond, void* mutex);


typedef int (*iFpp_t)(void*, void*);
typedef int (*iFppu_t)(void*, void*, uint32_t);
EXPORT int my_pthread_setname_np(x86emu_t* emu, void* t, void* n)
{
    library_t* lib = GetLib(emu->context->maplib, libpthreadName);
    if(!lib) return 0;
    void* f = dlsym(lib->priv.w.lib, "pthread_setname_np");
    if(f)
        return ((iFpp_t)f)(t, n);
    return 0;
}
EXPORT int my_pthread_getname_np(x86emu_t* emu, void* t, void* n, uint32_t s)
{
    library_t* lib = GetLib(emu->context->maplib, libpthreadName);
    if(!lib) return 0;
    void* f = dlsym(lib->priv.w.lib, "pthread_getname_np");
    if(f)
        return ((iFppu_t)f)(t, n, s);
    else 
        strncpy((char*)n, "dummy", s);
    return 0;
}



#include "wrappedlib_init.h"
