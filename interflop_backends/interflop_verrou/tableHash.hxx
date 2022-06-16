#pragma once

static uint32_t hashTable[3][8][256];
static uint32_t hashTableOp[2][256];


class tabulationHash{
public:

  template<class REALTYPE, int NB>
  static bool hashBool(const vr_packArg<REALTYPE,NB>& pack,
		       uint32_t hashOp){
    const uint32_t v=tabulationHash::hash(pack,hashOp);
    return v&1;
  }

  template<class REALTYPE, int NB>
  static double hashRatio(const vr_packArg<REALTYPE,NB>& pack,
			    uint32_t hashOp){
    const uint32_t v=tabulationHash::hash(pack,hashOp);
    return ((double)v / (double)(4294967296) ); //2**32 = 4294967296
  }


  static uint32_t hash(const vr_packArg<double,1>& pack,
		       uint32_t hashOp){
    uint32_t res=0;
    tabulationHash::hash_op(res, (uint16_t)hashOp);
    uint64_t a1=realToUint64_reinterpret_cast<double>(pack.arg1);
    tabulationHash::hash_aux(res, 0, a1);
    return res;
  }

  static uint32_t hash(const vr_packArg<float,1>& pack,
		       uint32_t hashOp){
    uint32_t res=0;
    tabulationHash::hash_op(res, (uint16_t)hashOp);
    uint32_t a1=realToUint32_reinterpret_cast(pack.arg1);
    tabulationHash::hash_aux(res, 0, a1);
    return res;
  }


  static uint32_t hash(const vr_packArg<double,2>& pack,
		       uint32_t hashOp){
    uint32_t res=0;
    tabulationHash::hash_op(res, (uint16_t)hashOp);
    uint64_t a1=realToUint64_reinterpret_cast<double>(pack.arg1);
    uint64_t a2=realToUint64_reinterpret_cast<double>(pack.arg2);
    tabulationHash::hash_aux(res, 0, a1);
    tabulationHash::hash_aux(res, 1, a2);
    return res;
  }

  static uint32_t hash(const vr_packArg<float,2>& pack,
		       uint32_t hashOp){
    uint32_t res=0;
    tabulationHash::hash_op(res, (uint16_t)hashOp);
    uint32_t a1=realToUint32_reinterpret_cast(pack.arg1);
    uint32_t a2=realToUint32_reinterpret_cast(pack.arg2);
    tabulationHash::hash_aux(res, 0, a1);
    tabulationHash::hash_aux(res, 1, a2);
    return res;
  }



  static uint32_t hash(const vr_packArg<double,3>& pack,
		       uint32_t hashOp){
    uint32_t res=0;
    tabulationHash::hash_op(res, (uint16_t)hashOp);
    uint64_t a1=realToUint64_reinterpret_cast<double>(pack.arg1);
    uint64_t a2=realToUint64_reinterpret_cast<double>(pack.arg2);
    uint64_t a3=realToUint64_reinterpret_cast<double>(pack.arg3);
    tabulationHash::hash_aux(res, 0, a1);
    tabulationHash::hash_aux(res, 1, a2);
    tabulationHash::hash_aux(res, 2, a3);
    return res;
  }


  static uint32_t hash(const vr_packArg<float,3>& pack,
		       uint32_t hashOp){
    uint32_t res=0;
    tabulationHash::hash_op(res, (uint16_t)hashOp);
    uint32_t a1=realToUint32_reinterpret_cast(pack.arg1);
    uint32_t a2=realToUint32_reinterpret_cast(pack.arg2);
    uint32_t a3=realToUint32_reinterpret_cast(pack.arg3);
    tabulationHash::hash_aux(res, 0, a1);
    tabulationHash::hash_aux(res, 1, a2);
    tabulationHash::hash_aux(res, 2, a3);
    return res;
  }


  static void  hash_op(uint32_t& h, uint16_t optEnum){
    uint32_t x(optEnum);
    uint32_t i;
    uint8_t c;
    for(i=0 ; i <2 ; i++){
      c=x;
      h^= hashTableOp[i][c];
      x = x >> 8;
    }
  }
  static void hash_aux(uint32_t& h, uint32_t index, uint64_t value){
    uint64_t x(value);
    uint32_t i;
    uint8_t c;
    for(i=0 ; i <8 ; i++){
      c=x;
      h^= hashTable[index][i][c];
      x = x >> 8;
    }
  }

  static void hash_aux(uint32_t& h, uint32_t index, uint32_t value){
    uint32_t x(value);
    uint32_t i;
    uint8_t c;
    for(i=0 ; i <4 ; i++){
      c=x;
      h^= hashTable[index][i][c];
      x = x >> 8;
    }
  }

  static void genTable(tinymt64_t& gen){
    for(int i=0 ; i< 3; i++){
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
