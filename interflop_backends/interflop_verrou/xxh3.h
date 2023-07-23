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



inline XXH64_hash_t
XXH3_64bits_withSeed(const double * input,  XXH64_hash_t seed)
{
   /// XXH3_len_4to8_64b rewrite
   seed ^= (xxh_u64)XXH_swap32((xxh_u32)seed) << 32;
   {
//      xxh_u32 const input1 = XXH_readLE32((void*)input);
//      xxh_u32 const input2 = XXH_readLE32(((void*)input) + len - 4);
      xxh_u64 const bitflip = (XXH_readLE64(XXH3_kSecret+8) ^ XXH_readLE64(XXH3_kSecret+16)) - seed;
      xxh_u64 const input64 = *((xxh_u64*)input);
      xxh_u64 const keyed = input64 ^ bitflip;
//       return XXH3_rrmxmx(keyed, len);
      xxh_u64 h64(keyed);
      h64 ^= XXH_rotl64(h64, 49) ^ XXH_rotl64(h64, 24);
      h64 *= 0x9FB21C651E98DF25ULL;
      h64 ^= (h64 >> 35) + 8 ; //8=len(double)
      h64 *= 0x9FB21C651E98DF25ULL;
      //      return XXH_xorshift64(h64, 28);
      return h64 ^ (h64 >> 28);
   }

};
