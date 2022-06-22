#pragma once

static uint32_t hashTable[4][8][256];
static uint32_t hashTableOp[2][256];

//static uint64_t hashTwistedTable[3][8][256];
//static uint64_t hashTwistedTableOp[2][256];




class vr_tabulation_hash{
public:

  template<class REALTYPE, int NB>
  static inline bool hashBool(__attribute__((unused)) const Vr_Rand * r,
			      const vr_packArg<REALTYPE,NB>& pack,
			      uint32_t hashOp){
    const uint32_t v=vr_tabulation_hash::hash(pack,hashOp);
    return v&1;
  }

  template<class REALTYPE, int NB>
  static inline double hashRatio(__attribute__((unused)) const Vr_Rand * r,
				 const vr_packArg<REALTYPE,NB>& pack,
				 uint32_t hashOp){
    const uint32_t v=vr_tabulation_hash::hash(pack,hashOp);
    constexpr double invMax=(1./ 4294967296.);
    return ((double)v *invMax );
  }


  static inline uint32_t hash(const vr_packArg<double,1>& pack,
			      uint32_t hashOp){
    uint32_t res=0;
    vr_tabulation_hash::hash_op(res, (uint16_t)hashOp);
    uint64_t a1=realToUint64_reinterpret_cast<double>(pack.arg1);
    vr_tabulation_hash::hash_aux(res, 0, a1);
    return res;
  }

  static inline uint32_t hash(const vr_packArg<float,1>& pack,
			      uint32_t hashOp){
    uint32_t res=0;
    vr_tabulation_hash::hash_op(res, (uint16_t)hashOp);
    uint32_t a1=realToUint32_reinterpret_cast(pack.arg1);
    vr_tabulation_hash::hash_aux(res, 0, a1);
    return res;
  }


  static inline uint32_t hash(const vr_packArg<double,2>& pack,
			      uint32_t hashOp){
    uint32_t res=0;
    vr_tabulation_hash::hash_op(res, (uint16_t)hashOp);
    uint64_t a1=realToUint64_reinterpret_cast<double>(pack.arg1);
    uint64_t a2=realToUint64_reinterpret_cast<double>(pack.arg2);
    vr_tabulation_hash::hash_aux(res, 0, a1);
    vr_tabulation_hash::hash_aux(res, 1, a2);
    return res;
  }

  static inline uint32_t hash(const vr_packArg<float,2>& pack,
			      uint32_t hashOp){
    uint32_t res=0;
    vr_tabulation_hash::hash_op(res, (uint16_t)hashOp);
    uint32_t a1=realToUint32_reinterpret_cast(pack.arg1);
    uint32_t a2=realToUint32_reinterpret_cast(pack.arg2);
    vr_tabulation_hash::hash_aux(res, 0, a1);
    vr_tabulation_hash::hash_aux(res, 1, a2);
    return res;
  }



  static inline uint32_t hash(const vr_packArg<double,3>& pack,
			      uint32_t hashOp){
    uint32_t res=0;
    vr_tabulation_hash::hash_op(res, (uint16_t)hashOp);
    uint64_t a1=realToUint64_reinterpret_cast<double>(pack.arg1);
    uint64_t a2=realToUint64_reinterpret_cast<double>(pack.arg2);
    uint64_t a3=realToUint64_reinterpret_cast<double>(pack.arg3);
    vr_tabulation_hash::hash_aux(res, 0, a1);
    vr_tabulation_hash::hash_aux(res, 1, a2);
    vr_tabulation_hash::hash_aux(res, 2, a3);
    return res;
  }


  static inline uint32_t hash(const vr_packArg<float,3>& pack,
			      uint32_t hashOp){
    uint32_t res=0;
    vr_tabulation_hash::hash_op(res, (uint16_t)hashOp);
    uint32_t a1=realToUint32_reinterpret_cast(pack.arg1);
    uint32_t a2=realToUint32_reinterpret_cast(pack.arg2);
    uint32_t a3=realToUint32_reinterpret_cast(pack.arg3);
    vr_tabulation_hash::hash_aux(res, 0, a1);
    vr_tabulation_hash::hash_aux(res, 1, a2);
    vr_tabulation_hash::hash_aux(res, 2, a3);
    return res;
  }


