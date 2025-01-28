/*
   This file is part of Verrou, a FPU instrumentation tool.

   Copyright (C) 2014-2021 EDF
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

#include "libverrouTask.h"
#define VERROU_TASK_INCLUDE
#include "verrou.h"
#include <map>
#include <string>
#include <vector>
#include <stdlib.h>
#include <algorithm>
#include <fstream>
#include <sstream>
#include <iostream>


#define outputPrefix "==libverrouTask.so== "

bool taskDebug=false;



bool generateTrace;
std::ofstream outputTraceFile;

bool generateTraceFP;
std::ofstream outputTraceFPFile;
std::string strNameOld("init unamed task");
int indexOld=0;
int countOld=0;

bool activeTask;
bool inactiveTask;
typedef std::vector< std::pair<std::string , int> >  storeTaskType;
storeTaskType activeTrace;
storeTaskType inactiveTrace;

bool stopStart_hard=true;


/*Private function prototype*/
void readDebug();

void verrou_task_generate_trace_init();
void verrou_task_generate_trace(const std::string& strName, int index);
void verrou_task_generate_trace_finalyze();

void verrou_task_generate_trace_fp_init();
void verrou_task_generate_trace_fp(const std::string& strName, int index);
void verrou_task_generate_trace_fp_finalyze();


void verrou_task_activation_init();
void verrou_task_activation(const std::string& strName, int index);
void verrou_task_activation_finalyze();

void fileToActiveInactiveStore(std::ifstream& input, storeTaskType& store);

/*Public function*/
void verrou_task_init(){
  readDebug();
  if(taskDebug){
    std::cerr << outputPrefix<<"task_init" <<std::endl;
    std::cerr << outputPrefix<<"\tfp     instr: " << VERROU_COUNT_FP_INSTRUMENTED<<std::endl;
    std::cerr << outputPrefix<<"\tfp not instr: " << VERROU_COUNT_FP_NOT_INSTRUMENTED<<std::endl;
  }
  verrou_task_activation_init();
  verrou_task_generate_trace_init();
  verrou_task_generate_trace_fp_init();

  verrou_task(strNameOld.c_str(),0);
}


void verrou_task(char const * const name, int index){
  std::string strName(name);
  if(taskDebug){
    std::cerr << outputPrefix<<"task: " <<name<< ":"<<index<<std::endl;
    std::cerr << outputPrefix<<"\tfp     instr: " << VERROU_COUNT_FP_INSTRUMENTED<<std::endl;
    std::cerr << outputPrefix<<"\tfp not instr: " << VERROU_COUNT_FP_NOT_INSTRUMENTED<<std::endl;
  }
  verrou_task_generate_trace(strName,index);
  verrou_task_generate_trace_fp(strName,index);
  verrou_task_activation(strName,index);
}

void verrou_task_finalyze(){
  verrou_task_generate_trace_finalyze();
  verrou_task_generate_trace_fp_finalyze();
  verrou_task_activation_finalyze();
}


//Private function

void readDebug()
  {//block debug
    char* debugStr;
    debugStr = getenv ("DEBUG_PRINT_TASK");
    if(debugStr==NULL){
      taskDebug=false;
    }else{
      std::cout<< outputPrefix<<"Debug activated"<<std::endl;
      std::cerr<< outputPrefix<<"Debug activated"<<std::endl;
      taskDebug=true;
    }
  }



void verrou_task_generate_trace_init()
{//block GENERATE TASK LIST
  char* fileNameList = getenv ("GENERATE_TASK_LIST");
  if (fileNameList==NULL){
      generateTrace=false;
  }else{
    std::cerr<< outputPrefix<<"Generate task list : init"<<std::endl;
    generateTrace=true;
    outputTraceFile.open(fileNameList, std::ios::out);
  }
}//fin block GENERATE TASK LIST


void verrou_task_generate_trace(const std::string& strName, int index){
  if(generateTrace){
    outputTraceFile << strName <<"\t"<<index <<std::endl;
    if(taskDebug){
      std::cerr <<outputPrefix<<"trace generation : "  << strName <<"\t"<<index <<std::endl;
    }
  }
}

void verrou_task_generate_trace_finalyze(){
  if(generateTrace){
    outputTraceFile.close();
  }
}



void verrou_task_generate_trace_fp_init(){
  char* fileNameFPList = getenv ("GENERATE_TASK_FP_LIST");
  if (fileNameFPList==NULL){
    generateTraceFP=false;
  }else{
    std::cerr<< outputPrefix <<"Generate task FP list : init"<<std::endl;
    generateTraceFP=true;
    outputTraceFPFile.open(fileNameFPList, std::ios::out);
  }
}

void verrou_task_generate_trace_fp(const std::string& strName, int index){
   if(generateTraceFP){
    long int count=VERROU_COUNT_FP_INSTRUMENTED;
    if(countOld!=count){
      outputTraceFPFile << strNameOld <<"\t"<<indexOld <<std::endl;
      if(taskDebug){
	std::cerr <<outputPrefix<<"trace FP generation : "  << strNameOld <<"\t"<<indexOld <<std::endl;
      }
    }
    strNameOld=strName;
    indexOld=index;
    countOld=count;
   }
}


