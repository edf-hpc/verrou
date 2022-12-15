#pragma once


static uint64_t seedTab[8];

class vr_multiply_shift_hash{
public:
  template<class REALTYPE, int NB>
  static inline bool hashBool(const Vr_Rand * r,
		       const vr_packArg<REALTYPE,NB>& pack,
		       uint32_t hashOp){
    const uint64_t m=vr_multiply_shift_hash::multiply(pack,hashOp);
    return (m +seedTab[7])>>63;
  }

  template<class REALTYPE, int NB>
  static inline double hashRatio(const Vr_Rand * r,
		       const vr_packArg<REALTYPE,NB>& pack,
		       uint32_t hashOp){
    const uint64_t m=vr_multiply_shift_hash::multiply(pack, hashOp);
    const uint32_t v=(m+seedTab[7])>>32;
    constexpr double  invMax= (1./4294967296.);//2**32 = 4294967296
    return ((double)v * invMax );
  }

  static inline uint64_t multiply(const vr_packArg<float,1>& pack, uint32_t hashOp){
    const uint32_t a1=realToUint32_reinterpret_cast(pack.arg1);
    return (a1+seedTab[0])*(hashOp+seedTab[6]);
  }
  static inline uint64_t multiply(const vr_packArg<float,2>& pack,  uint32_t hashOp){
    const uint32_t a1=realToUint32_reinterpret_cast(pack.arg1);
    const uint32_t a2=realToUint32_reinterpret_cast(pack.arg2);
    return (a1+seedTab[0]) * (a2+seedTab[1]) +(hashOp*seedTab[6]) ;
  }
  static inline uint64_t multiply(const vr_packArg<float,3>& pack, uint32_t hashOp){
    const uint32_t a1=realToUint32_reinterpret_cast(pack.arg1);
    const uint32_t a2=realToUint32_reinterpret_cast(pack.arg2);
    const uint32_t a3=realToUint32_reinterpret_cast(pack.arg3);
    return (a1+seedTab[0]) * (a2+seedTab[1]) + (a3+seedTab[2])* (hashOp+seedTab[6]);
  }

  static inline uint64_t multiply(const vr_packArg<double,1>& pack,uint32_t hashOp){
    const uint64_t a1=realToUint64_reinterpret_cast<double>(pack.arg1);
    const uint32_t a1_1=a1;
    const uint32_t a1_2=a1>>32;
    return (a1_1+seedTab[0]) * (a1_2+seedTab[1]) + (hashOp*seedTab[6]);
  }

  static inline uint64_t multiply(const vr_packArg<double,2>& pack,uint32_t hashOp){
    const uint64_t a1=realToUint64_reinterpret_cast<double>(pack.arg1);
    const uint32_t a1_1=a1;
    const uint32_t a1_2=a1>>32;

    const uint64_t a2=realToUint64_reinterpret_cast<double>(pack.arg2);
    const uint32_t a2_1=a2;
    const uint32_t a2_2=a2>>32;

    return (a1_1+seedTab[0]) * (a1_2+seedTab[1]) + (a2_1+seedTab[2]) * (a2_2+seedTab[3])+ (hashOp*seedTab[6]);
  }

  static inline uint64_t multiply(const vr_packArg<double,3>& pack, uint32_t hashOp){
    uint64_t a1=realToUint64_reinterpret_cast<double>(pack.arg1);
    uint32_t a1_1=a1;
    uint32_t a1_2=a1>>32;

    uint64_t a2=realToUint64_reinterpret_cast<double>(pack.arg2);
    uint32_t a2_1=a2;
    uint32_t a2_2=a2>>32;

    uint64_t a3=realToUint64_reinterpret_cast<double>(pack.arg3);
    uint32_t a3_1=a3;
    uint32_t a3_2=a3>>32;

    return (a1_1+seedTab[0]) * (a1_2+seedTab[1])
         + (a2_1+seedTab[2]) * (a2_2+seedTab[3])
         + (a3_1+seedTab[4]) * (a3_2+seedTab[5])
         + (hashOp*seedTab[6]);
  }

  static inline void genTable(tinymt64_t& gen){
    for(int i=0; i<8;i++){
      seedTab[i]= tinymt64_generate_uint64(&gen );
    }
  };

};
