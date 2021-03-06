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
#include "callback.h"
#include "librarian.h"
#include "box86context.h"
#include "x86emu_private.h"

const char* libx11Name = "libX11.so.6";
#define LIBNAME libx11

typedef int (*XErrorHandler)(void *, void *);
void* my_XSetErrorHandler(x86emu_t* t, XErrorHandler handler);
typedef int (*XIOErrorHandler)(void *);
void* my_XSetIOErrorHandler(x86emu_t* t, XIOErrorHandler handler);

EXPORT void* my_XESetWireToEvent(x86emu_t* emu, void* display, int32_t event_number, void* proc)
{
    printf_log(LOG_NONE, "BOX86: Error, called unimplemented XESetWireToEvent(%p, %d, %p)\n", display, event_number, proc);
    emu->quit = 1;
    return NULL;
}
EXPORT void* my_XESetEventToWire(x86emu_t* emu, void* display, int32_t event_number, void* proc)
{
    printf_log(LOG_NONE, "BOX86: Error, called unimplemented XESetEventToWire(%p, %d, %p)\n", display, event_number, proc);
    emu->quit = 1;
    return NULL;
}

typedef int(*EventHandler) (void*,void*,void*);
int32_t my_XIfEvent(x86emu_t* emu, void* d,void* ev, EventHandler h, void* arg);

typedef struct XImageSave_s {
    int   anyEmu;
    void* create;
    void* destroy;
    void* get;
    void* put;
    void* sub;
    void* add;
} XImageSave_t;

typedef struct ximage_s {
    void*(*create_image)(
            void*           /* display */,
            void*           /* visual */,
            uint32_t        /* depth */,
            int32_t         /* format */,
            int32_t         /* offset */,
            void*           /* data */,
            uint32_t        /* width */,
            uint32_t        /* height */,
            int32_t         /* bitmap_pad */,
            int32_t         /* bytes_per_line */);
    int32_t (*destroy_image)        (void*);
    uint32_t (*get_pixel)           (void*, int32_t, int32_t);
    int32_t (*put_pixel)            (void*, int32_t, int32_t, uint32_t);
    void*(*sub_image)    (void*, int32_t, int32_t, uint32_t, uint32_t); //sub_image return a new XImage that need bridging => custom wrapper
    int32_t (*add_pixel)            (void*, int32_t);
} ximage_t;

typedef struct _XImage {
    int32_t width, height;          /* size of image */
    int32_t xoffset;                /* number of pixels offset in X direction */
    int32_t format;                 /* XYBitmap, XYPixmap, ZPixmap */
    void*   data;                   /* pointer to image data */
    int32_t byte_order;             /* data byte order, LSBFirst, MSBFirst */
    int32_t bitmap_unit;            /* quant. of scanline 8, 16, 32 */
    int32_t bitmap_bit_order;       /* LSBFirst, MSBFirst */
    int32_t bitmap_pad;             /* 8, 16, 32 either XY or ZPixmap */
    int32_t depth;                  /* depth of image */
    int32_t bytes_per_line;         /* accelarator to next line */
    int32_t bits_per_pixel;         /* bits per pixel (ZPixmap) */
    uint32_t red_mask;              /* bits in z arrangment */
    uint32_t green_mask;
    uint32_t blue_mask;
    void*    obdata;                 /* hook for the object routines to hang on */
    ximage_t f;
} XImage;

typedef void* (*pFp_t)(void*);
typedef void* (*pFpip_t)(void*, int32_t, void*);
typedef int32_t (*iFp_t)(void*);
typedef int32_t (*iFpi_t)(void*, int32_t);
typedef int32_t (*iFpppp_t)(void*, void*, void*, void*);
typedef uint32_t (*uFpii_t)(void*, int32_t, int32_t);
typedef int32_t (*iFpiiu_t)(void*, int32_t, int32_t, uint32_t);
typedef void* (*pFpiiuu_t)(void*, int32_t, int32_t, uint32_t, uint32_t);
typedef void* (*pFppiiuuui_t)(void*, void*, int32_t, int32_t, uint32_t, uint32_t, uint32_t, int32_t);
typedef void* (*pFppuiipuuii_t)(void*, void*, uint32_t, int32_t, int32_t, void*, uint32_t, uint32_t, int32_t, int32_t);
typedef void* (*pFppiiuuuipii_t)(void*, void*, int32_t, int32_t, uint32_t, uint32_t, uint32_t, int32_t, void*, int32_t, int32_t);
typedef int32_t (*iFppppiiiiuu_t)(void*, void*, void*, void*, int32_t, int32_t, int32_t, int32_t, uint32_t, uint32_t);