void verrou_task_generate_trace_fp_finalyze(){
  if(generateTraceFP){
    long int count=VERROU_COUNT_FP_INSTRUMENTED;
    if(countOld!=count){
      outputTraceFPFile << strNameOld <<"\t"<<indexOld <<std::endl;
      if(taskDebug){
	std::cerr <<outputPrefix<<"trace FP generation : "  << strNameOld <<"\t"<<indexOld <<std::endl;
      }
    }

    outputTraceFPFile.close();
  }
}


void fileToActiveInactiveStore(std::ifstream& input, storeTaskType& store){
    std::string strLine;
    while (std::getline(input, strLine)){
      if(strLine[0]=='#'){
	continue;
      }
      std::stringstream sstream(strLine);
      std::string keyStr;
      getline(sstream,keyStr,'\t');
      int index;
      sstream >> index;

      std::pair<std::string,int> key(keyStr,index);
      if(std::find(store.begin(), store.end(), key)==store.end()){
	store.push_back(key);
      }
      if(taskDebug){
	std::cerr << outputPrefix<<"loading : " <<keyStr<< ":"<<index<<std::endl;
      }
    }
}

void verrou_task_activation_init(){
  char* stopOrStart;
  stopOrStart = getenv ("TASK_STOP_START");
  if(stopOrStart!=NULL){
    if(std::string(stopOrStart) == std::string("hard") ){
      stopStart_hard=true;
    }else{
      if(std::string(stopOrStart) == std::string("soft") ){
	stopStart_hard=false;
      }else{
	std::cerr << "TASK_STOP_START=" << stopOrStart << " not taken into account"<< std::endl;
      }
    }
  }


  char* fileNameList;
  fileNameList = getenv ("TASK_LIST");
  if (fileNameList==NULL){
    activeTask=false;
  }else{
    long int count=VERROU_COUNT_FP_INSTRUMENTED;
    if(count !=0){
      std::cerr << outputPrefix << "error : fp operation before verrou_task_init call" <<std::endl;
      std::cerr << outputPrefix << "advice : move verrou_task call or use --instr-atstart=no" <<std::endl;
    }

    activeTask=true;
    std::cerr<< outputPrefix<<"Loading task list"<<std::endl;

    std::ifstream inputTask(fileNameList, std::ios::in);

    if(inputTask.fail()){
      std::cerr<< outputPrefix<<"Fail loading file :"<< fileNameList<<std::endl;
    }
    fileToActiveInactiveStore(inputTask, activeTrace);

  }
  if(activeTask){
    fileNameList = getenv ("TASK_EXCLUDE_LIST");
    if (fileNameList==NULL){
      inactiveTask=false;
    }else{
      inactiveTask=true;
      std::cerr<< outputPrefix<<"Loading exclude task list"<<std::endl;
      std::ifstream inputTask(fileNameList, std::ios::in);
      if(inputTask.fail()){
	std::cerr<< outputPrefix<<"Fail loading file :"<< fileNameList<<std::endl;
      }
      fileToActiveInactiveStore(inputTask, inactiveTrace);
    }
  }
}


void verrou_task_activation(const std::string& strName, int index){
  /*Activation*/
  if(activeTask){
    std::pair<std::string, int> key(strName, index);
    bool isActivePair=!(std::find(activeTrace.begin(), activeTrace.end(), key)==activeTrace.end() );
    bool isInactivePair=!(std::find(inactiveTrace.begin(), inactiveTrace.end(), key)==inactiveTrace.end() );
    if(isInactivePair && isActivePair){
       if(taskDebug){
	std::cerr << outputPrefix<< strName << " " << index << " : coherence probleme detected" << std::endl;
      }

    }
    if(isActivePair){
      if(stopStart_hard){
	VERROU_START_INSTRUMENTATION;
      }else{
	VERROU_START_SOFT_INSTRUMENTATION;
      }
      if(taskDebug){
	std::cerr << outputPrefix<< strName << " " << index << " : activated" << std::endl;
      }
    }
    if((!inactiveTask&&(!isActivePair)) || (inactiveTask&&isActivePair)){
      if(stopStart_hard){
	VERROU_STOP_INSTRUMENTATION;
      }else{
	VERROU_STOP_SOFT_INSTRUMENTATION;
      }
      if(taskDebug){
	std::cerr << outputPrefix<< strName << " " << index << " : deactivated" << std::endl;
      }
    }
  }
}

void verrou_task_activation_finalyze(){
  if(stopStart_hard){
    VERROU_STOP_INSTRUMENTATION;
  }else{
    VERROU_STOP_SOFT_INSTRUMENTATION;
  }
  if(taskDebug){
    std::cerr << outputPrefix<< "Finalyse : deactivated" << std::endl;
  }
}
