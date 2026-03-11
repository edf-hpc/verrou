/*--------------------------------------------------------------------*/
/*--- Verrou: a FPU instrumentation tool.                          ---*/
/*--- This file contains code allowing to apply client request     ---*/
/*--- from an expect script.                                       ---*/
/*---                                             vr_IOMatch_clr.c ---*/
/*--------------------------------------------------------------------*/

/*
   This file is part of Verrou, a FPU instrumentation tool.

   Copyright (C) 2014-2023
     B. Lathuilière <bruno.lathuiliere@edf.fr>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 3 of the
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

#include "vr_main.h"
#include "vr_IOMatch_clr.h"

#include "pub_tool_seqmatch.h"
#include "pub_tool_libcfile.h"

#define STDIN_FILENO 0
#define STDOUT_FILENO 1


VgFile* vr_IOmatchCLRFileLog;
VgFile* vr_IOmatchCLRFileStdout;
VgFile* vr_IOmatchCLRFileFilteredStdout;

#define LINE_SIZEMAX 40000
#define FILTER_SIZEMAX 512


const HChar vr_defaultKeyStr[]="default: ";
SizeT vr_defaultKeyStrSize=sizeof(vr_defaultKeyStr)-1;

const HChar vr_initKeyStr[]="init: ";
SizeT vr_initKeyStrSize=sizeof(vr_initKeyStr)-1;

const HChar vr_postinitKeyStr[]="post-init: ";
SizeT vr_postinitKeyStrSize=sizeof(vr_postinitKeyStr)-1;


const HChar vr_filterLineExecKeyStr[]="filter_line_exec: ";
SizeT vr_filterLineExecKeyStrSize=sizeof(vr_filterLineExecKeyStr)-1 ;

const HChar vr_cmatchKeyStr[]= "cmatch: ";
SizeT vr_cmatchKeyStrSize=sizeof(vr_cmatchKeyStr)-1;

const HChar vr_bmatchKeyStr[]= "bmatch: ";
SizeT vr_bmatchKeyStrSize=sizeof(vr_bmatchKeyStr)-1;


const HChar vr_applyKeyStr[]=  "apply: ";
SizeT vr_applyKeyStrSize=sizeof(vr_applyKeyStr)-1;

const HChar vr_postApplyKeyStr[]=  "post-apply: ";
SizeT vr_postApplyKeyStrSize=sizeof(vr_postApplyKeyStr)-1;


const HChar vr_ignoreEmptyLineKeyStr[]=  "ignore-empty-line: ";
SizeT vr_ignoreEmptyLineKeyStrSize=sizeof(vr_ignoreEmptyLineKeyStr)-1;


Bool ignoreEmptyLine=True;

const HChar vr_verboseKeyStr[]=  "verbose: ";
SizeT vr_verboseKeyStrSize=sizeof(vr_verboseKeyStr)-1;

const HChar vr_logLevelKeyStr[]=  "log-level: ";
SizeT vr_logLevelKeyStrSize=sizeof(vr_logLevelKeyStr)-1;

const HChar vr_nbMatchMaxKeyStr[]=  "nb-match-max: ";
SizeT vr_nbMatchMaxKeyStrSize=sizeof(vr_nbMatchMaxKeyStr)-1;

const HChar vr_permutableBmatchIndexesKeyStr[]=  "permutable-bmatch-indexes: ";
SizeT vr_permutableBmatchIndexesKeyStrSize=sizeof(vr_permutableBmatchIndexesKeyStr)-1;


const HChar vr_dumpStdoutKeyStr[]=  "dump-stdout:";//no space : no param
SizeT vr_dumpStdoutKeyStrSize=sizeof(vr_dumpStdoutKeyStr)-1;
Bool vr_dumpStdout=False;

const HChar vr_dumpFilteredStdoutKeyStr[]=  "dump-filtered-stdout:";//no space : no param
SizeT vr_dumpFilteredStdoutKeyStrSize=sizeof(vr_dumpFilteredStdoutKeyStr)-1;
Bool vr_dumpFilteredStdout=False;


#define DEFAULT_MAX 10
#define DEFAULT_SIZE_MAX 30
HChar vr_applyDefault[DEFAULT_MAX][DEFAULT_SIZE_MAX];
SizeT vr_nbDefault=0;

HChar vr_applyInit[DEFAULT_MAX][DEFAULT_SIZE_MAX];
SizeT vr_nbInit=0;

HChar vr_applypostInit[DEFAULT_MAX][DEFAULT_SIZE_MAX];
SizeT vr_nbpostInit=0;
SizeT vr_countPostInit=0;

static int vr_nb_match_max=100;

#define APPLY_PER_MATCH_MAX 5
#define POST_APPLY_PER_MATCH_MAX 2
#define MATCH_SIZE_MAX 30

typedef struct vr_match_data {
   HChar* match_pattern;
   Bool  is_break_match_pattern;
   Bool  is_exact_match_pattern;
   SizeT count_match;
   HChar apply_match[APPLY_PER_MATCH_MAX][MATCH_SIZE_MAX];
   SizeT nb_apply_match;
   HChar post_apply_match[APPLY_PER_MATCH_MAX][MATCH_SIZE_MAX];
   SizeT nb_post_apply_match;
} vr_match_data_t;

vr_match_data_t* vr_match_tab=NULL;
SizeT vr_nbMatch=0;
SizeT vr_reorgBegin=1;
SizeT vr_reorgEnd=0;
SizeT vr_lastReorgMatch=0;

static inline void vr_allocate_match_if_needed(void){
   if(vr_match_tab==NULL){
      vr_match_tab=VG_(malloc)("vr_allocate_match_if_needed", vr_nb_match_max*sizeof(vr_match_data_t));
      if(vr_match_tab==NULL){
         VG_(tool_panic)("vr_IOmatch_clr : allocation failed");
      }
   }
}


Int previousMatchIndex=-1;

HChar* vr_IOmatch_CmdLine =NULL;
HChar* vr_writeLineBuff =NULL;
HChar* vr_writeLineBuffCurrent =NULL;

//SizeT vr_last_expect_lineNo=0;

Bool vr_filter=False;
HChar vr_filter_cmd[FILTER_SIZEMAX];
Int filter_fdin[2];
Int filter_fdout[2];
Int filter_pid;
HChar* vr_filtered_buff;

#define ARGV_FILTER_MAX 10
const HChar *argvFiltered[ARGV_FILTER_MAX];


const HChar nopStr[]="nop";
const HChar emptyStr[]="";
const HChar defaultStr[]="default";
const HChar initStr[]="init";
const HChar postinitStr[]="post-init";
const HChar stopStr[]="stop";
const HChar startStr[]="start";
const HChar stopSoftStr[]="stop_soft";
const HChar startSoftStr[]="start_soft";
const HChar displayCounterStr[]="display_counter";
const HChar nbInstrStr[]="nb_instr";
const HChar resetCounterStr[]="reset_counter";
const HChar dumpCoverStr[]="dump_cover";
const HChar panicStr[]="panic";
const HChar exitStr[]="exit";
const HChar denormCounterStr[]="print_denorm_counter";
const HChar resetDenormCounterStr[]="reset_denorm_counter";
const HChar backtraceStr[]="backtrace";

typedef enum {nopKey=0, emptyKey, defaultKey, initKey, postinitKey, stopKey, startKey, stopSoftKey, startSoftKey,displayCounterKey, nbInstrKey, resetCounterKey, dumpCoverKey, panicKey, exitKey, denormCounterKey,resetDenormCounterKey,backtraceKey} Vr_applyKey;
static const SizeT actionNumber=18;
const HChar* actionStrTab[]={nopStr, emptyStr, defaultStr, initStr, postinitStr, stopStr, startStr, stopSoftStr, startSoftStr, displayCounterStr, nbInstrStr, resetCounterStr, dumpCoverStr, panicStr, exitStr, denormCounterStr, resetDenormCounterStr,backtraceStr};
SizeT actionSizeTab[]={sizeof(nopStr), sizeof(emptyStr),sizeof(defaultStr), sizeof(initStr),  sizeof(postinitStr), sizeof(stopStr), sizeof(startStr),sizeof(stopSoftStr), sizeof(startSoftStr), sizeof(displayCounterStr), sizeof(nbInstrStr), sizeof(resetCounterStr), sizeof(dumpCoverStr),sizeof(panicStr),sizeof(exitStr), sizeof(denormCounterStr), sizeof(resetDenormCounterStr),sizeof(backtraceStr)};

//Bool actionRequireCacheCleanTab[]={False, False, False, False, False, True, True, False, False, False, False, False, False };

UInt IOMatch_verbose=1;
UInt IOMatch_log_level=1;

static Vr_applyKey vr_CmdToEnum(const HChar* cmd){

  for(SizeT i=0 ; i< actionNumber; i++){
    if(VG_(strncmp)(cmd, actionStrTab[i],actionSizeTab[i])==0){
      return i;
    }
  }
  VG_(umsg)("vr_CmdToEnum unknown cmd : |%s|\n", cmd);
  VG_(tool_panic)("invalid IOMatch script");
}

static
Bool vr_valid_apply_cmd(const HChar* cmd){
   /*Function to known if a cmd is valid : useful to get fast error with cmd error for default*/
   Bool res=False;
   for(SizeT i=0 ; i< actionNumber; i++){
     res= res || (VG_(strncmp)(cmd, actionStrTab[i],actionSizeTab[i])==0);
   }
   return res;
}


