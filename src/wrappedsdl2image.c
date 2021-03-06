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
#include "sdl2rwops.h"

typedef void* (*pFpi_t)(void*, int32_t);
typedef void* (*pFp_t)(void*);
typedef void* (*pFpip_t)(void*, int32_t, void*);

typedef struct sdl2image_my_s {
    pFp_t       IMG_LoadBMP_RW;
    pFp_t       IMG_LoadCUR_RW;
    pFp_t       IMG_LoadGIF_RW;
    pFp_t       IMG_LoadICO_RW;
    pFp_t       IMG_LoadJPG_RW;
    pFp_t       IMG_LoadLBM_RW;
    pFp_t       IMG_LoadPCX_RW;
    pFp_t       IMG_LoadPNG_RW;
    pFp_t       IMG_LoadPNM_RW;
    pFp_t       IMG_LoadTGA_RW;
    pFp_t       IMG_LoadTIF_RW;
    pFpip_t     IMG_LoadTyped_RW;
    pFp_t       IMG_LoadWEBP_RW;
    pFp_t       IMG_LoadXCF_RW;
    pFp_t       IMG_LoadXPM_RW;
    pFp_t       IMG_LoadXV_RW;
    pFpi_t      IMG_Load_RW;
} sdl2image_my_t;

static void* getSDL2ImageMy(library_t* lib)
{
    sdl2image_my_t* my = (sdl2image_my_t*)calloc(1, sizeof(sdl2image_my_t));
    #define GO(A, W) my->A = (W)dlsym(lib->priv.w.lib, #A);
    GO(IMG_LoadBMP_RW,pFp_t)
    GO(IMG_LoadCUR_RW,pFp_t)
    GO(IMG_LoadGIF_RW,pFp_t)
    GO(IMG_LoadICO_RW,pFp_t)
    GO(IMG_LoadJPG_RW,pFp_t)
    GO(IMG_LoadLBM_RW,pFp_t)
    GO(IMG_LoadPCX_RW,pFp_t)
    GO(IMG_LoadPNG_RW,pFp_t)
    GO(IMG_LoadPNM_RW,pFp_t)
    GO(IMG_LoadTGA_RW,pFp_t)
    GO(IMG_LoadTIF_RW,pFp_t)
    GO(IMG_LoadTyped_RW,pFpip_t)
    GO(IMG_LoadWEBP_RW,pFp_t)
    GO(IMG_LoadXCF_RW,pFp_t)
    GO(IMG_LoadXPM_RW,pFp_t)
    GO(IMG_LoadXV_RW,pFp_t)
    GO(IMG_Load_RW,pFpi_t)
    #undef GO
    return my;
}

#define GO(A) \
EXPORT void *my2_##A(x86emu_t* emu, void* a) \
{ \
    sdl2image_my_t *my = (sdl2image_my_t *)emu->context->sdl2imagelib->priv.w.p2; \
    SDL2RWSave_t save; \
    RWNativeStart2(emu, (SDL2_RWops_t*)a, &save); \
    void* r = my->A(a); \
    RWNativeEnd2(emu, (SDL2_RWops_t*)a, &save); \
    return r; \
}
GO(IMG_LoadBMP_RW)
GO(IMG_LoadCUR_RW)
GO(IMG_LoadGIF_RW)
GO(IMG_LoadICO_RW)
GO(IMG_LoadJPG_RW)
GO(IMG_LoadLBM_RW)
GO(IMG_LoadPCX_RW)
GO(IMG_LoadPNG_RW)
GO(IMG_LoadPNM_RW)
GO(IMG_LoadTGA_RW)
GO(IMG_LoadTIF_RW)
GO(IMG_LoadWEBP_RW)
GO(IMG_LoadXCF_RW)
GO(IMG_LoadXPM_RW)
GO(IMG_LoadXV_RW)
#undef GO

 EXPORT void *my2_IMG_LoadTyped_RW(x86emu_t* emu, void* a, int32_t b, void* c)
{
    sdl2image_my_t *my = (sdl2image_my_t *)emu->context->sdl2imagelib->priv.w.p2;
    SDL2RWSave_t save;
    RWNativeStart2(emu, (SDL2_RWops_t*)a, &save);
    void* r = my->IMG_LoadTyped_RW(a, b, c);
    if(b==0)
        RWNativeEnd2(emu, (SDL2_RWops_t*)a, &save);
    return r;
}
EXPORT void *my2_IMG_Load_RW(x86emu_t* emu, void* a, int32_t b)
{
    sdl2image_my_t *my = (sdl2image_my_t *)emu->context->sdl2imagelib->priv.w.p2;
    SDL2RWSave_t save;
    RWNativeStart2(emu, (SDL2_RWops_t*)a, &save);
    void* r = my->IMG_Load_RW(a, b);
    if(b==0)
        RWNativeEnd2(emu, (SDL2_RWops_t*)a, &save);
    return r;
}
const char* sdl2imageName = "libSDL2_image-2.0.so.0";
#define LIBNAME sdl2image

#define CUSTOM_INIT \
    box86->sdl2imagelib = lib; \
    lib->priv.w.p2 = getSDL2ImageMy(lib); \
    lib->altmy = strdup("my2_");

#define CUSTOM_FINI \
    free(lib->priv.w.p2); \
    ((box86context_t*)(lib->context))->sdl2imagelib = NULL;

#include "wrappedlib_init.h"

