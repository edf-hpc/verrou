
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
#include "vr_exclude_back.h"


static void vr_freeAddrList (Vr_Addr_List* list);
static UInt vr_freeExcludeBack (Vr_Exclude_Back* list, Bool verbose);
static void vr_dumpExcludeBack(Vr_Exclude_Back* listTab[HASH_TABLE_SIZE], VgFile* openFile, const HChar* fname);
static void vr_dumpAddrList(Vr_Addr_List* list, VgFile* openFile, const HChar* fname);
static Vr_Exclude_Back* vr_addExcludeBack(Vr_Exclude_Back* excludeBack, uint64_t hash, Int nbBack, Addr* ip, Bool used);
static void vr_loadExcludeBack(Vr_Exclude_Back* listTab[HASH_TABLE_SIZE], const HChar* fname);
static Vr_Addr_List* vr_addAddrList(Vr_Addr_List* addrList, Addr ip);
static void vr_printf_back( Int nbBack, Addr* ip);



static Vr_Exclude_Back* vr_addExcludeBack (Vr_Exclude_Back* list, uint64_t hash, Int nbBack, Addr* ip, Bool used) {
  Vr_Exclude_Back * cell = VG_(malloc)("vr.addExcludeBack.1", sizeof(Vr_Exclude_Back));
  cell->hash = hash;
  cell->nbBack = nbBack;
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
vr_findExcludeBack (Vr_Exclude_Back* list, uint64_t hash, Int nbBack, Addr* ip) {
  Vr_Exclude_Back * exclude;

  for (exclude = list ; exclude != NULL ; exclude = exclude->next) {
     if(exclude->hash!=hash){
        continue;
     }
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

static uint64_t random_tab[HASH_TABLE_SIZE*2 + 1]={ //81
   2985842423061390143u,
   6155994775641341576u,
   11133449992811317908u,
   2809993047858131118u,
   343786130611462202u,
   10690702445040133432u,
   4472513001295944961u,
   2637892484857255287u,
   15189824055630787375u,
   471453054957249064u,
   16552904328305087823u,
   16881247611676794125u,
   12152756136273148605u,
   3535861666160102650u,
   4951063668586377889u,
   10775862766476761754u,
   3748389828156510984u,
   2626395394426996450u,
   13170924636444605068u,
   7799449480253520417u,
   5122313439138483079u,
   2159572797422004593u,
   3136432834710339610u,
   12915487189227830479u,
   6496157115756572912u,
   7607983774205437223u,
   13287476727360348339u,
   9340551607431390514u,
   8013023239282372496u,
   9699839932870471558u,
   8513220962990564476u,
   1054323830018557827u,
   2296323699372013602u,
   15424791006065552493u,
   9042448944516228065u,
   6264448299341813139u,
   5063398896814650533u,
   11020122929152178401u,
   17671784740069361653u,
   7153875437183966428u,
   16885889230938066065u,
   16745672200074323915u,
   8154495383720775682u,
   15269517765492493200u,
   6823217064475791746u,
   17662671531950591504u,
   7579988816145209287u,
   6528958726939472121u,
   14980161600902429607u,
   9652664859464035868u,
   7703324691731719795u,
   8076745247662332549u,
   10528203704637706849u,
   7752710728204745783u,
   3247272757173584161u,
   6391961793109400208u,
   13925765315525301671u,
   14906710822340923727u,
   15786857429020403268u,
   13118961848124046344u,
   9061602297803521817u,
   14602810107122255403u,
   5181387183074701763u,
   12455440596304530655u,
   9488500234855566716u,
   2840795029857820201u,
   10637415451187460530u,
   9526742773740779915u,
   12993957145109984077u,
   16595983440426650519u,
   3068904279573555455u,
   4536579437332634936u,
   14391189681883609427u,
   2987416569882863943u,
   15305567246421999454u,
   15227420318761939354u,
   12404560068799548348u,
   15738609249941343559u,
   2577779507510210097u,
   5250821968302244420u,
   15436811984011331066u
};
/*This tab is generated with :
#include <random>
#include <iostream>
int main(int argc, char** argv){
    std::random_device rd;
    std::mt19937_64 gen(rd());
    std::uniform_int_distribution<uint64_t> dis;
    for(int i=0; i< atoi(argv[1]); i++){
      std::cout << "    "<<dis(gen) << "u,"<<std::endl;
    }
    return 0;
}*/


static inline uint64_t hash_back(Int nbBack, Addr* ip){
   uint64_t res=random_tab[0];
   for(int i=0; i< nbBack; i++){
      uint64_t val64=ip[i];
      uint32_t val32_1=val64;
      uint32_t val32_2=(val64>>32);

      res+= random_tab[2*i+1]*val32_1;
      res+= random_tab[2*i+2]*val32_2;
   }
   return res;
}

void vr_addBackGen(Vr_Back* vrBack, Int nbBack, Addr* ip){
   uint64_t hash=hash_back(nbBack, ip);
   uint32_t hashKey= hash & HASH_TABLE_MASK;
   Vr_Exclude_Back * excludeBack=vr_findExcludeBack (vrBack->gen_exclude[hashKey], hash, nbBack, ip);
   if(excludeBack==NULL){
      vrBack->gen_exclude[hashKey]=vr_addExcludeBack(vrBack->gen_exclude[hashKey], hash, nbBack, ip, True);
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
   for(int i=0; i < HASH_TABLE_SIZE; i++){
      vrBack->exclude[i]=NULL;
      vrBack->gen_exclude[i]=NULL;
   }
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

static UInt vr_freeExcludeBack (Vr_Exclude_Back* list, Bool verbose) {
   UInt count=0;
   while (list != NULL) {
      count++;
      if(verbose){
        if(!(list->used)){
           VG_(umsg)("Warning unused backtrace exclude: ");
           vr_printf_back(list->nbBack, list->ip);
           VG_(umsg)("\n");
        }
     }

     Vr_Exclude_Back *next = list->next;
     VG_(free)(list);
     list = next;
  }
  return count;
}

static void vr_freeAddrList (Vr_Addr_List* list) {
  while (list != NULL) {
    Vr_Addr_List *next = list->next;
    VG_(free)(list);
    list = next;
  }
}

void vr_back_finalize(Vr_Back* vrBack){
   VG_(umsg)("vrback->exclude hashmap repartition\n");
   UInt emptyCount=0;
   VG_(printf)("\t");
   for(int i=0; i< HASH_TABLE_SIZE; i++){
      UInt count=vr_freeExcludeBack(vrBack->exclude[i], True);
      if(count!=0){
         VG_(printf)("%u ", count);
      }else{
         emptyCount++;
      }
   }
   VG_(printf)("(empty cases: %u)\n", emptyCount);

   VG_(umsg)("vrback->gen_exclude hashmap repartition\n");
   emptyCount=0;
   VG_(printf)("\t");
   for(int i=0; i< HASH_TABLE_SIZE; i++){
      UInt count=vr_freeExcludeBack(vrBack->gen_exclude[i], False);
      if(count!=0){
         VG_(printf)("%u ", count);
      }else{
         emptyCount++;
      }
   }
   VG_(printf)("(empty cases: %u)\n", emptyCount);


   vr_freeAddrList(vrBack->addr_list);
}

static void vr_dumpExcludeBack (Vr_Exclude_Back* listTab[HASH_TABLE_SIZE], VgFile* fd, const HChar* fname) {
  if (VG_(clo_verbosity) >0){
    VG_(umsg)("Dumping backtrace exclusions list to `%s'... ", fname);
  }

  Vr_Exclude_Back * exclude;
  for ( Int indexHash=0; indexHash< HASH_TABLE_SIZE;indexHash++){
     for (exclude = listTab[indexHash] ; exclude != NULL ; exclude = exclude->next) {
        VG_(fprintf)(fd,"%d:%lu",exclude->nbBack,exclude->ip[0]);
        for( Int i=1; i<exclude->nbBack; i++){
           VG_(fprintf)(fd,",%lu",exclude->ip[i]);
        }
        VG_(fprintf)(fd,"\n");
     }
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

     const HChar * filename=NULL;
//     const HChar ** filenamePtr=&filenamenoname;
     const HChar ** filenamePtr=&filename;
     UInt  linenum=0;
     UInt*  linenumPtr=&linenum;
     VG_(get_filename_linenum)(de,
                               cell->ip,
                               filenamePtr,
                               NULL,
                               linenumPtr);

     VG_(fprintf)(fd,"%lu\t%s\t%s\t%u\n",cell->ip, fnname,filename, linenum);
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

static void vr_loadExcludeBack(Vr_Exclude_Back* listTab[HASH_TABLE_SIZE], const HChar* fname){
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
     uint64_t hash=hash_back(nb, addrTab);
     uint32_t hashKey=hash & HASH_TABLE_MASK;
     listTab[hashKey] = vr_addExcludeBack(listTab[hashKey], hash, nb, addrTab, False);
   }
   VG_(free)(line);
   VG_(close)(fd);
   if (VG_(clo_verbosity) >0){
      VG_(umsg)("OK.\n");
   }
}
#undef LINE_SIZEMAX

void vr_loadBack(Vr_Back* vrBack, const HChar* filename){
   vr_loadExcludeBack(vrBack->exclude, filename);
}

Bool vr_isInstrumentedBack(Vr_Back* vrBack, Int nbBack, Addr* ip){
   uint64_t hash=hash_back(nbBack, ip);
   uint32_t hashKey= hash & HASH_TABLE_MASK;
   Vr_Exclude_Back * excludeBack=vr_findExcludeBack (vrBack->exclude[hashKey] , hash, nbBack, ip);
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


