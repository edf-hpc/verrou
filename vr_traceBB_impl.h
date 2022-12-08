#include "vr_main.h"

/*
   This file is part of Verrou, a FPU instrumentation tool.

   Copyright (C) 2014-2021 EDF
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

#define COVER_GRAPH

struct traceBB_T {
  IRSB* irsb;
  UInt index;
  UInt counter;
  struct traceBB_T*  next;
};

typedef struct traceBB_T traceBB_t;


struct traceBBLink_T {
  UInt indexOld;
  UInt index;
  UInt counter;
  struct traceBBLink_T*  next;
};

typedef struct traceBBLink_T traceBBLink_t;


VgFile * vr_out_bb_info = NULL;
VgFile * vr_out_bb_cov= NULL;
VgFile * vr_out_bb_link= NULL;

traceBB_t* traceList=NULL ;
traceBBLink_t* traceLinkList=NULL ;


UInt indexOld=-1;

/* Trace */
//static void vr_trace_dyn_IRSB(HWord index, HWord counterPtr){
//  VG_(fprintf)(vr_out_bb_trace,"%u\n",(UInt)index);
//}

static void vr_count_dyn_IRSB(HWord index, HWord counterPtr){
  //  VG_(fprintf)(vr_out_bb_trace,"%u\n",(UInt)index);
  *((UInt*)counterPtr) +=1;
}

static void vr_link_dyn_IRSB(HWord index, HWord counterPtr);
static traceBBLink_t* vr_searchTraceLinkList(UInt indexOld);
traceBBLink_t* getNewTraceLinkBB(UInt indexOrg, UInt indexDest);

static void vr_link_dyn_IRSB(HWord index, HWord counterPtr){
  *((UInt*)counterPtr) +=1;

  traceBBLink_t* linkCell =vr_searchTraceLinkList(index);

  if( linkCell==NULL){
    traceLinkList=getNewTraceLinkBB(indexOld,index);
    linkCell=traceLinkList;
  }

  linkCell->counter += 1 ;
  indexOld=index;
}


static traceBBLink_t* vr_searchTraceLinkList(UInt index){
  traceBBLink_t* current=traceLinkList;
  while (current != NULL) {
    if( (current->indexOld)==indexOld && (current->index)==index){
      return current;
    }
    current = current->next;
  }
  return NULL;
}

//static void vr_countAndTrace_dyn_IRSB(HWord index, HWord counterPtr){
//  VG_(fprintf)(vr_out_bb_trace,"%u\n",(UInt)index);
//  *((UInt*)counterPtr) +=1;
//}


//typedef enum typeInstr{ instrTrace, instrCount, instrCountAndTrace} typeInstr_t;

static void vr_traceIRSB (IRSB* out, UInt  index, UInt* counterPtr){//, typeInstr_t select) {

  IRExpr** argv = mkIRExprVec_2 (mkIRExpr_HWord ((HWord)index),
				 mkIRExpr_HWord ((HWord)counterPtr));
  IRDirty* di;

  /* switch (select){ */
  /* case instrTrace: */
  /*   di = unsafeIRDirty_0_N(2, */
  /*       		   "vr_trace_dyn_IRSB", */
  /*       		   VG_(fnptr_to_fnentry)( &vr_trace_dyn_IRSB ), */
  /*       		   argv); */
  /*   break; */
  /* case instrCount: */
#ifndef COVER_GRAPH
  di = unsafeIRDirty_0_N(2,
			   "vr_count_dyn_IRSB",
			   VG_(fnptr_to_fnentry)( &vr_count_dyn_IRSB ),
			   argv);
#else
  di = unsafeIRDirty_0_N(2,
			   "vr_link_dyn_IRSB",
			   VG_(fnptr_to_fnentry)( &vr_link_dyn_IRSB ),
			   argv);
#endif
    /*   break; */
  /* case instrCountAndTrace: */
  /*   di = unsafeIRDirty_0_N(2, */
  /*       		   "vr_countAndTrace_dyn_IRSB", */
  /*       		   VG_(fnptr_to_fnentry)( &vr_countAndTrace_dyn_IRSB), */
  /*       		   argv); */
  /*   break; */
  /* } */

  addStmtToIRSB (out, IRStmt_Dirty (di));
}