  static inline void  hash_op(uint32_t& h, uint16_t optEnum){
    uint32_t x(optEnum);
    uint32_t i;
    uint8_t c;
    for(i=0 ; i <2 ; i++){
      c=x;
      h^= hashTableOp[i][c];
      x = x >> 8;
    }
  }
  static inline void hash_aux(uint32_t& h, uint32_t index, uint64_t value){
    uint64_t x(value);
    uint32_t i;
    uint8_t c;
    for(i=0 ; i <8 ; i++){
      c=x;
      h^= hashTable[index][i][c];
      x = x >> 8;
    }
  }

  static inline void hash_aux(uint32_t& h, uint32_t index, uint32_t value){
    uint32_t x(value);
    uint32_t i;
    uint8_t c;
    for(i=0 ; i <4 ; i++){
      c=x;
      h^= hashTable[index][i][c];
      x = x >> 8;
    }
  }

  static inline void genTable(tinymt64_t& gen){
    for(int i=0 ; i< 4; i++){
      for(int j=0 ; j< 8; j++){
	for(int k=0; k<256 /2; k++ ){
	  uint64_t current= tinymt64_generate_uint64(&gen );
	  hashTable[i][j][2*k]= current;
	  hashTable[i][j][2*k+1]= current>>32;
	}
      }
    }
    for(int j=0 ; j< 2; j++){
      for(int k=0; k<256 /2; k++ ){
	uint64_t current= tinymt64_generate_uint64(&gen );
	hashTableOp[j][2*k]= current;
	hashTableOp[j][2*k+1]= current>>32;
      }
    }
  };

};




