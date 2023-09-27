#include <iostream>


typedef double Realtype;

Realtype muller(size_t nt,bool verbose=false, bool withEmptyLine=false, bool debugPrev=false, bool debugPost=false){
    Realtype x0 = 11./2.;
    Realtype x1 = 61./11.;

    std::cout << "begin iter"<<std::endl;
    for(size_t it=0; it < nt ; it++){
        Realtype temp0 = 3000./x0;
        Realtype temp1 = 1130. - temp0;
        Realtype temp2 = temp1 /x1 ;

        Realtype x2 = 111. - temp2;
	if(debugPrev){
	  std::cout << "debugPrev"<<std::endl;
	}

        if(verbose){
           std::cout <<"it: "<< it
                     << "\tx2: "<<x2
                     <<"\ttemp0: "<<temp0//<< std::flush
                     <<"\ttemp1: "<<temp1
                     <<"\ttemp2: "<<temp2 
                     <<"addr: "<< &temp1
                     << std::endl<< std::flush;
        }
	if(withEmptyLine){
	  std::cout << std::endl<< std::flush;
	}
	if(debugPost){
	  std::cout << "debugPost"<<std::endl;
	}

        x0 = x1;
        x1 = x2;
    }
    std::cout <<"x["<<nt<<"]="<<x1<<std::endl;
    return x1;
}

int main(int argc, char** argv){

  bool withEmptyLine(false);
  bool debugPrev(false);
  bool debugPost(false);
  for(size_t i=1; i< argc; i++){
    if(std::string(argv[i])==std::string("emptyLine")){
      withEmptyLine=true;
      continue;
    }
    if(std::string(argv[i])==std::string("debugPrev")){
      debugPrev=true;
      continue;
    }
    if(std::string(argv[i])==std::string("debugPost")){
      debugPost=true;
      continue;
    }
  }
  muller(12,true,withEmptyLine, debugPrev, debugPost);
  //  muller(3000,true,withEmptyLine, debugPrev, debugPost);

}