typedef struct x11_my_s {
    // functions
    pFp_t           XSetErrorHandler;
    pFp_t           XSetIOErrorHandler;
    pFpip_t         XESetError;
    iFpppp_t        XIfEvent;
    iFpppp_t        XCheckIfEvent;
    iFpppp_t        XPeekIfEvent;
    pFppuiipuuii_t  XCreateImage;
    pFppiiuuui_t    XGetImage;
    iFppppiiiiuu_t  XPutImage;
    pFppiiuuuipii_t XGetSubImage;
} x11_my_t;

void* getX11My(library_t* lib)
{
    x11_my_t* my = (x11_my_t*)calloc(1, sizeof(x11_my_t));
    #define GO(A, W) my->A = (W)dlsym(lib->priv.w.lib, #A);
    GO(XSetErrorHandler, pFp_t)
    GO(XSetIOErrorHandler, pFp_t)
    GO(XESetError, pFpip_t)
    GO(XIfEvent, iFpppp_t)
    GO(XCheckIfEvent, iFpppp_t)
    GO(XPeekIfEvent, iFpppp_t)
    GO(XCreateImage, pFppuiipuuii_t)
    GO(XGetImage, pFppiiuuui_t)
    GO(XPutImage, iFppppiiiiuu_t)
    GO(XGetSubImage, pFppiiuuuipii_t)
    #undef GO
    return my;
}

void freeX11My(void* lib)
{
    x11_my_t *my = (x11_my_t *)lib;
}


void* my_XCreateImage(x86emu_t* emu, void* disp, void* vis, uint32_t depth, int32_t fmt, int32_t off
                    , void* data, uint32_t w, uint32_t h, int32_t pad, int32_t bpl);

void* my_XGetImage(x86emu_t* emu, void* disp, void* drawable, int32_t x, int32_t y
                    , uint32_t w, uint32_t h, uint32_t plane, int32_t fmt);

int32_t my_XPutImage(x86emu_t* emu, void* disp, void* drawable, void* gc, void* image
                    , int32_t src_x, int32_t src_y, int32_t dst_x, int32_t dst_y
                    , uint32_t w, uint32_t h);

void* my_XGetSubImage(x86emu_t* emu, void* disp, void* drawable
                    , int32_t x, int32_t y
                    , uint32_t w, uint32_t h, uint32_t plane, int32_t fmt
                    , void* image, int32_t dst_x, int32_t dst_y);

#define CUSTOM_INIT \
    lib->priv.w.p2 = getX11My(lib);

#define CUSTOM_FINI \
    freeX11My(lib->priv.w.p2); \
    free(lib->priv.w.p2);

#include "wrappedlib_init.h"

static x86emu_t *errorhandlercb = NULL;
static x86emu_t *ioerrorhandlercb = NULL;
static x86emu_t *exterrorhandlercb = NULL;  // should be set per screen and per extension!
static int my_errorhandle_callback(void* display, void* errorevent)
{
    if(!errorhandlercb)
        return 0;
    SetCallbackArg(errorhandlercb, 0, display);
    SetCallbackArg(errorhandlercb, 1, errorevent);
    return (int)RunCallback(errorhandlercb);
}
static int my_ioerrorhandle_callback(void* display)
{
    if(!ioerrorhandlercb)
        return 0;
    SetCallbackArg(ioerrorhandlercb, 0, display);
    return (int)RunCallback(ioerrorhandlercb);
}
static int my_exterrorhandle_callback(void* display, void* err, void* codes, int* ret_code)
{
    if(!exterrorhandlercb)
        return 0;
    SetCallbackArg(exterrorhandlercb, 0, display);
    SetCallbackArg(exterrorhandlercb, 1, err);
    SetCallbackArg(exterrorhandlercb, 2, codes);
    SetCallbackArg(exterrorhandlercb, 3, ret_code);
    return (int)RunCallback(exterrorhandlercb);
}

EXPORT void* my_XSetErrorHandler(x86emu_t* emu, XErrorHandler handler)
{
    library_t * lib = GetLib(emu->context->maplib, libx11Name);
    x11_my_t *my = (x11_my_t*)lib->priv.w.p2;
    x86emu_t *cb = NULL;
    void* ret;
    XErrorHandler old = NULL;
    if(GetNativeFnc((uintptr_t)handler)) {
        old = (XErrorHandler)my->XSetErrorHandler(GetNativeFnc((uintptr_t)handler));
    } else {
        cb = AddCallback(emu, (uintptr_t)handler, 2, NULL, NULL, NULL, NULL);
        old = (XErrorHandler)my->XSetErrorHandler(cb);
    }
    if(CheckBridged(lib->priv.w.bridge, old))
        ret = (void*)CheckBridged(lib->priv.w.bridge, old);
    else
        ret = (void*)AddBridge(lib->priv.w.bridge, iFpp, old, 0);
    if(errorhandlercb) FreeCallback(errorhandlercb);
    errorhandlercb = cb;
    return ret;
}

