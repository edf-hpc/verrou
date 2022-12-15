/*
   This file is part of Verrou, a FPU instrumentation tool.

   Copyright (C) 2014-2022 EDF
     B. Lathuilière <bruno.lathuiliere@edf.fr>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public License as
   published by the Free Software Foundation; either version 2.1 of the
   License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU Lesser General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307, USA.

   The GNU Lesser General Public License is contained in the file COPYING.
*/



#include <iostream>
#include <cstdlib>
#include <math.h>

template<class REAL> std::string typeName(){
  return std::string("unknown");
}

template<>
std::string typeName<long double>(){
  return std::string("long double");
}

template<>
std::string typeName<double>(){
  return std::string("double");
}
template<>
std::string typeName<float>(){
  return std::string("float");
}






template<class REALTYPE>
class testInc0d1{
 public:
  testInc0d1():
    size(1048576),//2**20
    step(0.1),
    init(0.)
  {
  }


  REALTYPE computeSeq(){    
    REALTYPE acc=init;
    for(int i=0; i<size; i++){
      acc+=step;
    }
    return acc;
  }

  REALTYPE computeRecurssiveTree(int threadOf=1024, int base=4){
    return init+computeTreeHelper(0, size, threadOf, base);
  }
  inline REALTYPE computeTreeHelper(int begin, int end, int threadOf, int base){
    if(begin==end){
      return 0;
    }
    if(begin+1==end){
      return step;
    }
    if(begin+threadOf> end){
      REALTYPE res=0;
      for(int i=begin; i<end;i++){
	res+=step;
      }
      return res;
    }
    
    if(end> begin){
      int step= (end-begin)/base;
      REALTYPE res=0;

      for(int i=0; i<base ; i++){
	int ibegin=begin+i*step;
	int inext=std::min(ibegin+step,end);
	res+= computeTreeHelper(ibegin,inext, threadOf, base);
      }
      return res;
    }

    return 0.;
  }

  void run(){
    REALTYPE resSeq=computeSeq();
    REALTYPE resSeq2=computeSeq();
    REALTYPE resRec=computeRecurssiveTree();
    REALTYPE resRec2=computeRecurssiveTree();

    std::cout.precision(16);
    std::cout <<"<"<< typeName<REALTYPE>()<<">"
	      <<"\tresSeq: " << resSeq
	      <<"\tresRec: " << resRec
	      <<"\tresSeqRatio: " << resSeq / resSeq2
	      <<"\tresRecRatio: " << resRec / resRec2
	      <<std::endl;
  }
 private:
  const int size;
  const REALTYPE step;
  const REALTYPE init;
};




int main(int argc, char** argv){
  
  testInc0d1 <double> t1d; t1d.run();
  testInc0d1 <float>  t1f; t1f.run();
  
  return EXIT_SUCCESS;
}



