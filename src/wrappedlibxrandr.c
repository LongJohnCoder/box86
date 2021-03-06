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

const char* libxrandrName = "libXrandr.so.2";
#define LIBNAME libxrandr

#include "wrappedlib_init.h"
