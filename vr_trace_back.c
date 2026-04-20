
/*--------------------------------------------------------------------*/
/*--- Verrou: a FPU instrumentation tool.                          ---*/
/*--- This file contains code allowing to exclude some symbols     ---*/
/*--- from the instrumentation.                                    ---*/
/*---                                              vr_trace_back.c ---*/
/*--------------------------------------------------------------------*/

/*
   This file is part of Verrou, a FPU instrumentation tool.

   Copyright (C) 2014-2026 EDF
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
#include "vr_trace_back.h"


static UInt vr_freeTraceBack (Vr_Trace_Back* list);
static void vr_trace_dump_and_clear_aux(Vr_Trace_Back* listTab[HASH_TABLE_TRACE_SIZE], VgFile* openFile, const HChar* fname, UInt indexCov);

static Vr_Trace_Back* vr_addTraceBack(Vr_Trace_Back* list, uint64_t hash, Addr bbAddr, Int nbBack, Addr* ip, uint64_t indexFirst);


static Vr_Trace_Back* vr_addTraceBack (Vr_Trace_Back* list, uint64_t hash, Addr bbAddr, Int nbBack, Addr* ip, uint64_t indexFirst) {
  Vr_Trace_Back * cell = VG_(malloc)("vr.addTraceBack.1", sizeof(Vr_Trace_Back));
  cell->hash = hash;
  cell->bbAddr=bbAddr;
  cell->nbBack = nbBack;
  for(Int i =0; i<nbBack ; i++){
     (cell->ip)[i]=ip[i];
  }
  cell->counter=1;
  cell->next = list;
  cell->indexFirst=indexFirst;
  return cell;
}

static Vr_Trace_Back *
vr_findTraceBack (Vr_Trace_Back* list, uint64_t hash, Addr bbAddr, Int nbBack, Addr* ip) {
  Vr_Trace_Back * trace_cell;

  for (trace_cell = list ; trace_cell != NULL ; trace_cell = trace_cell->next) {
     if(trace_cell->bbAddr!=bbAddr){
        continue;
     }
     if(trace_cell->hash!=hash){
        continue;
     }
     if (trace_cell->nbBack != nbBack){
      continue;
     }
     Bool detectDiff=False;
     for(Int i=0; i<(nbBack- vr.backIgnoreSize); i++){
        if( trace_cell->ip[i]!= ip[i]){
           detectDiff=True;
           break;
        }
     }
     if(detectDiff){
        continue;
     }
     return trace_cell;
  }
  return NULL;
}




void vr_trace_inc(Vr_Trace* vrTrace, Addr bbAddr, Int nbBack, Addr* ip){
   uint64_t hash=hash_back_bb(bbAddr, nbBack, ip);
   uint32_t hashKey= hash & HASH_TABLE_TRACE_MASK;
   Vr_Trace_Back * trace_cell = vr_findTraceBack (vrTrace->trace[hashKey], hash, bbAddr, nbBack, ip);
   if(trace_cell==NULL){
      vrTrace->currentIndexFirst++;
      vrTrace->trace[hashKey] = vr_addTraceBack(vrTrace->trace[hashKey], hash, bbAddr, nbBack, ip, vrTrace->currentIndexFirst);

      for(Int i=0; i< nbBack; i++){
         Vr_Addr_List * cell = vr_findAddr(vrTrace->back_addr_list, ip[i]);
         if(cell==NULL){
            DiEpoch de = VG_(current_DiEpoch)();
            Addr addrBack=ip[i];
            vrTrace->back_addr_list = vr_addAddrList(vrTrace->back_addr_list, addrBack);            
            HChar const * fnname;
            VG_(get_fnname_w_offset)(de, addrBack, &fnname);
            const HChar * filename=NULL;
            const HChar ** filenamePtr=&filename;
            UInt  linenum=0;
            UInt*  linenumPtr=&linenum;
            VG_(get_filename_linenum)(de,
                                      addrBack,
                                      filenamePtr,
                                      NULL,
                                      linenumPtr);

            VG_(fprintf)(vrTrace->handlerBackAddrList, "%lu\t%s\t%s\t%u\n",addrBack, fnname,filename, linenum);
         }
      }
   }else{
      (trace_cell->counter)++;
   }
}


void vr_trace_init0(Vr_Trace* vrTrace){
   for(int i=0; i < HASH_TABLE_TRACE_SIZE; i++){
      vrTrace->trace[i]=NULL;
   }
   vrTrace->back_addr_list=NULL;
   vrTrace->bb_addr_list=NULL;
   vrTrace->currentIndexFirst=0;

   vrTrace->cover_file=NULL;
   vrTrace->back_addr_file=NULL;
   vrTrace->bb_addr_file=NULL;
   vrTrace->handlerCover=NULL;
   vrTrace->handlerBackAddrList=NULL;
   vrTrace->handlerBBAddrList=NULL;
}

void vr_trace_init_rep(Vr_Trace* vrTrace, const HChar* rep){
   vr_trace_init0(vrTrace);

   const HChar * strBackAddr="backAddrInfo-%p";
   const HChar * strBBAddr="bbAddrInfo-%p";
   const HChar * strCover="backCoverInfo-%p";
   HChar absfileBackAddr[512];
   HChar absfileBBAddr[512];
   HChar absfileCover[512];

   if (rep!=NULL) {
      if(VG_(strlen)(rep) >400){
         VG_(tool_panic)("too long output path\n");
      }

      if(!vr_is_dir(rep) ){
         HChar mkdCmd[512];
         VG_(sprintf)(mkdCmd, "mkdir -p %s", rep);
         VG_(umsg)("Cmd : %s\n",mkdCmd);
         Int r=VG_(system)(mkdCmd);
         if(r){
            VG_(tool_panic)("not able to create directory");
         }
      }
      VG_(sprintf)(absfileBackAddr, "%s/%s", rep, strBackAddr);
      VG_(sprintf)(absfileBBAddr,  "%s/%s", rep, strBBAddr);
      VG_(sprintf)(absfileCover,  "%s/%s", rep, strCover);
   } else {
      VG_(sprintf)(absfileBackAddr, "./%s", strBackAddr);
      VG_(sprintf)(absfileBBAddr,  "./%s", strBBAddr);
      VG_(sprintf)(absfileCover,  "./%s",  strCover);
   }
   vrTrace->cover_file = VG_(expand_file_name)("vr.cover.strBack",  absfileCover);
   vrTrace->back_addr_file  =  VG_(expand_file_name)("vr.backAddr.strInfo",   absfileBackAddr);
   vrTrace->bb_addr_file  =  VG_(expand_file_name)("vr.BBAddr.strInfo",   absfileBBAddr);

   vrTrace->handlerCover = VG_(fopen)(vrTrace->cover_file ,
                                      VKI_O_WRONLY | VKI_O_CREAT | VKI_O_EXCL, // VKI_O_TRUNC,
                                      VKI_S_IRUSR|VKI_S_IWUSR|VKI_S_IRGRP|VKI_S_IROTH);

   vrTrace->handlerBackAddrList = VG_(fopen)(vrTrace->back_addr_file,
                                        VKI_O_WRONLY | VKI_O_CREAT | VKI_O_EXCL, // VKI_O_TRUNC,
                                        VKI_S_IRUSR|VKI_S_IWUSR|VKI_S_IRGRP|VKI_S_IROTH);

   vrTrace->handlerBBAddrList = VG_(fopen)(vrTrace->bb_addr_file,
                                           VKI_O_WRONLY | VKI_O_CREAT | VKI_O_EXCL, // VKI_O_TRUNC,
                                           VKI_S_IRUSR|VKI_S_IWUSR|VKI_S_IRGRP|VKI_S_IROTH);

   if( vrTrace->handlerCover==NULL ||    vrTrace->handlerBackAddrList==NULL || vrTrace->handlerBBAddrList==NULL){
      VG_(umsg)("Error with %s or %s or %s\n",vrTrace->cover_file, vrTrace->back_addr_file, vrTrace->bb_addr_file);
      VG_(tool_panic)("backtrace files initialization failed");
   }
}

static UInt vr_freeTraceBack (Vr_Trace_Back* list) {
   UInt count=0;
   while (list != NULL) {
     count++;
     Vr_Trace_Back *next = list->next;
     VG_(free)(list);
     list = next;
  }
  return count;
}


void vr_trace_finalize(Vr_Trace* vrTrace){
   VG_(umsg)("vrTrace->trace hashmap repartition\n");
   UInt emptyCount=0;
   VG_(printf)("\t");
   for(int i=0; i< HASH_TABLE_TRACE_SIZE; i++){
      UInt count=vr_freeTraceBack(vrTrace->trace[i]);
      if(count!=0){
         VG_(printf)("%u ", count);
      }else{
         emptyCount++;
      }
   }
   VG_(printf)("(empty cases: %u)\n", emptyCount);

   vr_freeAddrList(vrTrace->back_addr_list);
   vr_freeAddrList(vrTrace->bb_addr_list);

   VG_(fclose)(vrTrace->handlerCover);
   VG_(fclose)(vrTrace->handlerBackAddrList);
   VG_(fclose)(vrTrace->handlerBBAddrList);
}



static void vr_trace_dump_and_clear_aux (Vr_Trace_Back* listTab[HASH_TABLE_TRACE_SIZE], VgFile* fd, const HChar* fname, UInt indexCov) {
  if (VG_(clo_verbosity) >0){
     VG_(umsg)("Dumping backtrace exclusions list to `%s' [%u] ... ", fname, indexCov);
  }

  Vr_Trace_Back * cell;
  VG_(fprintf)(fd, "cover-%u\n", indexCov);
  for ( Int indexHash=0; indexHash< HASH_TABLE_TRACE_SIZE;indexHash++){
     for (cell = listTab[indexHash] ; cell != NULL ; cell = cell->next) {
        if(cell->counter!=0){
           VG_(fprintf)(fd,"[%lu] %d:%lu",cell->bbAddr,  cell->nbBack, cell->ip[0]);
           for( Int i=1; i<cell->nbBack; i++){
              VG_(fprintf)(fd,",%lu",cell->ip[i]);
           }
           VG_(fprintf)(fd,"\t%lu\t%lu\n", cell->counter, cell->indexFirst);
           cell->counter=0;
        }
        cell->indexFirst=0;
     }
  }
  if(VG_(clo_verbosity) >0){
     VG_(umsg)("OK.\n");
  }
}

void vr_trace_dump_and_clear(Vr_Trace* vrTrace, UInt indexCov ){
   vr_trace_dump_and_clear_aux(vrTrace->trace, vrTrace->handlerCover, vrTrace->cover_file, indexCov);
   vrTrace->currentIndexFirst=0;
}

static void vr_printf_back_BB( Int nbBack, Addr* ip, Addr* bb){
   VG_(umsg)("[%lu] %d:%lu",*bb,nbBack, ip[0]);
   for( Int i=1; i<nbBack; i++){
      VG_(umsg)(",%lu",ip[i]);
   }
}

Bool vr_trace_is_bbAddr_already_defined(Vr_Trace* vrTrace, Addr bbAddr){
   Vr_Addr_List* res=vr_findAddr (vrTrace->bb_addr_list, bbAddr);
   if(res==NULL){
      if(VG_(clo_verbosity) >1){
         VG_(umsg)("bbAddr already seen %lu\n", bbAddr);
      }
      return False;
   }else{
      vrTrace->bb_addr_list= vr_addAddrList (vrTrace->bb_addr_list, bbAddr);
      return True;
   }
}

static inline HChar containFloatToC(Bool containFloat){
   if(containFloat){
      return 'F';
   }else{
      return 'I';
   }
}
static inline HChar containCmpToC(Bool containCmp){
   if(containCmp){
      return '?';
   }else{
      return '!';
   }
}


void vr_trace_set_debugdata_for_bbAddr(Vr_Trace* vrTrace, Addr bbAddr,
                                       const char* filename, int linenum,
                                       Bool containFloat, Bool containCmp){
   VgFile * fd=vrTrace->handlerBBAddrList;

   VG_(fprintf)(fd,"[%lu] %s\t%d\t%c\t%c\n", bbAddr, filename, linenum,
                containFloatToC(containFloat),
                containCmpToC(containCmp));
}




VG_REGPARM(0) void vr_trace_dyn_IRSB(HWord addrBB){
   Addr bbAddr=(Addr)addrBB;
   Addr ips[BACKTRACE_SIZE];
   Int nb=VG_(get_StackTrace)(VG_(get_running_tid)(),ips, BACKTRACE_SIZE,
                                 NULL, NULL,0);
   Int nbReduced=reduced_nb_below_main(ips, nb);

   vr_trace_inc(&(vr.traceBack), bbAddr, nbReduced, ips);
}


void vr_traceBackIRSB (IRSB* out,  Addr  addrBB){
   IRExpr** argv = mkIRExprVec_1 (mkIRExpr_HWord ((HWord)addrBB));
   IRDirty* di;
   di = unsafeIRDirty_0_N(1,
                          "vr_trace_dyn_IRSB",
                          VG_(fnptr_to_fnentry)( &vr_trace_dyn_IRSB ),
                          argv);

   addStmtToIRSB (out, IRStmt_Dirty (di));
}

