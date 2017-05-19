
/*--------------------------------------------------------------------*/
/*--- Verrou: a FPU instrumentation tool.                          ---*/
/*--- Interface for random number generation.                      ---*/
/*---                                                  vr_rand.hxx ---*/
/*--------------------------------------------------------------------*/

/*
   This file is part of Verrou, a FPU instrumentation tool.

   Copyright (C) 2014-2016
     F. Févotte     <francois.fevotte@edf.fr>
     B. Lathuilière <bruno.lathuiliere@edf.fr>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307, USA.

   The GNU General Public License is contained in the file COPYING.
*/

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
    privatesrand(c);
    currentRand_=privaterand();
   };

  inline void setTimeSeed(unsigned int pid){
    unsigned int seed=/* time(NULL) + */ pid;
    VG_(umsg)("First seed : %u\n",seed);
    setSeed(seed);
  };


  inline bool getBool(){
     if(count_==nbReload){
       currentRand_=privaterand();
       count_=0;
     }
     const bool res(((currentRand_>>(count_++))&1));
     //     VG_(umsg)("Count : %u  res: %u\n",count_ ,res);
     return res;
   };

  inline bool getBoolNaive(){
    return privaterand()%2 ;
  };


  inline int getRandomInt(){
    return privaterand();
  };

  inline int getRandomIntMax()const{
    return privateRAND_MAX();
  };


private:
  int currentRand_;
  int count_;


  unsigned long int  vr_next_;

  inline void privatesrand(unsigned int c){
    //    srand(c);
    vr_next_=c;
  }
  
  inline int privaterand(){
    //    rand();
    vr_next_ = vr_next_ * 1103515245 + 12345;
    return (unsigned int)(vr_next_/65536) % 32768;
  }
  
  inline int privateRAND_MAX()const{
    //    return RAND_MAX;
    return 32767;
  }

};
