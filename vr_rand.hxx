#pragma once


class vrRand{
public:
  static const int nbReload=31;
   vrRand():count_(0){    
     // WARNING this constructor is never called :  
   };
  
  //~vrRand(){}; This line is commented because of link problem

  inline void setSeed(unsigned int c){
    count_=0;
    srand(c);
    currentRand_=rand();
   };

  inline void setTimeSeed(){
    unsigned int seed=time(NULL);
    VG_(umsg)("First seed : %u\n",seed );
    srand(time(NULL));
    count_=0;
    currentRand_=rand();
   };


  inline bool getBool(){
     if(count_==nbReload){
       currentRand_=rand();
       count_=0;
     }
     const bool res(((currentRand_>>(count_++))&1));
     //     VG_(umsg)("Count : %u  res: %u\n",count_ ,res);
     return res;
   };

  inline bool getBoolNaive(){
    return rand()%2 ;
  };
  
  
  inline int getRandomInt(){
    return rand();
  };

  inline int getRandomIntMax()const{
    return RAND_MAX;
  };
  

private:
  int currentRand_;
  int count_;
};
