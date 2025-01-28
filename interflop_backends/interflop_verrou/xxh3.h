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
      xxh_u64 const ksecretContrib=(XXH_readLE64(XXH3_kSecret+8) ^ XXH_readLE64(XXH3_kSecret+16));
      xxh_u64 const bitflip = ksecretContrib - seed;
      xxh_u64 const input64 = *(reinterpret_cast<const xxh_u64*>(input));
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


inline XXH64_hash_t
XXH3_64bits_withSeed(const float * input,  XXH64_hash_t seed)
{
   /// XXH3_len_4to8_64b rewrite
   seed ^= (xxh_u64)XXH_swap32((xxh_u32)seed) << 32;
   {
      xxh_u32 const input1 = *(reinterpret_cast<const xxh_u32*>(input));
      xxh_u32 const input2 = input1;
      xxh_u64 const input64 = input2 + (((xxh_u64)input1) << 32);

      xxh_u64 const ksecretContrib=(XXH_readLE64(XXH3_kSecret+8) ^ XXH_readLE64(XXH3_kSecret+16));
      xxh_u64 const bitflip = ksecretContrib - seed;

      xxh_u64 const keyed = input64 ^ bitflip;
//       return XXH3_rrmxmx(keyed, len);
      xxh_u64 h64(keyed);
      h64 ^= XXH_rotl64(h64, 49) ^ XXH_rotl64(h64, 24);
      h64 *= 0x9FB21C651E98DF25ULL;
      h64 ^= (h64 >> 35) + 4 ; //4=len(float)
      h64 *= 0x9FB21C651E98DF25ULL;
      //      return XXH_xorshift64(h64, 28);
      return h64 ^ (h64 >> 28);
   }
};


xxh_u64 pre_computed_bitflip=0;
inline void update_pre_computed_bitflip( XXH64_hash_t seed){
   seed ^= (xxh_u64)XXH_swap32((xxh_u32)seed) << 32;
   xxh_u64 const ksecretContrib=(XXH_readLE64(XXH3_kSecret+8) ^ XXH_readLE64(XXH3_kSecret+16));
   pre_computed_bitflip = ksecretContrib - seed;
}

inline XXH64_hash_t
XXH3_64bits_withSeed_with_precomputedbitflip(const double * input)
{
  xxh_u64 const input64 = *(reinterpret_cast<const xxh_u64*>(input));
  xxh_u64 const keyed = input64 ^ pre_computed_bitflip;
//       return XXH3_rrmxmx(keyed, len);
   xxh_u64 h64(keyed);
   h64 ^= XXH_rotl64(h64, 49) ^ XXH_rotl64(h64, 24);
   h64 *= 0x9FB21C651E98DF25ULL;
   h64 ^= (h64 >> 35) + 8 ; //8=len(double)
   h64 *= 0x9FB21C651E98DF25ULL;
   //      return XXH_xorshift64(h64, 28);
   return h64 ^ (h64 >> 28);
};


inline XXH64_hash_t
XXH3_64bits_withSeed_with_precomputedbitflip(const float * input)
{
  xxh_u32 const input1 = *(reinterpret_cast<const xxh_u32*>(input));
  xxh_u32 const input2 = input1;
  xxh_u64 const input64 = input2 + (((xxh_u64)input1) << 32);

  xxh_u64 const keyed = input64 ^ pre_computed_bitflip;
//       return XXH3_rrmxmx(keyed, len);
  xxh_u64 h64(keyed);
  h64 ^= XXH_rotl64(h64, 49) ^ XXH_rotl64(h64, 24);
  h64 *= 0x9FB21C651E98DF25ULL;
  h64 ^= (h64 >> 35) + 4 ; //4=len(float)
  h64 *= 0x9FB21C651E98DF25ULL;
  //      return XXH_xorshift64(h64, 28);
  return h64 ^ (h64 >> 28);
};