/*to avoid side effect we duplicate the code of get_char and get_line*/

#define my_assert(expr)                                                 \
  ((void) (LIKELY(expr) ? 0 :                                           \
           (VG_(assert_fail) (/*isCore*/True, #expr,                    \
                              __FILE__, __LINE__, __PRETTY_FUNCTION__,  \
                              ""),                                      \
                              0)))

#define my_assert2(expr, format, args...)                               \
  ((void) (LIKELY(expr) ? 0 :                                           \
           (VG_(assert_fail) (/*isCore*/True, #expr,                    \
                              __FILE__, __LINE__, __PRETTY_FUNCTION__,  \
                              format, ##args),                          \
                              0)))




/*Remarks : get_char and VG_(get_line) are reimplemented because of a side-effect
of VG_(get_line) which implies to fully read a file before to read an other. Moreover VG_(get_line)
has specificities for suppression file

*/

static Int my_get_char ( Int fd, HChar* out_buf )
{
   Int r;
   static HChar buf[256];
   static Int buf_size = 0;
   static Int buf_used = 0;
   my_assert(buf_size >= 0 && buf_size <= sizeof buf);
   my_assert(buf_used >= 0 && buf_used <= buf_size);
   if (buf_used == buf_size) {
      r = VG_(read)(fd, buf, sizeof buf);
      if (r < 0) return r; /* read failed */
      my_assert(r >= 0 && r <= sizeof buf);
      buf_size = r;
      buf_used = 0;
   }
   if (buf_size == 0)
     return 0; /* eof */
   my_assert(buf_size >= 0 && buf_size <= sizeof buf);
   my_assert(buf_used >= 0 && buf_used < buf_size);
   *out_buf = buf[buf_used];
   buf_used++;
   return 1;
}


// Get a non blank non comment line.
// Returns True if eof.
static Bool get_fullnc_line ( Int fd, HChar** bufpp)
{
   // VG_(get_line) adapted
   HChar* buf  = *bufpp;

   //Loop over the line until valid line
   while (True) {
      HChar  ch=0;
      SizeT  n; //error of my_get_char
      SizeT i;  // index for characters buffer
      Bool isBlank=True;

      /* Now, read the line into buf. */
      i = 0;

      //Loop over the char until end of line
      while (True) {
         n = my_get_char(fd, &ch);

         if (n <= 0){
            if(i==0 || buf[0]=='#' || isBlank){
               return True;
            }else{
               buf[i] = 0;
               return False;
            }
         }
         if (i > 0 && i == LINE_SIZEMAX-1) {
            VG_(umsg)("too large line\n");
            VG_(exit)(1);
         }

         if (ch == '\n'){
            buf[i]=0;
            break;
         }else{
            buf[i] = ch;
            buf[i+1] = 0;
         }
         if( (ch!=' ') && ((ch!='\t'))){
            isBlank=False;
         }
         i++;
      }
      /* Ok, we have a line.  If a non-comment line, return.
         If a comment line, start all over again. */
      if ( (buf[0] != '#') && (isBlank==False) ){
         return False;
      }
   }
}


inline static Bool vr_containWildCard(const HChar* matchString){
   int i=0;
   while(matchString[i]!=(HChar)0){
      if(matchString[i]=='*' || matchString[i]=='?'){
         return True;
      }
      i++;
   }
   return False;
}

inline static Bool vr_string_match(SizeT matchIndex, const HChar* filtered_line){
   if(vr_match_tab[matchIndex].is_exact_match_pattern){
      return (VG_(strcmp)(vr_match_tab[matchIndex].match_pattern,filtered_line)==0);
   }else{
      return VG_(string_match)(vr_match_tab[matchIndex].match_pattern,filtered_line);
   }
}


static void vr_applyCmd(Vr_applyKey key, const HChar* cmd,  Bool noIntrusiveOnly){
  if(IOMatch_verbose>2){
     VG_(umsg)("vr_applyCmd : %s\n", cmd);
  }
  switch(key){
  case nopKey:
    return;
  case emptyKey:
    return;
  case defaultKey:
    VG_(tool_panic)("default treated before");
    return;
  case initKey:
    VG_(tool_panic)("init treated before");
    return;
  case postinitKey:
    VG_(tool_panic)("postinit treated before");
    return;
  case stopKey:
    if(noIntrusiveOnly){
      vr.instrument_hard = False;
    }else{
      vr_set_instrument_state ("IO Match CLR", False, True);
    }
    if( IOMatch_log_level>0 ){
       VG_(fprintf)(vr_IOmatchCLRFileLog,"apply: stop\n");
    }
    return;
  case startKey:
    if(noIntrusiveOnly){
      vr.instrument_hard = True;
    }else{
      vr_set_instrument_state ("IO Match CLR", True, True);
    }
    if( IOMatch_log_level>0 ){
       VG_(fprintf)(vr_IOmatchCLRFileLog,"apply: start\n");
    }
    return;
  case stopSoftKey:
    vr_set_instrument_state ("IO Match CLR", False, False);
    if( IOMatch_log_level>0 ){
       VG_(fprintf)(vr_IOmatchCLRFileLog,"apply: stop soft\n");
    }
    return;
  case startSoftKey:
    vr_set_instrument_state ("IO Match CLR", True, False);
    if( IOMatch_log_level>0 ){
       VG_(fprintf)(vr_IOmatchCLRFileLog,"apply: start soft\n");
    }
    return;
  case displayCounterKey:
    vr_ppOpCount();
    if( IOMatch_log_level>0 ){
       VG_(fprintf)(vr_IOmatchCLRFileLog,"apply: display_counter\n");
    }
    return;
  case nbInstrKey:
  {
     UInt nbInstr=vr_count_fp_instrumented();
     if( IOMatch_log_level>0 ){
        VG_(fprintf)(vr_IOmatchCLRFileLog,"fp_instr: %u\n", nbInstr );
     }
     return;
  }
  case resetCounterKey:
  {
     vr_resetCount();
     return;
  }
  case dumpCoverKey:
    {
      SizeT ret;
      ret=vr_traceBB_dumpCov();
      if( IOMatch_log_level>0 ){
         VG_(fprintf)(vr_IOmatchCLRFileLog,"apply: dump_cover : %lu\n", ret);
      }
      return;
    }
  case denormCounterKey:
  {
     vr_print_denorm_counter();
     return;
  }
  case resetDenormCounterKey:
  {
     vr_reset_denorm_counter();
     return;
  }
  case backtraceKey:
  {
     VG_(get_and_pp_StackTrace)(VG_(get_running_tid)(), VG_(clo_backtrace_size));
     return;
  }

  case panicKey:
    {
      VG_(fprintf)(vr_IOmatchCLRFileLog, "apply: panic\n");
      VG_(tool_panic)("apply: panic");
    }
  case exitKey:
    {
      if( IOMatch_log_level>0 ){
          VG_(fprintf)(vr_IOmatchCLRFileLog, "apply: exit\n");
      }
      if(vr_filter){
         char msgEnd[]="";
         VG_(write)(filter_fdin[1], msgEnd, 1);

         VG_(close)(filter_fdout[0]);
         VG_(close)(filter_fdin[1]);
         VG_(waitpid)(filter_pid, NULL, 0);

         VG_(free)(vr_filtered_buff);
      }

      VG_(exit)(1);
    }
  }
  VG_(umsg)("vr_applyCmd :  unknown cmd : |%s|\n", cmd);
  VG_(exit)(1);
}




static
void vr_IOmatch_apply_clr(const HChar* cmd, Bool noIntrusiveOnly){
   if(IOMatch_verbose>2){
      VG_(umsg)("vr_IOmatch_apply_clr: %s\n",cmd);
   }

  Vr_applyKey key=vr_CmdToEnum(cmd);

  if(key==initKey){
    for(SizeT i=0; i< vr_nbInit;i++){
      Vr_applyKey keyInit=vr_CmdToEnum(vr_applyInit[i]);
      vr_applyCmd(keyInit,vr_applyInit[i], noIntrusiveOnly);
    }
    return;
  }
  if(key==postinitKey){
    for(SizeT i=0; i< vr_nbpostInit;i++){
      Vr_applyKey keypostInit=vr_CmdToEnum(vr_applypostInit[i]);
      vr_applyCmd(keypostInit,vr_applypostInit[i],noIntrusiveOnly);
    }
    return;
  }

  if(key==defaultKey){
    for(SizeT i=0; i< vr_nbDefault;i++){
      Vr_applyKey keyDefault=vr_CmdToEnum(vr_applyDefault[i]);
      vr_applyCmd(keyDefault,vr_applyDefault[i],noIntrusiveOnly);
    }
    return;
  }

  vr_applyCmd(key,cmd,noIntrusiveOnly);

}




VgFile* openOutputIOMatchFile(const HChar * fileName, const HChar * fileNameIOMatch, const HChar * strPost);
VgFile* openOutputIOMatchFile(const HChar * fileName, const HChar * fileNameIOMatch, const HChar * strPost){
  /*Open output File*/
  HChar strFilename[512];
  if( VG_(strncmp)(fileName, "",512)==0){
     if(vr.outputIOMatchRep==NULL){
        VG_(sprintf)(strFilename, "%s%s",fileNameIOMatch,strPost);
     }else{
        VG_(sprintf)(strFilename, "%s/%s%s",vr.outputIOMatchRep,"IOMatch",strPost);
     }
  }else{
     if(vr.outputIOMatchRep==NULL){
        VG_(sprintf)(strFilename, "%s",fileName);
     }else{
        if(fileName[0]=='/'){
           VG_(umsg)("incompatible option --output-expect-rep with abspath : %s", fileName);
           VG_(tool_panic)("incompatible option --output-expect-rep with abspath");
        }
        VG_(sprintf)(strFilename, "%s/%s", vr.outputIOMatchRep, fileName);
     }
  }

  const HChar * strFilenameExpanded=   VG_(expand_file_name)(strPost,  strFilename);

  VG_(umsg)("Open IOMatchClr output file : `%s'... \n", strFilenameExpanded);
  VgFile* vr_IOmatchCLRFile = VG_(fopen)(strFilenameExpanded,
					   VKI_O_WRONLY | VKI_O_CREAT | VKI_O_TRUNC,
					   VKI_S_IRUSR|VKI_S_IWUSR|VKI_S_IRGRP|VKI_S_IROTH);

  if(vr_IOmatchCLRFile==NULL){
      VG_(umsg)("ERROR (fopen) %s\n",strFilenameExpanded);
      VG_(tool_panic)("vr_IOmatch_clr : missing write file");
  }
  return vr_IOmatchCLRFile;
};


HChar* stripSpace(HChar* str);
HChar* stripSpace(HChar* str){
   if(str[0]==0) return str;

   HChar* res=str;
   while(res[0]==' ') res+=1;

   HChar* end=res;
   while(*end!=0) end+=1;

   end--;
   while(*end==' '){
      *end=0;
      end-=1;
   }
   return res;
}

void vr_IOmatch_clr_init (const HChar * fileName) {
  /*Open input File*/
  VG_(umsg)("Open IOMatchClr file : `%s'... \n", fileName);
  vr.IOMatchCLRFileInput= VG_(fd_open)(fileName, VKI_O_RDONLY,0);

  if (vr.IOMatchCLRFileInput == -1) {
    VG_(umsg)("ERROR (open)\n");
    VG_(exit)(1);
  }

  /*Open output File*/
  vr_IOmatchCLRFileLog=openOutputIOMatchFile("",fileName, ".log-%p");

  /*Allocate requiered Buffer*/
  vr_IOmatch_CmdLine = VG_(malloc)("vr_IOmatch_cl_init.1", LINE_SIZEMAX*sizeof(HChar));
  vr_writeLineBuff = VG_(malloc)("vr_IOmatch_cl_init.2", LINE_SIZEMAX*sizeof(HChar));
  vr_writeLineBuff[0]=0;
  vr_writeLineBuffCurrent= vr_writeLineBuff;

  /*Loop over input line until first expect key*/

  while (! get_fullnc_line(vr.IOMatchCLRFileInput, &vr_IOmatch_CmdLine)) {

     if(IOMatch_verbose>3){
        VG_(umsg)("debug line : %s\n", vr_IOmatch_CmdLine);
     }

     //Treat default key
    if( VG_(strncmp)(vr_IOmatch_CmdLine, vr_defaultKeyStr, vr_defaultKeyStrSize)==0  ){
       const HChar* defaultAction=stripSpace(vr_IOmatch_CmdLine+vr_defaultKeyStrSize);
       if(IOMatch_verbose>2){
          VG_(umsg)("default action str : %s\n", defaultAction);
       }
       if (vr_valid_apply_cmd(defaultAction)){
         if(IOMatch_log_level>0){
             VG_(fprintf)(vr_IOmatchCLRFileLog, "default action [%lu] : %s\n",vr_nbDefault ,defaultAction);
         }
	 VG_(strncpy)(vr_applyDefault[vr_nbDefault],defaultAction, DEFAULT_SIZE_MAX);
	 vr_nbDefault++;
       }else{
          VG_(umsg)("default action %s is not valid", defaultAction);
          VG_(tool_panic)("vr_IOmatch_clr : invalid action");
       }
       continue;
    }

     //Treat ignoreEmptyLine key
    if( VG_(strncmp)(vr_IOmatch_CmdLine, vr_ignoreEmptyLineKeyStr, vr_ignoreEmptyLineKeyStrSize)==0  ){
       const HChar* boolStr=stripSpace(vr_IOmatch_CmdLine+vr_ignoreEmptyLineKeyStrSize);
       const HChar trueStr[]="true";
       const HChar falseStr[]="false";
       if( VG_(strncmp)(boolStr,trueStr, sizeof(trueStr))==0){
	 ignoreEmptyLine=True;
	 VG_(umsg)("ignoreEmptyLine=True\n");
       }else{
	 if( VG_(strncmp)(boolStr,falseStr, sizeof(falseStr))==0){
	   ignoreEmptyLine=False;
	   VG_(umsg)("ignoreEmptyLine=False\n");
	 }else{
	   VG_(umsg)("ignore-empty-line %s is not valid", boolStr);
	   VG_(tool_panic)("ignore-empty-line %s is not valid");
	 }
       }

       if(IOMatch_verbose>2){
          VG_(umsg)("ignore-empty-line: %s\n", boolStr);
       }
       continue;
    }

     //Treat init key
    if( VG_(strncmp)(vr_IOmatch_CmdLine, vr_initKeyStr, vr_initKeyStrSize)==0  ){
       const HChar* initAction=stripSpace(vr_IOmatch_CmdLine+vr_initKeyStrSize);
       if(IOMatch_verbose>2){
          VG_(umsg)("init action str : %s\n", initAction);
       }
       if (vr_valid_apply_cmd(initAction)){
         if( IOMatch_log_level>0){
            VG_(fprintf)(vr_IOmatchCLRFileLog, "init action [%lu] : %s\n",vr_nbInit , initAction);
         }
         VG_(strncpy)(vr_applyInit[vr_nbInit],initAction, DEFAULT_SIZE_MAX);
	 vr_nbInit++;
       }else{
          VG_(umsg)("init action %s is not valid", initAction);
          VG_(tool_panic)("vr_IOmatch_clr : invalid action");
       }
       continue;
    }

         //Treat postinit key
    if( VG_(strncmp)(vr_IOmatch_CmdLine, vr_postinitKeyStr, vr_postinitKeyStrSize)==0  ){
       const HChar* postinitAction=stripSpace(vr_IOmatch_CmdLine+vr_postinitKeyStrSize);
       if(IOMatch_verbose>2){
          VG_(umsg)("post-init action str : %s\n", postinitAction);
       }
       if (vr_valid_apply_cmd(postinitAction)){
         if( IOMatch_log_level>0){
            VG_(fprintf)(vr_IOmatchCLRFileLog, "postinit action [%lu] : %s\n",vr_nbpostInit , postinitAction);
         }
	 VG_(strncpy)(vr_applypostInit[vr_nbpostInit], postinitAction, DEFAULT_SIZE_MAX);
	 vr_nbpostInit++;
       }else{
          VG_(umsg)("post init action %s is not valid", postinitAction);
          VG_(tool_panic)("vr_IOmatch_clr : invalid action");
       }
       continue;
    }

    //Treat bmatch key
    if( VG_(strncmp)(vr_IOmatch_CmdLine, vr_bmatchKeyStr, vr_bmatchKeyStrSize)==0 ){
      vr_allocate_match_if_needed();
      if(vr_nbMatch> vr_nb_match_max){
	VG_(tool_panic)("vr_IOmatch_clr : to many match");
      }
      const HChar* matchPattern=vr_IOmatch_CmdLine+vr_bmatchKeyStrSize;
      if( IOMatch_log_level>0){
         VG_(fprintf)(vr_IOmatchCLRFileLog, "bmatch pattern [%lu] : %s\n",vr_nbMatch ,matchPattern);
      }
      vr_match_tab[vr_nbMatch].match_pattern=VG_(strdup)("bmatch.patterndup", matchPattern);
      vr_match_tab[vr_nbMatch].is_break_match_pattern=True;
      vr_match_tab[vr_nbMatch].is_exact_match_pattern=!(vr_containWildCard(matchPattern));
      vr_match_tab[vr_nbMatch].count_match=0;
      vr_match_tab[vr_nbMatch].nb_apply_match=0;
      vr_match_tab[vr_nbMatch].nb_post_apply_match=0;
      vr_nbMatch++;
      continue;
    }
    if( VG_(strncmp)(vr_IOmatch_CmdLine, vr_cmatchKeyStr, vr_cmatchKeyStrSize)==0 ){
       vr_allocate_match_if_needed();
       if(vr_nbMatch> vr_nb_match_max){
	VG_(tool_panic)("vr_IOmatch_clr : to many match");
      }
      const HChar* matchPattern=vr_IOmatch_CmdLine+vr_cmatchKeyStrSize;
      if( IOMatch_log_level>0){
         VG_(fprintf)(vr_IOmatchCLRFileLog, "cmatch pattern [%lu] : %s\n",vr_nbMatch ,matchPattern);
      }
      vr_match_tab[vr_nbMatch].match_pattern=VG_(strdup)("cmatch.patterndup", matchPattern);
      vr_match_tab[vr_nbMatch].is_break_match_pattern=False;
      vr_match_tab[vr_nbMatch].is_exact_match_pattern=!(vr_containWildCard(matchPattern));
      vr_match_tab[vr_nbMatch].count_match=0;
      vr_match_tab[vr_nbMatch].nb_apply_match=0;
      vr_match_tab[vr_nbMatch].nb_post_apply_match=0;
      vr_nbMatch++;
      continue;
    }

    if( VG_(strncmp)(vr_IOmatch_CmdLine, vr_applyKeyStr, vr_applyKeyStrSize)==0 ){
      const HChar* applyCmd=stripSpace(vr_IOmatch_CmdLine+vr_applyKeyStrSize);
      if(IOMatch_verbose>2){
         VG_(umsg)("apply : %s\n", applyCmd);
      }
      if(vr_nbMatch<=0){
	VG_(tool_panic)("vr_IOmatch_clr : match expected before apply ");
      }
      vr_match_data_t* last_match_data=&(vr_match_tab[vr_nbMatch-1]);
      (last_match_data->nb_apply_match)++;
      if( (last_match_data->nb_apply_match) >= APPLY_PER_MATCH_MAX ){
	VG_(tool_panic)("vr_IOmatch_clr : too many apply per match ");
      }
      VG_(strncpy)( last_match_data->apply_match[last_match_data->nb_apply_match -1], applyCmd, MATCH_SIZE_MAX);
      continue;
    }

    if( VG_(strncmp)(vr_IOmatch_CmdLine, vr_postApplyKeyStr, vr_postApplyKeyStrSize)==0 ){
      const HChar* applyCmd=stripSpace(vr_IOmatch_CmdLine+vr_postApplyKeyStrSize);
      if(IOMatch_verbose>2){
         VG_(umsg)("post_apply : %s\n", applyCmd);
      }
      if(vr_nbMatch<0){
	VG_(tool_panic)("vr_IOmatch_clr : match expected before post-apply ");
      }
      vr_match_data_t* last_match_data=&(vr_match_tab[vr_nbMatch-1]);
      if(!(last_match_data->is_break_match_pattern)){
         VG_(tool_panic)("cmatch and post_apply are incompatible ");
      }

      (last_match_data->nb_post_apply_match)++;
      if((last_match_data->nb_post_apply_match)>=POST_APPLY_PER_MATCH_MAX ){
	VG_(tool_panic)("vr_IOmatch_clr : too many post-apply per match ");
      }
      VG_(strncpy)( last_match_data->post_apply_match[last_match_data->nb_post_apply_match -1], applyCmd, MATCH_SIZE_MAX);
      continue;
    }

    if( VG_(strncmp)(vr_IOmatch_CmdLine, vr_verboseKeyStr, vr_verboseKeyStrSize)==0 ){
      const HChar* verboseStr=stripSpace(vr_IOmatch_CmdLine+vr_verboseKeyStrSize);
      IOMatch_verbose=VG_(strtoull10)(verboseStr,  NULL);
      continue;
    }
    if( VG_(strncmp)(vr_IOmatch_CmdLine, vr_logLevelKeyStr, vr_logLevelKeyStrSize)==0 ){
      const HChar* logLevelStr=stripSpace(vr_IOmatch_CmdLine+vr_logLevelKeyStrSize);
      IOMatch_log_level=VG_(strtoull10)(logLevelStr,  NULL);
      continue;
    }

    if( VG_(strncmp)(vr_IOmatch_CmdLine, vr_nbMatchMaxKeyStr, vr_nbMatchMaxKeyStrSize)==0 ){
       if(vr_nbMatch>0){
          VG_(tool_panic)("vr_IOmatch_clr : nb-match-max is invalid after bmatch of cmatch");
       }
       const HChar* nbMatchMaxStr=stripSpace(vr_IOmatch_CmdLine+vr_nbMatchMaxKeyStrSize);
       vr_nb_match_max=VG_(strtoull10)(nbMatchMaxStr,  NULL);
       continue;
    }

    if( VG_(strncmp)(vr_IOmatch_CmdLine, vr_permutableBmatchIndexesKeyStr, vr_permutableBmatchIndexesKeyStrSize)==0 ){

       HChar* indexesStr=stripSpace(vr_IOmatch_CmdLine+vr_permutableBmatchIndexesKeyStrSize);
       HChar* next;
       vr_reorgBegin  =VG_(strtoull10)(indexesStr, &next );
       vr_reorgEnd =VG_(strtoull10)(next, NULL );

       if(vr_reorgEnd <=vr_reorgBegin){
          VG_(tool_panic)("vr_IOmatch_clr : vr_reorgEnd < vr_reorgBegin");
       }
       if(vr_reorgEnd >=vr_nb_match_max){
          VG_(tool_panic)("vr_IOmatch_clr : vr_reorgEnd >= vr_nb_match_max");
       }
       continue;
    }

    if( VG_(strncmp)(vr_IOmatch_CmdLine, vr_dumpStdoutKeyStr, vr_dumpStdoutKeyStrSize)==0 ){
       const HChar* dumpStr=stripSpace(vr_IOmatch_CmdLine+vr_dumpStdoutKeyStrSize);
       vr_dumpStdout=True;
       vr_IOmatchCLRFileStdout=openOutputIOMatchFile(dumpStr,fileName,".stdout-%p");
       continue;
    }

    if( VG_(strncmp)(vr_IOmatch_CmdLine, vr_dumpFilteredStdoutKeyStr, vr_dumpFilteredStdoutKeyStrSize)==0 ){
       const HChar* dumpStr=stripSpace(vr_IOmatch_CmdLine+vr_dumpFilteredStdoutKeyStrSize);
       vr_dumpFilteredStdout=True;
       vr_IOmatchCLRFileFilteredStdout=openOutputIOMatchFile(dumpStr,fileName,".filtered.stdout-%p");
       continue;
    }


    //Treat filter line exec key
    if( VG_(strncmp)(vr_IOmatch_CmdLine, vr_filterLineExecKeyStr, vr_filterLineExecKeyStrSize)==0 ){
       const HChar* filterExecCmd=stripSpace(vr_IOmatch_CmdLine+vr_filterLineExecKeyStrSize);
       if(IOMatch_verbose>3){
          VG_(umsg)("filter line exec str : %s\n", filterExecCmd);
       }

       if(vr_filter==True){
	 VG_(tool_panic)("vr_IOmatch_clr : only one filter_line_exec cmd is accepted");
       }
       vr_filter=True;
       VG_(strncpy)(vr_filter_cmd,filterExecCmd,FILTER_SIZEMAX);
       Bool beginWord=True;
       Int indexWord=0;
       Int indexShift=0;
       Bool escaped=False;
       for(Int indexStr=0; indexStr<FILTER_SIZEMAX; indexStr++){

	 HChar currentChar=vr_filter_cmd[indexStr];
	 if(currentChar== '\\'){
	   indexShift++;
	   escaped=True;
	   continue;
	 }
	 if(currentChar==0){
	   vr_filter_cmd[indexStr-indexShift]=0;
	   argvFiltered[indexWord+1]=NULL;
	   break;
	 }
	 if(currentChar==' ' && (!escaped)){
	   vr_filter_cmd[indexStr-indexShift]=0;
	   indexWord++;
	   if(indexWord==(ARGV_FILTER_MAX-1) ){
	     VG_(tool_panic)("too many args for filter_line_exec:");
	   }
	   beginWord=True;
	   escaped=False;
	   continue;
	 }
	 if(beginWord && ((currentChar!=' ')|| escaped) ){
	   argvFiltered[indexWord] = vr_filter_cmd+indexStr-indexShift;
	   beginWord=False;
	 }
	 if(beginWord==False && ((currentChar!=' ')|| escaped)){
	   vr_filter_cmd[indexStr-indexShift]=vr_filter_cmd[indexStr];
	 }
	 escaped=False;
       }

       if(IOMatch_verbose>1){
	 VG_(umsg)("vr_filter_cmd: %s\n", argvFiltered[0]);
	 Int indexArg=1;
	 while(True){
	   if(argvFiltered[indexArg]==NULL){
	     break;
	   }
	   VG_(umsg)("\tARGV[%d]: %s\n", indexArg,argvFiltered[indexArg]);
	   indexArg+=1;
	 }
       }

       // Run the filterCMD
       if( VG_(pipe)(filter_fdin)!=0){
	 VG_(umsg)("vr_IOmatchCLR: problem with PIPE fdin\n");
	 VG_(tool_panic)("Pipe error");
       }
       if( VG_(pipe)(filter_fdout)!=0){
	 VG_(umsg)("vr_IOmatchCLR: problem with PIPE fdout\n");
	 VG_(tool_panic)("Pipe error");
       }
       filter_pid= VG_(fork)();
       if(filter_pid == 0){
	 VG_(close)(filter_fdin[1]);
	 SysRes res = VG_(dup2)(filter_fdin[0], STDIN_FILENO);
	 if (sr_Res(res) < 0){
	   VG_(umsg)("vr_IOmatchCLR: problem with DUP2 in pipe 0\n");
	   VG_(exit)(1);
	 }
	 VG_(close)(filter_fdout[0]);
	 SysRes res2 = VG_(dup2)(filter_fdout[1], STDOUT_FILENO );
	 if (sr_Res(res2) < 0){
	   VG_(umsg)("vr_IOmatchCLR: problem with DUP2 out pipe 1\n");
	   VG_(exit)(1);
	 }

	 VG_(execv)(argvFiltered[0], argvFiltered);

	 /* If we are still alive here, execv failed. */
	 VG_(umsg)("vr_IOmatchCLR: problem with EXECV\n");
	 VG_(umsg)("ARGV[0] %s\n", argvFiltered[0]);
	 Int indexArg=1;
	 while(True){
	   if(argvFiltered[indexArg]==NULL){
	     break;
	   }
	   VG_(umsg)("ARGV[%d] %s\n", indexArg,argvFiltered[indexArg]);
	   indexArg+=1;
	 }
	 VG_(tool_panic)("EXECV filter failed");
       }

       if (filter_pid < 0) {
	 VG_(umsg)("vr_IOmatchCLR: problem with fork\n");
       }

       VG_(close)(filter_fdin[0]);
       VG_(close)(filter_fdout[1]);

       //initialize  vr_filtered_buff
       vr_filtered_buff=VG_(malloc)("vr_filtered_buff.1", LINE_SIZEMAX*sizeof(HChar));
       continue;
    }

    VG_(umsg)("Unused line : %s\n", vr_IOmatch_CmdLine);
    VG_(tool_panic)("vr_IOmatchCLR: bad IOMatch script\n");
  }//End while loop over line

  VG_(free)(vr_IOmatch_CmdLine);
  VG_(close)(vr.IOMatchCLRFileInput);

  vr_IOmatch_apply_clr("init", True);
}


int readlineCharByChar(int fd, char* msgRead,int sizeMax);
int readlineCharByChar(int fd, char* msgRead,int sizeMax){
  int totalSize=0;
  while(totalSize<sizeMax){
    char buffer[1];
    int size=VG_(read)(fd,buffer, 1);
    if(size==1){
      msgRead[totalSize]=buffer[0];
      if(buffer[0]=='\n' || buffer[0]==0){
	msgRead[totalSize]=0;
	return totalSize+1;
      }else{
	totalSize+=1;
      }
      continue;
    }
  }
  return -1;
};

static inline Bool isApplyMatchBreakable(Bool* isMatchPtr,//inout,
                                         Int* previousMatchIndexPtr,//out
                                         SizeT matchIndex, const HChar* filteredBuf)//in
{
   if( vr_string_match(matchIndex, filteredBuf)){
      *isMatchPtr=True;
      vr_match_data_t* current_match=&(vr_match_tab[matchIndex]);
      //The line match the expect pattern
      if(IOMatch_verbose>0){
         VG_(umsg)("match [%lu]: |%s|\n",matchIndex ,vr_writeLineBuffCurrent);
      }
      (current_match->count_match)++;
      if(IOMatch_log_level>0){
         VG_(fprintf)(vr_IOmatchCLRFileLog,"match [%lu]: %s\n",matchIndex ,vr_writeLineBuffCurrent);
      }
      if(vr_filter){
         if(IOMatch_verbose>1){
            VG_(umsg)("match(filtered): |%s|\n", filteredBuf);
         }
         if(IOMatch_log_level>0){
            VG_(fprintf)(vr_IOmatchCLRFileLog,"match(filtered): %s\n", filteredBuf);
         }
      }
      //Loop apply to DO
      if(current_match->nb_apply_match==0){
         vr_IOmatch_apply_clr("default", False);
      }
      for(SizeT applyIndex=0 ; applyIndex< current_match->nb_apply_match; applyIndex++){
         vr_IOmatch_apply_clr((current_match->apply_match)[applyIndex], False);
      }
      if(current_match->nb_post_apply_match!=0){
         *previousMatchIndexPtr=matchIndex;
      }
      if(current_match->is_break_match_pattern){
         return True;
         //break; //match only once
      }
   }
   return False;
}

void vr_IOmatch_clr_checkmatch(const HChar* writeLine,SizeT size){
   /*As the syscall to not give always a full line we need to create a buffer and to treat the buffer only we detect the end of line*/
  static Bool first=True;

   SizeT totalSize= (vr_writeLineBuffCurrent - vr_writeLineBuff) + size;
   if(totalSize >=  LINE_SIZEMAX){
      VG_(tool_panic)("vr_IOmatchCLR: line sizemax excedeed\n");
   }
   VG_(strncat)(vr_writeLineBuffCurrent , writeLine, size);
   vr_writeLineBuff[totalSize]=0;

   if(IOMatch_verbose>4){
     VG_(umsg)("vr_writeLineBuff: |%s|\n",vr_writeLineBuff);
   }

   vr_writeLineBuffCurrent=vr_writeLineBuff;

   //Search end of file
   for( SizeT i =0; i<totalSize ; i++){
       if(vr_writeLineBuff[i]=='\n'){
	 vr_writeLineBuff[i]=0;
	 HChar* filteredBuf=vr_writeLineBuffCurrent;

	 if(vr_dumpStdout){
	   VG_(fprintf)(vr_IOmatchCLRFileStdout,"%s\n", vr_writeLineBuffCurrent);
	 }
	 if(vr_filter){
	   // apply filter
	   vr_writeLineBuff[i]='\n';
	   VG_(write)(filter_fdin[1],vr_writeLineBuffCurrent, i+1 - (vr_writeLineBuffCurrent -vr_writeLineBuff ));
	   vr_writeLineBuff[i]=0;

	   Int sizeRead=readlineCharByChar(filter_fdout[0], vr_filtered_buff, LINE_SIZEMAX);

	   if(sizeRead <0){
	     VG_(umsg)("vr_IOmatchCLR: read error\n");
	   }
	   if(sizeRead >=LINE_SIZEMAX){
	     VG_(umsg)("vr_IOmatchCLR: read error sizemax\n");
	   }

	   filteredBuf=vr_filtered_buff;
	   if(vr_dumpFilteredStdout){
	     VG_(fprintf)(vr_IOmatchCLRFileFilteredStdout,"%s\n", vr_filtered_buff);
	   }
	 }

	 if(first){
	   if( !ignoreEmptyLine || (filteredBuf[0]!=0) ){
              if( IOMatch_log_level>0){
                 VG_(fprintf)(vr_IOmatchCLRFileLog,"post-init:\n");
              }
              vr_IOmatch_apply_clr("post-init", False);
              vr_countPostInit+=1;
              first=False;
	   }
	 }

	 if(previousMatchIndex!=-1 ){
	   if((ignoreEmptyLine) && (filteredBuf[0]==0 )){
	     if(IOMatch_verbose >0){
	       VG_(umsg)("empty Line ignored: \n");
	     }
	   }else{
              if(IOMatch_verbose >0){
		 VG_(umsg)("post apply:\n");
              }
              vr_match_data_t* previous_match=&(vr_match_tab[previousMatchIndex]);
              for(SizeT postApplyIndex=0 ; postApplyIndex< previous_match->nb_post_apply_match; postApplyIndex++){
                 vr_IOmatch_apply_clr((previous_match->post_apply_match)[postApplyIndex], False);
              }
              previousMatchIndex=-1;
	   }
	 }

         if(previousMatchIndex==-1){
	     Bool matchFound=False;
             Bool needToBreak=False;
             if(vr_reorgEnd >=vr_nbMatch){
                VG_(tool_panic)("vr_IOmatchCLR: vr_reorgEnd>=vr_nbMatch\n");
             }
	     for(SizeT matchIndex=0; matchIndex< vr_nbMatch; matchIndex++){
                SizeT matchIndexReorg=matchIndex;
                if( (vr_reorgBegin <= matchIndex) &&  (matchIndex <= vr_reorgEnd)){
                   matchIndexReorg= vr_lastReorgMatch + (matchIndex-vr_reorgBegin);
                   if(matchIndexReorg> vr_reorgEnd){
                      matchIndexReorg = matchIndexReorg -vr_reorgEnd+vr_reorgBegin-1;
                   }
                }

                needToBreak=isApplyMatchBreakable(&matchFound, &previousMatchIndex,
                                                       matchIndexReorg,filteredBuf);
                if(matchFound){
                   vr_lastReorgMatch=matchIndexReorg;
                }
                if(needToBreak)  break;
	     }


	     if( matchFound==False){
                if(IOMatch_log_level>1){
                   VG_(fprintf)(vr_IOmatchCLRFileLog,"line unmatch          : |%s|\n", vr_writeLineBuffCurrent);
                   VG_(fprintf)(vr_IOmatchCLRFileLog,"line(filtered) unmatch: |%s|\n", filteredBuf);
                }
                if(IOMatch_verbose>1){
                   VG_(umsg)("line unmatch          : |%s|\n", vr_writeLineBuffCurrent);
                   VG_(umsg)("line(filtered) unmatch: |%s|\n", filteredBuf);
                }
	     }
         }
	 vr_writeLineBuffCurrent= vr_writeLineBuff+i+1;
       }
   }

   //Move the end of buffer at the begin
   SizeT nbRemain=vr_writeLineBuff+totalSize-vr_writeLineBuffCurrent;
   for(SizeT i=0 ; i< nbRemain; i++){
      vr_writeLineBuff[i]=vr_writeLineBuffCurrent[i];
   }
   vr_writeLineBuff[nbRemain]=0;
   vr_writeLineBuffCurrent=vr_writeLineBuff+nbRemain;

}


void vr_IOmatch_clr_finalize (void){
  if(vr_countPostInit==0 && vr_nbpostInit!=0){
     if( IOMatch_log_level>0 ){
        VG_(fprintf)(vr_IOmatchCLRFileLog,"post-init:\n");
     }
     vr_IOmatch_apply_clr("post-init", True);
  }


  // If post_apply need to be applied
  if(previousMatchIndex!=-1){
    vr_match_data_t* previous_match=&(vr_match_tab[previousMatchIndex]);
    for(SizeT postApplyIndex=0 ; postApplyIndex< previous_match->nb_post_apply_match; postApplyIndex++){
       vr_IOmatch_apply_clr((previous_match->post_apply_match)[postApplyIndex], True);
    }
    previousMatchIndex=-1;
  }

  if(IOMatch_log_level>0){
     VG_(fprintf)(vr_IOmatchCLRFileLog,"match pattern count\n");
  }
  if(IOMatch_verbose>0){
     VG_(umsg)("match pattern count\n");
  }
  for(SizeT matchIndex=0; matchIndex< vr_nbMatch; matchIndex++){
     SizeT count=vr_match_tab[matchIndex].count_match;
     if(IOMatch_log_level>0){
        VG_(fprintf)(vr_IOmatchCLRFileLog,"\t%lu : %s\n", count, vr_match_tab[matchIndex].match_pattern);
     }
     if(IOMatch_verbose>0){
        VG_(umsg)("\t%lu : %s\n", count, vr_match_tab[matchIndex].match_pattern);
     }
  }

   //free and close evrything
   VG_(free)(vr_writeLineBuff);
   if(vr_filter){
     char msgEnd[]="";
     VG_(write)(filter_fdin[1], msgEnd, 1);

     VG_(close)(filter_fdout[0]);
     VG_(close)(filter_fdin[1]);
     VG_(waitpid)(filter_pid, NULL, 0);

     VG_(free)(vr_filtered_buff);
   }

   for(SizeT i=0; i< vr_nbMatch; i++){
     VG_(free)(vr_match_tab[i].match_pattern);
   }

   VG_(fclose)(vr_IOmatchCLRFileLog);
   if(vr_dumpStdout){
     VG_(fclose)(vr_IOmatchCLRFileStdout);
   }
   if(vr_IOmatchCLRFileFilteredStdout!=NULL){
     VG_(fclose)(vr_IOmatchCLRFileFilteredStdout);
   }
}
