#pragma once

#include <dolphin/os.h>

#ifndef GX_STUB_LOG
#define GX_STUB_LOG 1  // Enable by default to track unimplemented functions
#endif

#if GX_STUB_LOG
#define GX_STUB_NOTICE(name) OSReport("[GX STUB] %s() - Not yet implemented\n", name)
#else
#define GX_STUB_NOTICE(name) ((void)0)
#endif

#define GX_UNUSED(x) (void)(x)


