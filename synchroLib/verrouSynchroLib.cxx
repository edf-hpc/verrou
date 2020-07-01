#include "verrouSynchroLib.h"
#define VERROU_SYNCHRO_INCLUDE
#include "verrou.h"
#include <map>
#include <string>
#include <vector>
#include <stdlib.h>
#include <algorithm>
#include <fstream>
#include <sstream>
#include <iostream>

bool synchroDebug=false;
std::string outputPrefix("==verrouSynchroLib.so== ");


bool generateTrace;
std::ofstream outputTraceFile;

bool generateTraceFP;
std::ofstream outputTraceFPFile;
std::string strNameOld("init unamed synchro");
int indexOld=0;
int countOld=0;

bool activeInactiveSynchro;
typedef std::vector< std::pair<std::string , int> >  storeSynchroType;
storeSynchroType activeInactiveTrace;



/*Private function prototype*/
void readDebug();

void verrou_synchro_generate_trace_init();
void verrou_synchro_generate_trace(const std::string& strName, int index);
void verrou_synchro_generate_trace_finalyze();

void verrou_synchro_generate_trace_fp_init();
void verrou_synchro_generate_trace_fp(const std::string& strName, int index);
void verrou_synchro_generate_trace_fp_finalyze();


void verrou_synchro_activation_init();
void verrou_synchro_activation(const std::string& strName, int index);
void verrou_synchro_activation_finalyze();

/*Public function*/
void verrou_synchro_init(){
  readDebug();

  verrou_synchro_activation_init();
  verrou_synchro_generate_trace_init();
  verrou_synchro_generate_trace_fp_init();
}


void verrou_synchro(char const * const name, int index){
  std::string strName(name);
  if(synchroDebug){
    std::cerr << outputPrefix<<"synchro: " <<name<< ":"<<index<<std::endl;
    std::cerr << "fp     instr: " << VERROU_COUNT_FP_INSTRUMENTED<<std::endl;
    std::cerr << "fp not instr: " << VERROU_COUNT_FP_NOT_INSTRUMENTED<<std::endl;
  }
  verrou_synchro_generate_trace(strName,index);
  verrou_synchro_generate_trace_fp(strName,index);
  verrou_synchro_activation(strName,index);
}

void verrou_synchro_finalyze(){
  verrou_synchro_generate_trace_finalyze();
  verrou_synchro_generate_trace_fp_finalyze();
  verrou_synchro_activation_finalyze();
}


//Private function

void readDebug()
  {//block debug
    char* debugStr;
    debugStr = getenv ("DEBUG_PRINT_SYNCHRO");
    if(debugStr==NULL){
      synchroDebug=false;
    }else{
      std::cerr<< outputPrefix<<"Debug activated"<<std::endl;
      synchroDebug=true;
    }
  }



void verrou_synchro_generate_trace_init()
{//block GENERATE SYNCHRO LIST
  char* fileNameList = getenv ("GENERATE_SYNCHRO_LIST");
  if (fileNameList==NULL){
      generateTrace=false;
  }else{
    std::cerr<< outputPrefix<<"Generating synchro list"<<std::endl;
    generateTrace=true;
    outputTraceFile.open(fileNameList, std::ios::out);
  }
}//fin block GENERATE SYNCHRO LIST


void verrou_synchro_generate_trace(const std::string& strName, int index){
  if(generateTrace){
    outputTraceFile << strName <<"\t"<<index <<std::endl;
    if(synchroDebug){
      std::cerr <<outputPrefix<<"trace generation : "  << strName <<"\t"<<index <<std::endl;
    }
  }
}

void verrou_synchro_generate_trace_finalyze(){
  if(generateTrace){
    outputTraceFile.close();
  }
}



void verrou_synchro_generate_trace_fp_init(){
  char* fileNameFPList = getenv ("GENERATE_SYNCHRO_FP_LIST");
  if (fileNameFPList==NULL){
    generateTraceFP=false;
  }else{
    std::cerr<< outputPrefix <<"Generating synchro FP list"<<std::endl;
    generateTraceFP=true;
    outputTraceFPFile.open(fileNameFPList, std::ios::out);
  }
}

void verrou_synchro_generate_trace_fp(const std::string& strName, int index){
   if(generateTraceFP){
    long int count=VERROU_COUNT_FP_INSTRUMENTED;
    if(countOld!=count){
      outputTraceFPFile << strNameOld <<"\t"<<indexOld <<std::endl;
      if(synchroDebug){
	std::cerr <<outputPrefix<<"trace FP generation : "  << strNameOld <<"\t"<<indexOld <<std::endl;
      }
    }
    strNameOld=strName;
    indexOld=index;
    countOld=count;
   }
}


void verrou_synchro_generate_trace_fp_finalyze(){
  if(generateTraceFP){
    long int count=VERROU_COUNT_FP_INSTRUMENTED;
    if(countOld!=count){
      outputTraceFPFile << strNameOld <<"\t"<<indexOld <<std::endl;
      if(synchroDebug){
	std::cerr <<outputPrefix<<"trace FP generation : "  << strNameOld <<"\t"<<indexOld <<std::endl;
      }
    }

    outputTraceFPFile.close();
  }
}




void verrou_synchro_activation_init(){
  char* fileNameList;
  fileNameList = getenv ("SYNCHRO_LIST");
  if (fileNameList==NULL){
    activeInactiveSynchro=false;
  }else{
    long int count=VERROU_COUNT_FP_INSTRUMENTED;
    if(count !=0){
      std::cerr << outputPrefix << "error : fp operation before verrou_synchro_init call" <<std::endl;
      std::cerr << outputPrefix << "advice : move verrou_synchro call or use --instr-atstart=no" <<std::endl;
    }

    std::cerr<< outputPrefix<<"Loading synchro list"<<std::endl;
    activeInactiveSynchro=true;
    std::ifstream inputSynchro(fileNameList, std::ios::in);

    if(inputSynchro.fail()){
      std::cerr<< outputPrefix<<"Fail loading file :"<< fileNameList<<std::endl;
    }

    std::string strLine;
    while (std::getline(inputSynchro, strLine)){
      if(strLine[0]=='#'){
	continue;
      }
      std::stringstream sstream(strLine);
      std::string keyStr;
      getline(sstream,keyStr,'\t');
      int index;
      sstream >> index;

      std::pair<std::string,int> key(keyStr,index);
      if(std::find(activeInactiveTrace.begin(), activeInactiveTrace.end(), key)==activeInactiveTrace.end()){
	activeInactiveTrace.push_back(key);
      }
      if(synchroDebug){
	std::cerr << outputPrefix<<"loading : " <<keyStr<< ":"<<index<<std::endl;
      }
    }
  }
}


void verrou_synchro_activation(const std::string& strName, int index){
  /*Activation*/
  if(activeInactiveSynchro){
    std::pair<std::string, int> key(strName, index);
    if(std::find(activeInactiveTrace.begin(), activeInactiveTrace.end(), key)==activeInactiveTrace.end()){
      VERROU_STOP_INSTRUMENTATION;
      if(synchroDebug){
	std::cerr << outputPrefix<< strName << " " << index << " : deactivated" << std::endl;
      }
    }else{
      VERROU_START_INSTRUMENTATION;
      if(synchroDebug){
	std::cerr << outputPrefix<< strName << " " << index << " : activated" << std::endl;
      }
    }
  }

}
void verrou_synchro_activation_finalyze(){
  VERROU_STOP_INSTRUMENTATION;
  if(synchroDebug){
    std::cerr << outputPrefix<< "Finalyse : deactivated" << std::endl;
  }
}