traceBB_t* getNewTraceBB(IRSB* irsb_in);
traceBB_t* getNewTraceBB(IRSB* irsb_in){
  traceBB_t * res = VG_(malloc)("vr.getNewTraceBB", sizeof(traceBB_t));
  res->next=traceList;
  res->counter=0;
  if(res->next !=NULL){
    res->index=((res->next)->index) + 1;
  }else{
    res->index=0;
  }
  res->irsb=irsb_in;

  traceList=res;
  return res;
}


traceBBLink_t* getNewTraceLinkBB(UInt indexOrg, UInt indexDest){
  traceBBLink_t * res = VG_(malloc)("vr.getNewTraceLinkBB", sizeof(traceBBLink_t));
  res->next=traceLinkList;
  res->counter=0;
  res->indexOld=indexOrg;
  res->index=indexDest;
  traceLinkList=res;
  return res;
}


void freeTraceBBList(void);
void freeTraceBBList(void){
    while (traceList != NULL) {
      traceBB_t* next= traceList->next;
      VG_(free)(traceList);
      traceList=next;
    }
}

Bool vr_is_dir ( const HChar* f );
Bool vr_is_dir ( const HChar* f )
{ //copy of /coregrind/m_libcfile.c which is not public
   struct vg_stat buf;
   SysRes res = VG_(stat)(f, &buf);
   return sr_isError(res) ? False
     : VKI_S_ISDIR(buf.mode) ? True : False;
}



void vr_traceBB_initialize(char* path);
void vr_traceBB_initialize(char* path){
  const HChar * strInfo="./trace_bb_info.log-%p";
  const HChar * strCov="./trace_bb_cov.log-%p";
  HChar absfileInfo[512];
  HChar absfileCov[512];

#ifdef COVER_GRAPH
  const HChar * strLink="./trace_bb_link.log-%p";
  HChar absfileLink[512];
#endif

  if (path!=NULL) {
    if(VG_(strlen)(path) >400){
      VG_(tool_panic)("too long output path\n");
    }

    if(!vr_is_dir(path) ){
      HChar mkdCmd[512];
      VG_(sprintf)(mkdCmd, "mkdir -p %s", path);
      VG_(umsg)("Cmd : %s\n",mkdCmd);
      Int r=VG_(system)(mkdCmd);
      if(r){
	VG_(tool_panic)("not able to create directory");
      }
    }
    VG_(sprintf)(absfileInfo, "%s/%s", path, strInfo);
    VG_(sprintf)(absfileCov,  "%s/%s", path, strCov);
#ifdef COVER_GRAPH
    VG_(sprintf)(absfileLink,  "%s/%s", path, strLink);
#endif
  } else {
    VG_(sprintf)(absfileInfo, "./%s", strInfo);
    VG_(sprintf)(absfileCov,  "./%s", strCov);
#ifdef COVER_GRAPH
    VG_(sprintf)(absfileLink,  "./%s", strLink);
#endif
  }

  const HChar * strExpInfo= VG_(expand_file_name)("vr.traceBB.strInfo",  absfileInfo);
  const HChar * strExpCov=  VG_(expand_file_name)("vr.traceBB.strCov",   absfileCov);

  vr_out_bb_info = VG_(fopen)(strExpInfo,
			      VKI_O_WRONLY | VKI_O_CREAT | VKI_O_EXCL, // VKI_O_TRUNC,
			      VKI_S_IRUSR|VKI_S_IWUSR|VKI_S_IRGRP|VKI_S_IROTH);

  vr_out_bb_cov = VG_(fopen)(strExpCov,
			       VKI_O_WRONLY | VKI_O_CREAT | VKI_O_EXCL, // VKI_O_TRUNC,
			       VKI_S_IRUSR|VKI_S_IWUSR|VKI_S_IRGRP|VKI_S_IROTH);

  if(/*vr_out_bb_trace==NULL || */ vr_out_bb_info==NULL /*|| vr_out_bb_info_backtrace==NULL */|| vr_out_bb_cov==NULL){
    VG_(umsg)("Error with %s or %s",strExpInfo,strExpCov);
    VG_(tool_panic)("trace file initialization failed");
  }
#ifdef COVER_GRAPH
  const HChar * strExpLink=  VG_(expand_file_name)("vr.traceBB.strLink",   absfileLink);
  vr_out_bb_link = VG_(fopen)(strExpLink,
			       VKI_O_WRONLY | VKI_O_CREAT | VKI_O_EXCL, // VKI_O_TRUNC,
			       VKI_S_IRUSR|VKI_S_IWUSR|VKI_S_IRGRP|VKI_S_IROTH);
  if(vr_out_bb_link==NULL){
    VG_(umsg)("Error with %s",strExpLink);
    VG_(tool_panic)("trace file initialization failed");
  }
#endif
};

