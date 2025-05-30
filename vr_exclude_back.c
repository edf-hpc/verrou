
/*--------------------------------------------------------------------*/
/*--- Verrou: a FPU instrumentation tool.                          ---*/
/*--- This file contains code allowing to exclude some symbols     ---*/
/*--- from the instrumentation.                                    ---*/
/*---                                                 vr_exclude.c ---*/
/*--------------------------------------------------------------------*/

/*
   This file is part of Verrou, a FPU instrumentation tool.

   Copyright (C) 2014-2021 EDF
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
#include "vr_exclude_back.h"

static void vr_freeAddrList (Vr_Addr_List* list);
static void vr_freeExcludeBack (Vr_Exclude_Back* list);
static void vr_dumpExcludeBack(Vr_Exclude_Back* list, VgFile* openFile, const HChar* fname);
static void vr_dumpAddrList(Vr_Addr_List* list, VgFile* openFile, const HChar* fname);
static Vr_Exclude_Back* vr_addExcludeBack(Vr_Exclude_Back* excludeBack, Int nbBack, Addr* ip, Bool used);
static Vr_Exclude_Back* vr_loadExcludeBack(Vr_Exclude_Back* list, const HChar* fname);
static Vr_Addr_List* vr_addAddrList(Vr_Addr_List* addrList, Addr ip);
static void vr_printf_back( Int nbBack, Addr* ip);



static Vr_Exclude_Back* vr_addExcludeBack (Vr_Exclude_Back* list, Int nbBack, Addr* ip, Bool used) {
  Vr_Exclude_Back * cell = VG_(malloc)("vr.addExcludeBack.1", sizeof(Vr_Exclude_Back));
  cell->nbBack    = nbBack;
  for(Int i =0; i<nbBack ; i++){
     (cell->ip)[i]=ip[i];
     cell->used=used;
  }
  cell->next    = list;
  return cell;
}

static Vr_Addr_List* vr_addAddrList (Vr_Addr_List* list, Addr ip) {
  Vr_Addr_List * cell = VG_(malloc)("vr.addAddrList.1", sizeof(Vr_Addr_List));
  cell->ip=ip;
  cell->next    = list;
  return cell;
}

static Vr_Exclude_Back *
vr_findExcludeBack (Vr_Exclude_Back* list, Int nbBack, Addr* ip) {
  Vr_Exclude_Back * exclude;

  Int count=0;
  for (exclude = list ; exclude != NULL ; exclude = exclude->next) {
     count++;
     if (exclude->nbBack != nbBack){
      continue;
     }
     Bool detectDiff=False;
     for(Int i=0; i<(nbBack- vr.backIgnoreSize); i++){
        if( exclude->ip[i]!= ip[i]){
           detectDiff=True;
           break;
        }
     }
     if(detectDiff){
        continue;
     }
     return exclude;
  }
  return NULL;
}

static Vr_Addr_List *
vr_findAddr (Vr_Addr_List* list, Addr ip) {
  Vr_Addr_List * cell;
  for (cell = list ; cell != NULL ; cell = cell->next) {
     if (cell->ip != ip){
      continue;
     }
     return cell;
  }
  return NULL;
}

void vr_addBackGen(Vr_Back* vrBack, Int nbBack, Addr* ip){
   Vr_Exclude_Back * excludeBack=vr_findExcludeBack (vrBack->gen_exclude, nbBack, ip);
   if(excludeBack==NULL){
      vrBack->gen_exclude=vr_addExcludeBack(vrBack->gen_exclude, nbBack, ip, True);
      for(Int i=0; i< nbBack; i++){
         Vr_Addr_List * cell=vr_findAddr(vrBack->addr_list, ip[i]);
         if(cell==NULL){
            vrBack->addr_list=vr_addAddrList(vrBack->addr_list, ip[i]);
         }
      }
   }
}


Bool vr_is_dir ( const HChar* f ); // declaration : implem in .vr_traceBB_impl.h

void vr_back_init(Vr_Back* vrBack, Bool genExclude, const HChar* rep){
   vrBack->exclude=NULL;
   vrBack->gen_exclude=NULL;
   vrBack->addr_list=NULL;
   vrBack->gen_exclude_file=NULL;
   vrBack->gen_addr_file=NULL;
   vrBack->handlerGenExclude=NULL;
   vrBack->handlerAddrList=NULL;

   if(genExclude){
      const HChar * strAddr="backAddrInfo-%p";
      const HChar * strBack="backInfo-%p";
      HChar absfileAddr[512];
      HChar absfileBack[512];

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
         VG_(sprintf)(absfileAddr, "%s/%s", rep, strAddr);
         VG_(sprintf)(absfileBack,  "%s/%s", rep, strBack);
      } else {
         VG_(sprintf)(absfileAddr, "./%s", strAddr);
         VG_(sprintf)(absfileBack,  "./%s", strBack);
      }
      vrBack->gen_exclude_file = VG_(expand_file_name)("vr.back.strBack",  absfileBack);
      vrBack->gen_addr_file  =  VG_(expand_file_name)("vr.add.strInfo",   absfileAddr);   

      vrBack->handlerGenExclude = VG_(fopen)(vrBack->gen_exclude_file ,
                                             VKI_O_WRONLY | VKI_O_CREAT | VKI_O_EXCL, // VKI_O_TRUNC,
                                             VKI_S_IRUSR|VKI_S_IWUSR|VKI_S_IRGRP|VKI_S_IROTH);

      vrBack->handlerAddrList = VG_(fopen)(vrBack->gen_addr_file,
                                           VKI_O_WRONLY | VKI_O_CREAT | VKI_O_EXCL, // VKI_O_TRUNC,
                                           VKI_S_IRUSR|VKI_S_IWUSR|VKI_S_IRGRP|VKI_S_IROTH);

      if( vrBack->handlerGenExclude==NULL ||    vrBack->handlerAddrList==NULL){
         VG_(umsg)("Error with %s or %s",vrBack->gen_exclude_file, vrBack->gen_addr_file);
         VG_(tool_panic)("backtrace files initialization failed");
      }
   }
}

static void vr_freeExcludeBack (Vr_Exclude_Back* list) {
  while (list != NULL) {
    Vr_Exclude_Back *next = list->next;
    VG_(free)(list);
    list = next;
  }
}

static void vr_freeAddrList (Vr_Addr_List* list) {
  while (list != NULL) {
    Vr_Addr_List *next = list->next;
    VG_(free)(list);
    list = next;
  }
}

void vr_back_finalize(Vr_Back* vrBack){
   vr_freeExcludeBack(vrBack->exclude);
   vr_freeExcludeBack(vrBack->gen_exclude);
   vr_freeAddrList(vrBack->addr_list);
}

static void vr_dumpExcludeBack (Vr_Exclude_Back* list, VgFile* fd, const HChar* fname) {
  if (VG_(clo_verbosity) >0){
    VG_(umsg)("Dumping backtrace exclusions list to `%s'... ", fname);
  }

  Vr_Exclude_Back * exclude;
  for (exclude = list ; exclude != NULL ; exclude = exclude->next) {
     VG_(fprintf)(fd,"%d:%lu",exclude->nbBack,exclude->ip[0]);
     for( Int i=1; i<exclude->nbBack; i++){
        VG_(fprintf)(fd,",%lu",exclude->ip[i]);
     }
     VG_(fprintf)(fd,"\n");
  }
  VG_(fclose)(fd);
  if(VG_(clo_verbosity) >0){
     VG_(umsg)("OK.\n");
  }
}

static void vr_printf_back( Int nbBack, Addr* ip){
     VG_(umsg)("%d:%lu",nbBack, ip[0]);
     for( Int i=1; i<nbBack; i++){
        VG_(umsg)(",%lu",ip[i]);
     }
}

static void vr_dumpAddrList (Vr_Addr_List* list, VgFile* fd, const HChar* fname) {
  if (VG_(clo_verbosity) >0){
    VG_(umsg)("Dumping backtrace addr list to `%s'... ", fname);
  }
  Vr_Addr_List * cell;
  DiEpoch de = VG_(current_DiEpoch)();
  for (cell = list ; cell != NULL ; cell = cell->next) {
     HChar const * fnname;
     //Bool errorFnname=
     VG_(get_fnname_w_offset)(de, cell->ip, &fnname);
     //debug sym
     HChar const * debugSym="";
     VG_(fprintf)(fd,"%lu\t%s\t%s\n",cell->ip, fnname,debugSym);
  }
  VG_(fclose)(fd);
  if(VG_(clo_verbosity) >0){
     VG_(umsg)("OK.\n");
  }
}

void vr_dumpBack(Vr_Back* vrBack){
   vr_dumpExcludeBack (vrBack->gen_exclude, vrBack->handlerGenExclude, vrBack->gen_exclude_file);
   vr_dumpAddrList (vrBack->addr_list, vrBack->handlerAddrList, vrBack->gen_addr_file);
}



#define LINE_SIZEMAX 512

static Vr_Exclude_Back* vr_loadExcludeBack(Vr_Exclude_Back* list, const HChar* fname){
   if (VG_(clo_verbosity) >0){
      VG_(umsg)("Loading backtrace exclusions list from `%s'... ", fname);
   }
   Int fd = VG_(fd_open)(fname,VKI_O_RDONLY, 0);
   if (fd == -1) {
      VG_(umsg)("\nError while loading backtrace exclusions list from `%s'\n", fname);
      VG_(tool_panic)("ERROR (open)\n");
   }

   SizeT nLine = LINE_SIZEMAX;
   HChar *line = VG_(malloc)("vr.loadBackExcludes.1", nLine*sizeof(HChar));
   Int lineno = 0;

   while (! VG_(get_line (fd, &line, &nLine, &lineno))) {
     if( *line== '#'){ //Workaround : VG_(get_line) can return a comment line for the last line without \n */
        continue;
     }
     HChar * c;
     for (c = line; c<line+LINE_SIZEMAX && *c != ':' && *c!=0; ++c) {}
     if(*c==':'){
        *c=0;
     }else{
        VG_(umsg)("error while reading %s : invalid format\n", fname);
        VG_(tool_panic)("error loadExcludeBack");
     }
     Int nb = VG_(strtoll10)(line,NULL);
     if(nb > BACKTRACE_SIZE){
        VG_(umsg)("error while reading %s nb=%d > BACKTRACE_SIZE=%d ", fname, nb, BACKTRACE_SIZE );
        VG_(tool_panic)("error loadExcludeBack");
     }
     HChar* addrLine=c+1;
     Addr addrTab[BACKTRACE_SIZE];
     for(Int addrIndex=0; addrIndex <nb; addrIndex++){
        for (c = addrLine; c<line+LINE_SIZEMAX && *c != ',' && *c != 0 ; ++c) {}
        *c=0;
        Addr addr = VG_(strtoull10)(addrLine,NULL);
        addrTab[addrIndex]=addr;
        addrLine=c+1;
     }
     list = vr_addExcludeBack(list, nb, addrTab, False);
   }
   VG_(free)(line);
   VG_(close)(fd);
   if (VG_(clo_verbosity) >0){
      VG_(umsg)("OK.\n");
   }
   return list;
}
#undef LINE_SIZEMAX

void vr_loadBack(Vr_Back* vrBack, const HChar* filename){
   vrBack->exclude= vr_loadExcludeBack(vrBack->exclude, filename);
}

Bool vr_isInstrumentedBack(Vr_Back* vrBack, Int nbBack, Addr* ip){
   Vr_Exclude_Back * excludeBack=vr_findExcludeBack (vrBack->exclude , nbBack, ip);
   if(excludeBack==NULL){
      return True;
   }else{
      if(!excludeBack->used){
         if(VG_(clo_verbosity) >0){
            VG_(umsg)("using backtrace exclusion rule: ");
            vr_printf_back(nbBack, ip);
            VG_(umsg)("\n");
         }
         excludeBack->used=True;
      }
      return False;
   }
}


