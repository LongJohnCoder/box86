#ifndef __LIBRARIAN_H_
#define __LIBRARIAN_H_
#include <stdint.h>

typedef struct lib_s lib_t;
typedef struct bridge_s bridge_t;
typedef struct library_s library_t;
typedef struct kh_mapsymbols_s kh_mapsymbols_t;
typedef struct dlprivate_s dlprivate_t;
typedef struct box86context_s  box86context_t;
typedef struct x86emu_s x86emu_t;
typedef struct elfheader_s elfheader_t;
typedef struct box86context_s box86context_t;

lib_t *NewLibrarian(box86context_t* context);
void FreeLibrarian(lib_t **maplib);
dlprivate_t *NewDLPrivate();
void FreeDLPrivate(dlprivate_t **lib);

kh_mapsymbols_t* GetMapSymbol(lib_t* maplib);
kh_mapsymbols_t* GetWeakSymbol(lib_t* maplib);
kh_mapsymbols_t* GetLocalSymbol(lib_t* maplib);
int AddNeededLib(lib_t* maplib, const char* path, box86context_t* box86, x86emu_t* emu); // 0=success, 1=error
int FinalizeNeededLib(lib_t* maplib, const char* path, box86context_t* box86, x86emu_t* emu);
library_t* GetLib(lib_t* maplib, const char* name);
uintptr_t FindGlobalSymbol(lib_t *maplib, const char* name);
int GetNoSelfSymbolStartEnd(lib_t *maplib, const char* name, uintptr_t* start, uintptr_t* end, elfheader_t* self);
int GetGlobalSymbolStartEnd(lib_t *maplib, const char* name, uintptr_t* start, uintptr_t* end);
int GetGlobalNoWeakSymbolStartEnd(lib_t *maplib, const char* name, uintptr_t* start, uintptr_t* end);
int GetLocalSymbolStartEnd(lib_t *maplib, const char* name, uintptr_t* start, uintptr_t* end, elfheader_t *self);

void AddSymbol(kh_mapsymbols_t *mapsymbols, const char* name, uintptr_t addr, uint32_t sz);
uintptr_t FindSymbol(kh_mapsymbols_t *mapsymbols, const char* name);
int GetSymbolStartEnd(kh_mapsymbols_t* mapsymbols, const char* name, uintptr_t* start, uintptr_t* end);
const char* GetSymbolName(kh_mapsymbols_t* mapsymbols, void* p, uintptr_t* offs, uint32_t* sz);

const char* FindSymbolName(lib_t *maplib, void* p, void** start, uint32_t* sz, const char** libname, void** base);

#endif //__LIBRARIAN_H_