EXPORT void* my_XSetIOErrorHandler(x86emu_t* emu, XIOErrorHandler handler)
{
    library_t * lib = GetLib(emu->context->maplib, libx11Name);
    x11_my_t *my = (x11_my_t*)lib->priv.w.p2;
    if(ioerrorhandlercb) { FreeCallback(ioerrorhandlercb); ioerrorhandlercb=NULL;}
    x86emu_t *cb = NULL;
    void* ret;
    XIOErrorHandler old = NULL;
    if(GetNativeFnc((uintptr_t)handler)) {
        old = (XIOErrorHandler)my->XSetIOErrorHandler(GetNativeFnc((uintptr_t)handler));
    } else {
        cb = AddCallback(emu, (uintptr_t)handler, 2, NULL, NULL, NULL, NULL);
        old = (XIOErrorHandler)my->XSetIOErrorHandler(cb);
    }
    if(CheckBridged(lib->priv.w.bridge, old))
        ret = (void*)CheckBridged(lib->priv.w.bridge, old);
    else
        ret = (void*)AddBridge(lib->priv.w.bridge, iFp, old, 0);
    if(ioerrorhandlercb) FreeCallback(ioerrorhandlercb);
    ioerrorhandlercb = cb;
    return ret;
}

EXPORT void* my_XESetError(x86emu_t* emu, void* display, int32_t extension, void* handler)
{
    library_t * lib = GetLib(emu->context->maplib, libx11Name);
    x11_my_t *my = (x11_my_t*)lib->priv.w.p2;
    x86emu_t *cb = NULL;
    void* ret;
    void* old = NULL;
    if(GetNativeFnc((uintptr_t)handler)) {
        old = my->XESetError(display, extension, GetNativeFnc((uintptr_t)handler));
    } else {
        cb = AddCallback(emu, (uintptr_t)handler, 4, NULL, NULL, NULL, NULL);
        old = my->XESetError(display, extension, cb);
    }
    if(CheckBridged(lib->priv.w.bridge, old))
        ret = (void*)CheckBridged(lib->priv.w.bridge, old);
    else
        ret = (void*)AddBridge(lib->priv.w.bridge, iFpip, old, 0);
    if(exterrorhandlercb) FreeCallback(exterrorhandlercb);
    exterrorhandlercb = cb;
    return ret;
}

int32_t xifevent_callback(void* dpy, void *event, void* arg)
{
    x86emu_t *emu = (x86emu_t*)arg;
    SetCallbackArg(emu, 0, dpy);
    SetCallbackArg(emu, 1, event);
    return (int32_t)RunCallback(emu);
}

EXPORT int32_t my_XIfEvent(x86emu_t* emu, void* d,void* ev, EventHandler h, void* arg)
{
    library_t * lib = GetLib(emu->context->maplib, libx11Name);
    x11_my_t *my = (x11_my_t*)lib->priv.w.p2;
    x86emu_t *cb = NULL;
    cb = AddSharedCallback(emu, (uintptr_t)h, 3, NULL, NULL, arg, NULL);
    int32_t ret = my->XIfEvent(d, ev, xifevent_callback, (void*)cb);
    FreeCallback(cb);
    return ret;
}

EXPORT int32_t my_XCheckIfEvent(x86emu_t* emu, void* d,void* ev, EventHandler h, void* arg)
{
    library_t * lib = GetLib(emu->context->maplib, libx11Name);
    x11_my_t *my = (x11_my_t*)lib->priv.w.p2;
    x86emu_t *cb = NULL;
    cb = AddSharedCallback(emu, (uintptr_t)h, 3, NULL, NULL, arg, NULL);
    int32_t ret = my->XCheckIfEvent(d, ev, xifevent_callback, (void*)cb);
    FreeCallback(cb);
    return ret;
}

EXPORT int32_t my_XPeekIfEvent(x86emu_t* emu, void* d,void* ev, EventHandler h, void* arg)
{
    library_t * lib = GetLib(emu->context->maplib, libx11Name);
    x11_my_t *my = (x11_my_t*)lib->priv.w.p2;
    x86emu_t *cb = NULL;
    cb = AddSharedCallback(emu, (uintptr_t)h, 3, NULL, NULL, arg, NULL);
    int32_t ret = my->XPeekIfEvent(d, ev, xifevent_callback, (void*)cb);
    FreeCallback(cb);
    return ret;
}