void vr_traceBB_finalize(void);
void vr_traceBB_finalize(void){
   freeTraceBBList();

  if(vr_out_bb_info!=NULL){
    VG_(fclose)(vr_out_bb_info);
  }
/*  if(vr_out_bb_trace!=NULL){
    VG_(fclose)(vr_out_bb_trace);
    }
  if(vr_out_bb_info_backtrace !=NULL){
    VG_(fclose)(vr_out_bb_info_backtrace);
    }*/
  if(vr_out_bb_cov !=NULL){
    VG_(fclose)(vr_out_bb_cov);
  }

#ifdef COVER_GRAPH
  if(vr_out_bb_link !=NULL){
    VG_(fclose)(vr_out_bb_link);
  }
#endif
}


void vr_traceBB_resetCov(void){
  traceBB_t* current=traceList;
  while (current != NULL) {
    current->counter=0;
    current = current->next;
  }
#ifdef COVER_GRAPH
  traceBBLink_t* currentLink=traceLinkList;
  while (currentLink != NULL) {
    currentLink->counter=0;
    currentLink = currentLink->next;
  }
#endif
}



UInt vr_traceBB_dumpCov(void){
  static UInt numPartialCov=0;
  if(vr_out_bb_cov==NULL){
     numPartialCov+=1;
     return numPartialCov-1;
  }

  VG_(fprintf)(vr_out_bb_cov, "cover-%u\n", numPartialCov);
  traceBB_t* current=traceList;
  while (current != NULL) {
    if(current->counter!=0){
      VG_(fprintf)(vr_out_bb_cov,"%u:%u\n",(current->index),(current->counter));
    }
    current = current->next;
  }
#ifdef COVER_GRAPH
  VG_(fprintf)(vr_out_bb_link, "cover-%u\n", numPartialCov);
  traceBBLink_t* currentLink=traceLinkList;
  while (currentLink != NULL) {
    if(currentLink->counter!=0){
      VG_(fprintf)(vr_out_bb_link,"%u,%u:%u\n",(currentLink->indexOld), (currentLink->index),(currentLink->counter));
    }
    currentLink = currentLink->next;
  }
#endif
  numPartialCov+=1;
  vr_traceBB_resetCov();
  return numPartialCov-1;
}




void vr_traceBB_trace_imark(traceBB_t* tr,const HChar * fnname,const HChar * filename,UInt lineNum,
			    Bool doLineContainFloat, Bool doLineContainFloatCmp);
void vr_traceBB_trace_imark(traceBB_t* tr,const HChar * fnname,const HChar * filename,UInt lineNum,
			    Bool doLineContainFloat, Bool doLineContainFloatCmp){
  VG_(fprintf)(vr_out_bb_info, "%u : %s : %s : %u : %s : %s\n",
	       (tr->index), fnname, filename, lineNum,
	       doLineContainFloat?"1":"0",
	       doLineContainFloatCmp?"1":"0");
}


/* void vr_traceBB_trace_backtrace(traceBB_t* tr); */
/* void vr_traceBB_trace_backtrace(traceBB_t* tr){ */
/*   Addr ips[256]; */

/*   const HChar * fnname; */
/*   const HChar * objname; */

/*   int n_ips=VG_(get_StackTrace)(VG_(get_running_tid)(), */
/* 				ips, 256, */
/* 				NULL, NULL, */
/* 				0); */
/*   DiEpoch de = VG_(current_DiEpoch)(); */
/*   VG_(fprintf)(vr_out_bb_info_backtrace, "begin: %p\n",  (void*)(tr->irsb)); */

/*   int i; */
/*   for (i = n_ips - 1; i >= 0; i--) { */
/*     Vg_FnNameKind kind = VG_(get_fnname_kind_from_IP)(de, ips[i]); */
/*     if (Vg_FnNameMain == kind || Vg_FnNameBelowMain == kind) */
/*       n_ips = i + 1; */
/*     if (Vg_FnNameMain == kind) */
/*       break; */
/*   } */

/*   for(i=0; i<n_ips;i++){ */
/*     Addr addr = ips[i]; */
/*     VG_(get_fnname)(de, addr, &fnname); */
/*     VG_(get_objname)(de, addr, &objname); */
/*     VG_(fprintf)(vr_out_bb_info_backtrace, "%p : %s - %s\n", (void*)addr, fnname, objname); */
/*   } */
/* } */