/*

class vr_twisted_tabulation_hash{
public:

  template<class REALTYPE, int NB>
  static inline bool hashBool(__attribute__((unused)) const Vr_Rand * r,
			      const vr_packArg<REALTYPE,NB>& pack,
			      uint32_t hashOp){
    const uint32_t v=vr_twisted_tabulation_hash::hash(pack,hashOp);
    return v&1;
  }

  template<class REALTYPE, int NB>
  static inline double hashRatio(__attribute__((unused)) const Vr_Rand * r,
				 const vr_packArg<REALTYPE,NB>& pack,
				 uint32_t hashOp){
    const uint32_t v=vr_twisted_tabulation_hash::hash(pack,hashOp);
    constexpr double invMax=(1./ 4294967296.);
    return ((double)v *invMax );
  }


  static inline uint32_t hash(const vr_packArg<double,1>& pack,
			      uint32_t hashOp){
    uint64_t res=0;
    uint64_t a1=realToUint64_reinterpret_cast<double>(pack.arg1);
    vr_twisted_tabulation_hash::hash_aux(res, 0, a1);
    vr_twisted_tabulation_hash::hash_op(res, (uint16_t)hashOp);
    return (uint32_t)res;
  }

  static inline uint32_t hash(const vr_packArg<float,1>& pack,
			      uint32_t hashOp){
    uint64_t res=0;
    uint32_t a1=realToUint32_reinterpret_cast(pack.arg1);
    vr_twisted_tabulation_hash::hash_aux(res, 0, a1);
    vr_twisted_tabulation_hash::hash_op(res, (uint16_t)hashOp);
    return (uint32_t)res;
  }


  static inline uint32_t hash(const vr_packArg<double,2>& pack,
			      uint32_t hashOp){
    uint64_t res=0;
    uint64_t a1=realToUint64_reinterpret_cast<double>(pack.arg1);
    uint64_t a2=realToUint64_reinterpret_cast<double>(pack.arg2);
    vr_twisted_tabulation_hash::hash_aux(res, 0, a1);
    vr_twisted_tabulation_hash::hash_aux(res, 1, a2);
    vr_twisted_tabulation_hash::hash_op(res, (uint16_t)hashOp);
    return (uint32_t)res;
  }

  static inline uint32_t hash(const vr_packArg<float,2>& pack,
			      uint32_t hashOp){
    uint64_t res=0;
    uint32_t a1=realToUint32_reinterpret_cast(pack.arg1);
    uint32_t a2=realToUint32_reinterpret_cast(pack.arg2);
    vr_twisted_tabulation_hash::hash_aux(res, 0, a1);
    vr_twisted_tabulation_hash::hash_aux(res, 1, a2);
    vr_twisted_tabulation_hash::hash_op(res, (uint16_t)hashOp);
    return (uint32_t)res;
  }



  static inline uint32_t hash(const vr_packArg<double,3>& pack,
			      uint32_t hashOp){
    uint64_t res=0;
    uint64_t a1=realToUint64_reinterpret_cast<double>(pack.arg1);
    uint64_t a2=realToUint64_reinterpret_cast<double>(pack.arg2);
    uint64_t a3=realToUint64_reinterpret_cast<double>(pack.arg3);
    vr_twisted_tabulation_hash::hash_aux(res, 0, a1);
    vr_twisted_tabulation_hash::hash_aux(res, 1, a2);
    vr_twisted_tabulation_hash::hash_aux(res, 2, a3);
    vr_twisted_tabulation_hash::hash_op(res, (uint16_t)hashOp);
    return (uint32_t)res;
  }


  static inline uint32_t hash(const vr_packArg<float,3>& pack,
			      uint32_t hashOp){
    uint64_t res=0;
    uint32_t a1=realToUint32_reinterpret_cast(pack.arg1);
    uint32_t a2=realToUint32_reinterpret_cast(pack.arg2);
    uint32_t a3=realToUint32_reinterpret_cast(pack.arg3);
    vr_twisted_tabulation_hash::hash_aux(res, 0, a1);
    vr_twisted_tabulation_hash::hash_aux(res, 1, a2);
    vr_twisted_tabulation_hash::hash_aux(res, 2, a3);
    vr_twisted_tabulation_hash::hash_op(res, (uint16_t)hashOp);
    return (uint32_t) res;
  }


  static inline void  hash_op(uint64_t& h, uint16_t optEnum){
    uint32_t x(optEnum);
    uint32_t i;
    uint8_t c;
    for(i=0 ; i <1 ; i++){
      c=x;
      h^= hashTwistedTableOp[i][c];
      x = x >> 8;
    }

    c=x^h;
    h^=hashTwistedTableOp[i][c];
    //    h>>=32;
    h>>=8;
  }
  static inline void hash_aux(uint64_t& h, uint32_t index, uint64_t value){
    uint64_t x(value);
    uint32_t i;
    uint8_t c;
    for(i=0 ; i <8 ; i++){
      c=x;
      h^= hashTwistedTable[index][i][c];
      x = x >> 8;
    }
  }

  static inline void hash_aux(uint64_t& h, uint32_t index, uint32_t value){
    uint32_t x(value);
    uint32_t i;
    uint8_t c;
    for(i=0 ; i <4 ; i++){
      c=x;
      h^= hashTwistedTable[index][i][c];
      x = x >> 8;
    }
  }

  static inline  void genTable(tinymt64_t& gen){
    for(int i=0 ; i< 3; i++){
      for(int j=0 ; j< 8; j++){
	for(int k=0; k<256; k++ ){
	  uint64_t current= tinymt64_generate_uint64(&gen );
	  hashTwistedTable[i][j][k]= current;
	}
      }
    }
    for(int j=0 ; j< 2; j++){
      for(int k=0; k<256; k++ ){
	uint64_t current= tinymt64_generate_uint64(&gen );
	hashTwistedTableOp[j][k]= current;
      }
    }
  };

};
*/

class vr_double_tabulation_hash{
public:

  template<class REALTYPE, int NB>
  static inline  bool hashBool(__attribute__((unused)) const Vr_Rand * r,
			       const vr_packArg<REALTYPE,NB>& pack,
			       uint32_t hashOp){
    const uint32_t tmp=vr_tabulation_hash::hash(pack,hashOp);
    uint32_t res=0;
    vr_tabulation_hash::hash_aux(res, 3, tmp);
    return res&1;
  }

  template<class REALTYPE, int NB>
  static inline double hashRatio(__attribute__((unused)) const Vr_Rand * r,
				 const vr_packArg<REALTYPE,NB>& pack,
				 uint32_t hashOp){
    const uint32_t tmp=vr_tabulation_hash::hash(pack,hashOp);
    uint32_t res=0;
    vr_tabulation_hash::hash_aux(res, 3, tmp);
    constexpr double invMax=(1./ 4294967296.); //2**32 = 4294967296
    return ((double)res *invMax );
  }
};
