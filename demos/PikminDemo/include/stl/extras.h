#ifndef _EXTRAS_H
#define _EXTRAS_H
#include "types.h"

#ifdef TARGET_PC
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

static inline int stricmp(const char* lhs, const char* rhs)
{
#if defined(_WIN32)
	return _stricmp(lhs, rhs);
#else
	return strcasecmp(lhs, rhs);
#endif
}

#ifdef __cplusplus
}
#endif

#else

#ifdef __cplusplus
extern "C" {
#endif

int stricmp(const char*, const char*);

#ifdef __cplusplus
}
#endif

#endif // TARGET_PC
#endif
