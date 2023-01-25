#pragma once

//#define XXH_NAMESPACE vr_
#define XXH_FORCE_MEMORY_ACCESS 2

#define XXH_VECTOR XXH_SSE2
#define XXH_NO_PREFETCH
#define XXH_NO_STREAM
#define XXH_NO_STDLIB
#define XXH_STATIC_LINKING_ONLY
#define XXH_INLINE_ALL

#include "xxHashOrg/xxhash.h"
