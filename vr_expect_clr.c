/*--------------------------------------------------------------------*/
/*--- Verrou: a FPU instrumentation tool.                          ---*/
/*--- This file contains code allowing to exclude some symbols     ---*/
/*--- from the instrumentation.                                    ---*/
/*---                                                 vr_exclude.c ---*/
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

#include "vr_main.h"
#include "pub_tool_seqmatch.h"
#include "coregrind/pub_core_libcfile.h"

#define LINE_SIZEMAX 40000
#define FILTER_SIZEMAX 512

//void vr_expect_apply_clrs(void);
//void vr_expect_apply_clr(HChar* cmd);

HChar vr_defaultKeyStr[]="default: ";
SizeT vr_defaultKeyStrSize=sizeof(vr_defaultKeyStr)-1;

HChar vr_filterLineExecKeyStr[]="filter_line_exec: ";
SizeT vr_filterLineExecKeyStrSize=sizeof(vr_filterLineExecKeyStr)-1 ;

HChar vr_expectKeyStr[]= "expect: ";
SizeT vr_expectKeyStrSize=sizeof(vr_expectKeyStr)-1;

HChar vr_applyKeyStr[]=  "apply: ";
SizeT vr_applyKeyStrSize=sizeof(vr_applyKeyStr)-1;

#define DEFAULT_MAX 10
#define DEFAULT_SIZE_MAX 50
HChar vr_applyDefault[DEFAULT_MAX][DEFAULT_SIZE_MAX];
SizeT vr_nbDefault=0;

HChar* vr_currentExpectStr=NULL;
HChar* vr_expectTmpLine =NULL;
HChar* vr_writeLineBuff =NULL;
HChar* vr_writeLineBuffCurrent =NULL;

SizeT vr_last_expect_lineNo=0;

Bool vr_filter=False;
HChar vr_filter_cmd[FILTER_SIZEMAX];
HChar* vr_filtered_buff;
HChar tmpFileNameFilter[256]="/tmp/shouldNotBeUsed";



HChar nopStr[]="nop";
HChar emptyStr[]="";
HChar defaultStr[]="default";
HChar stopStr[]="stop";
HChar startStr[]="start";
HChar displayCounterStr[]="display_counter";
HChar dumpCoverStr[]="dumpCoverStr";


typedef enum {nopKey=0, emptyKey, defaultKey,stopKey, startKey, displayCounterKey, dumpCoverKey} Vr_applyKey;
static const SizeT actionNumber=7;
HChar* actionStrTab[]={nopStr, emptyStr, defaultStr, stopStr, startStr, displayCounterStr,dumpCoverStr};
SizeT actionSizeTab[]={sizeof(nopStr), sizeof(emptyStr),sizeof(defaultStr),sizeof(stopStr), sizeof(startStr), sizeof(displayCounterStr),sizeof(dumpCoverStr)};


static Vr_applyKey vr_CmdToEnum(const HChar* cmd){

  for(SizeT i=0 ; i< actionNumber; i++){
    if(VG_(strncmp)(cmd, actionStrTab[i],actionSizeTab[i])==0){
      return i;
    }
  }
  VG_(umsg)("vr_CmdToEnum unknown cmd : %s\n", cmd);
  VG_(tool_panic)("invalid expect script");
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

static Int my2_get_char ( Int fd, HChar* out_buf )
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
static Bool get_fullnc_line ( Int fd, HChar** bufpp, SizeT* lineno )
{
   // VG_(get_line) adapted

   HChar* buf  = *bufpp;
   static SizeT staticLineNo=0;

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
            *(lineno)=staticLineNo;
            return True;//False; /* the next call will return True */
         }
         if (i > 0 && i == LINE_SIZEMAX-1) {
            VG_(umsg)("too large line\n");
            VG_(exit)(1);
         }

         if (ch == '\n'){
            staticLineNo++;
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
         *(lineno)=staticLineNo;
         return False;
      }
   }
}