void sub_image_wrapper(x86emu_t *emu, uintptr_t fnc);
typedef void* (*sub_image_wrapper_t)(void*, int32_t, int32_t, uint32_t, uint32_t);


void BridgeImageFunc(x86emu_t *emu, XImage *img)
{
    bridge_t* system = emu->context->system;

    #define GO(A, W) \
    fnc = CheckBridged(system, img->f.A); \
    if(!fnc) fnc = AddBridge(system, W, img->f.A, 0); \
    img->f.A = (W##_t)fnc;

    uintptr_t fnc;

    GO(create_image, pFppuiipuuii)
    GO(destroy_image, iFp)
    GO(get_pixel, uFpii)
    GO(put_pixel, iFpiiu)
    GO(sub_image, sub_image_wrapper)
    GO(add_pixel, iFpi)
    #undef GO
}

void UnbridgeImageFunc(x86emu_t *emu, XImage *img)
{
    bridge_t* system = emu->context->system;

    #define GO(A, W) \
    fnc = GetNativeFnc((uintptr_t)img->f.A); \
    if(fnc) \
        img->f.A = (W##_t)fnc;

    void* fnc;

    GO(create_image, pFppuiipuuii)
    GO(destroy_image, iFp)
    GO(get_pixel, uFpii)
    GO(put_pixel, iFpiiu)
    GO(sub_image, sub_image_wrapper)
    GO(add_pixel, iFpi)
    #undef GO
}

void sub_image_wrapper(x86emu_t *emu, uintptr_t fnc)
{
    pFpiiuu_t fn = (pFpiiuu_t)fnc; 
    void* img = fn(*(void**)(R_ESP + 4), *(int32_t*)(R_ESP + 8), *(int32_t*)(R_ESP + 12), *(uint32_t*)(R_ESP + 16), *(uint32_t*)(R_ESP + 20));
    BridgeImageFunc(emu, (XImage*)img);
    R_EAX=(uintptr_t)img;
}


EXPORT void* my_XCreateImage(x86emu_t* emu, void* disp, void* vis, uint32_t depth, int32_t fmt, int32_t off
                    , void* data, uint32_t w, uint32_t h, int32_t pad, int32_t bpl)
{
    library_t * lib = GetLib(emu->context->maplib, libx11Name);
    x11_my_t *my = (x11_my_t*)lib->priv.w.p2;

    XImage *img = my->XCreateImage(disp, vis, depth, fmt, off, data, w, h, pad, bpl);
    if(!img)
        return img;
    // bridge all access functions...
    BridgeImageFunc(emu, img);
    return img;
}

EXPORT void* my_XGetImage(x86emu_t* emu, void* disp, void* drawable, int32_t x, int32_t y
                    , uint32_t w, uint32_t h, uint32_t plane, int32_t fmt)
{
    library_t * lib = GetLib(emu->context->maplib, libx11Name);
    x11_my_t *my = (x11_my_t*)lib->priv.w.p2;

    XImage *img = my->XGetImage(disp, drawable, x, y, w, h, plane, fmt);
    if(!img)
        return img;
    // bridge all access functions...
    BridgeImageFunc(emu, img);
    return img;
}

EXPORT int32_t my_XPutImage(x86emu_t* emu, void* disp, void* drawable, void* gc, void* image
                    , int32_t src_x, int32_t src_y, int32_t dst_x, int32_t dst_y
                    , uint32_t w, uint32_t h)
{
    library_t * lib = GetLib(emu->context->maplib, libx11Name);
    x11_my_t *my = (x11_my_t*)lib->priv.w.p2;

    UnbridgeImageFunc(emu, (XImage*)image);
    int32_t r = my->XPutImage(disp, drawable, gc, image, src_x, src_y, dst_x, dst_y, w, h);
    // bridge all access functions...
    BridgeImageFunc(emu, (XImage*)image);
    return r;
}

EXPORT void* my_XGetSubImage(x86emu_t* emu, void* disp, void* drawable
                    , int32_t x, int32_t y
                    , uint32_t w, uint32_t h, uint32_t plane, int32_t fmt
                    , void* image, int32_t dst_x, int32_t dst_y)
{
    library_t * lib = GetLib(emu->context->maplib, libx11Name);
    x11_my_t *my = (x11_my_t*)lib->priv.w.p2;

    UnbridgeImageFunc(emu, (XImage*)image);
    XImage *img = my->XGetSubImage(disp, drawable, x, y, w, h, plane, fmt, image, dst_x, dst_y);
    if(img)
        BridgeImageFunc(emu, img);

    BridgeImageFunc(emu, (XImage*)image);
    return img;
}