static Bool get_first_line ( Int fd, HChar** bufpp)
{
   // VG_(get_line) adapted
   HChar* buf  = *bufpp;
   Int i=0;
   Int n;
   HChar ch=0;
   //Loop over the char until end of line
   while (True) {
     n = my2_get_char(fd, &ch);
     if (n <= 0){
       return False;
     }
     if (i == LINE_SIZEMAX-1) {
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
     i++;
   }
   return True;
}






void vr_expect_clr_init (const HChar * fileName) {
   /*Open input File*/
   VG_(umsg)("Open expect clr file : `%s'... \n", fileName);
  vr.expectCLRFileInput= VG_(fd_open)(fileName, VKI_O_RDONLY,0);

  if (vr.expectCLRFileInput == -1) {
    VG_(umsg)("ERROR (open)\n");
    VG_(exit)(1);
  }

  /*Open output File*/
  const HChar * strLogFilename="expect_clr.log-%p";
  const HChar * strLogFilenameExpanded=   VG_(expand_file_name)("vr.expect_clr_log.1",  strLogFilename);

  VG_(umsg)("Open expect clr file : `%s'... \n", strLogFilenameExpanded);
  vr.expectCLRFileOutput = VG_(fopen)(strLogFilenameExpanded,
                                      VKI_O_WRONLY | VKI_O_CREAT | VKI_O_TRUNC,
                                      VKI_S_IRUSR|VKI_S_IWUSR|VKI_S_IRGRP|VKI_S_IROTH);

  if(vr.expectCLRFileOutput==NULL){
      VG_(umsg)("ERROR (fopen) %s\n",strLogFilenameExpanded);
      VG_(tool_panic)("vr_expect_clr : missing file");
  }


  /*Allocate requiered Buffer*/
  vr_expectTmpLine = VG_(malloc)("vr_expect_cl_init.1", LINE_SIZEMAX*sizeof(HChar)); 
  vr_writeLineBuff= VG_(malloc)("vr_expect_cl_init.2",  LINE_SIZEMAX*sizeof(HChar)); 
  vr_writeLineBuff[0]=0;
  vr_writeLineBuffCurrent= vr_writeLineBuff;

  /*Loop over input line until first expect key*/
  SizeT lineNo = 0;
  while (! get_fullnc_line(vr.expectCLRFileInput, &vr_expectTmpLine, &lineNo)) { 

     //Treat default key
    if( VG_(strncmp)(vr_expectTmpLine, vr_defaultKeyStr, vr_defaultKeyStrSize)==0  ){
       const HChar* defaultAction=vr_expectTmpLine+vr_defaultKeyStrSize;
       VG_(umsg)("default action str : %s\n", defaultAction);
       if (vr_valid_apply_cmd(defaultAction)){
	 VG_(fprintf)(vr.expectCLRFileOutput, "default action [%lu] : %s\n",vr_nbDefault ,defaultAction);
	 VG_(strncpy)(vr_applyDefault[vr_nbDefault],defaultAction, DEFAULT_SIZE_MAX);
	 vr_nbDefault++;
       }else{
          VG_(umsg)("default action %s is not valid", defaultAction);
          VG_(tool_panic)("vr_expect_clr : invalid action");
       }
       continue;
    }

    //Treat filter line exec key
    if( VG_(strncmp)(vr_expectTmpLine, vr_filterLineExecKeyStr, vr_filterLineExecKeyStrSize)==0  ){
       const HChar* filterExecCmd=vr_expectTmpLine+vr_filterLineExecKeyStrSize;
       VG_(umsg)("filter line exec str : %s\n", filterExecCmd);
       vr_filter=True;
       VG_(strncpy)(vr_filter_cmd,filterExecCmd,FILTER_SIZEMAX);
       VG_(umsg)("vr_filter_cmd : %s\n", vr_filter_cmd);
       vr_filtered_buff=VG_(malloc)("vr_filtered_buff.1", LINE_SIZEMAX*sizeof(HChar));
       Int id=VG_(mkstemp) ("vrFilterCmd", tmpFileNameFilter );
       VG_(close)(id);
       continue;
    }

    //Treat expect key
    if( VG_(strncmp)(vr_expectTmpLine, vr_expectKeyStr, vr_expectKeyStrSize)==0  ){
       vr_currentExpectStr=vr_expectTmpLine+vr_expectKeyStrSize;
       vr_last_expect_lineNo=lineNo;
       break;
    }
  }
//  VG_(umsg)("expectCLR init done\n");
}



static void vr_applyCmd(Vr_applyKey key, const HChar* cmd){

  switch(key){
  case nopKey:
    return;
  case emptyKey:
    return;
  case defaultKey:
    VG_(tool_panic)("default treated before");
    return;
  case stopKey:
    vr_set_instrument_state ("Expect CLR", False, True);
    VG_(fprintf)(vr.expectCLRFileOutput,"apply : stop\n");
    return;
  case startKey:
    vr_set_instrument_state ("Expect CLR", True, True);
    VG_(fprintf)(vr.expectCLRFileOutput,"apply : start\n");
    return;
  case displayCounterKey:
    vr_ppOpCount();
    VG_(fprintf)(vr.expectCLRFileOutput,"apply : display_counter\n");
    return;
  case dumpCoverKey:
    {
      SizeT ret;
      ret=vr_traceBB_dumpCov();
      VG_(fprintf)(vr.expectCLRFileOutput,"apply : dump_cover : %lu\n", ret);
      return;
    }
  }
  VG_(umsg)("vr_applyCmd :  unknown cmd : %s\n", cmd);
  VG_(exit)(1);
}


static
void vr_expect_apply_clr(const HChar* cmd){
  Vr_applyKey key=vr_CmdToEnum(cmd);

  if(key==defaultKey){
    for(SizeT i=0; i< vr_nbDefault;i++){
      Vr_applyKey keyDefault=vr_CmdToEnum(vr_applyDefault[i]);
      vr_applyCmd(keyDefault,vr_applyDefault[i]);
    }
  }else{
    vr_applyCmd(key,cmd);
  }
}


static
void vr_expect_apply_clrs(void){
   Int countApply=0;
   SizeT lineNo = 0;

   //Loop over input file until next expect line
   while (!get_fullnc_line(vr.expectCLRFileInput, &vr_expectTmpLine, &lineNo)) {

      //except key
      if( VG_(strncmp)(vr_expectTmpLine, vr_expectKeyStr, vr_expectKeyStrSize)==0  ){
	if(countApply==0){
	  vr_expect_apply_clr("default");
	}
	vr_last_expect_lineNo=lineNo;
	vr_currentExpectStr=vr_expectTmpLine+vr_expectKeyStrSize;
	return;
      }
      //Apply key
      if( VG_(strncmp)(vr_expectTmpLine, vr_applyKeyStr, vr_applyKeyStrSize)==0  ){
	vr_expect_apply_clr(vr_expectTmpLine+vr_applyKeyStrSize);
	countApply++;
	continue;
      }

      // what to do with this line
      VG_(fprintf)(vr.expectCLRFileOutput,"Line %lu ignored : %s\n", lineNo, vr_expectTmpLine);
      VG_(umsg)("expect_clr : Line %lu ignored : %s\n", lineNo, vr_expectTmpLine);
   }
   if(countApply==0){
      vr_expect_apply_clr("default");
   }
   vr_expectTmpLine[0]=0;
}



void vr_expect_clr_checkmatch(const HChar* writeLine,SizeT size){
   /*As the syscall to not give always a full line we need to create a buffer and to treat the buffer only we detect the end of line*/

   SizeT totalSize= (vr_writeLineBuffCurrent - vr_writeLineBuff) + size;
   if(totalSize >=  LINE_SIZEMAX){
      VG_(umsg)("sizemax excedeed\n");
      VG_(exit(1));
   }
   VG_(strncat)(vr_writeLineBuffCurrent , writeLine, size);
   vr_writeLineBuff[totalSize]=0;

   vr_writeLineBuffCurrent=vr_writeLineBuff;

   //Search end of file
   for( SizeT i =0; i<totalSize ; i++){
       if(vr_writeLineBuff[i]=='\n'){
	 vr_writeLineBuff[i]=0;

	 HChar* filteredBuf=vr_writeLineBuffCurrent;
	 if(vr_filter){
	   // apply filter

	   HChar cmdPattern[]="/usr/bin/sh -c \"echo %s | %s\" > %s ";
	   HChar cmdPatternReplaced[FILTER_SIZEMAX];
	   VG_(snprintf)(cmdPatternReplaced,FILTER_SIZEMAX, cmdPattern, vr_writeLineBuffCurrent, vr_filter_cmd, tmpFileNameFilter);
	   //	   VG_(umsg)("debug: %s\n" , cmdPatternReplaced);
	   VG_(system)(cmdPatternReplaced);
	   Int tmpFile=VG_(fd_open)(tmpFileNameFilter,  VKI_O_RDONLY, 0);
	   get_first_line(tmpFile, &vr_filtered_buff);

	   filteredBuf=vr_filtered_buff;
	 }

	 if( VG_(string_match)(vr_currentExpectStr,filteredBuf)){
	    //The line match the expect pattern
	   VG_(umsg)("expect: %s\n", vr_writeLineBuffCurrent);
	   VG_(fprintf)(vr.expectCLRFileOutput,"expect: %s\n", vr_writeLineBuffCurrent);
	    if(vr_filter){
	      VG_(umsg)("expect(filtered): %s\n", filteredBuf);
	      VG_(fprintf)(vr.expectCLRFileOutput,"expect(filtered): %s\n", filteredBuf);
	    }

	    //All actions are applied
	    vr_expect_apply_clrs();
	 }else{
	   /*Nothing to do*/
	 }
	 vr_writeLineBuffCurrent= vr_writeLineBuff+i+1;
      }
   }
   //Move the end of buffer at the begin
   SizeT i;
   SizeT nbReste=vr_writeLineBuff+totalSize-vr_writeLineBuffCurrent;
   for(i=0 ; i< nbReste; i++){
      vr_writeLineBuff[i]=vr_writeLineBuffCurrent[i];
   }
   vr_writeLineBuff[i]=0;
   vr_writeLineBuffCurrent=vr_writeLineBuff+i;
}


void vr_expect_clr_finalize (void){

   // If there are still line in the input file : it probably mean that an expect line was not found
   // Messages are required to debug (or not)
   if(vr_expectTmpLine[0]!=0){
      VG_(umsg)("Warning incomplete except script\n");

      VG_(fprintf)(vr.expectCLRFileOutput,"Used but not found line %lu : %s\n", vr_last_expect_lineNo,vr_expectTmpLine);
      VG_(umsg)("\tUsed but not found line %lu : %s\n", vr_last_expect_lineNo,vr_expectTmpLine);

      SizeT lineNo=0;
      while (!get_fullnc_line(vr.expectCLRFileInput, &vr_expectTmpLine, &lineNo)) {
	if( VG_(strncmp)(vr_expectTmpLine ,"", LINE_SIZEMAX)==0) continue;
	VG_(fprintf)(vr.expectCLRFileOutput,"Unused line %lu : %s\n", lineNo,vr_expectTmpLine);
	VG_(umsg)("\tUnused line %lu : %s\n", lineNo,vr_expectTmpLine);
      }
   }

   //free and close evrything
   VG_(free)(vr_expectTmpLine);
   VG_(free)(vr_writeLineBuff);
   if(vr_filter){
     VG_(free)(vr_filtered_buff);
     VG_(unlink)(tmpFileNameFilter);
   }
   VG_(close)(vr.expectCLRFileInput);
   VG_(fclose)(vr.expectCLRFileOutput);

}
