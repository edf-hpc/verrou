#pragma once




static Vr_entry_func* vr_add_entry_func (Vr_entry_func* list, const HChar * fnname, const HChar * sym_name, UInt index) {
  Vr_entry_func * cell = VG_(malloc)("vr.entryFunc.1", sizeof(Vr_entry_func));
  cell->fnname  = VG_(strdup)("vr.entryFunc.2", fnname);
  cell->sym_name = VG_(strdup)("vr.entryFunc.3", sym_name);
  cell->index    = index;
  cell->next    = list;
  return cell;
}

static Vr_entry_func *
vr_find_entry_func (Vr_entry_func* list, const HChar * fnname, const HChar * sym_name) {
  Vr_entry_func * current;
  for (current = list ; current != NULL ; current = current->next) {
    if (VG_(strcmp)(current->fnname, fnname) != 0)
      continue;

    if (VG_(strcmp)(current->sym_name, sym_name) != 0)
      continue;

    return current;
  }

  return NULL;
}



static void vr_entry_func_trace_init(Vr_entry_func_trace* entry_trace, char* path){
  //  VG_(umsg)("debug path vr_entry_func_trace_init %s\n", path);
  entry_trace->currentIndex=0;
  entry_trace->list=NULL;

  entry_trace->bufferIndex=0;
  entry_trace->traceBuffer=VG_(malloc)("vr.entryFunc_traceinit", sizeof(UInt) * VERROU_TRACE_BUFFER_SIZE );

  const HChar * strInfo="./trace_info.log-%p";
  const HChar * strTrace="./trace_data.log-%p";
  HChar abs_info[512];
  HChar abs_trace[512];
  /*No need of length verification : need to be called after */

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
    VG_(sprintf)(abs_info,  "%s/%s", path, strInfo);
    VG_(sprintf)(abs_trace, "%s/%s", path, strTrace);
  } else {
    VG_(sprintf)(abs_info, "./%s", strInfo);
    VG_(sprintf)(abs_trace,"./%s", strTrace);
  }

  const HChar * strExpInfo=   VG_(expand_file_name)("vr.trace.strInfo",  abs_info);
  const HChar * strExpTrace=  VG_(expand_file_name)("vr.trace.strTrace", abs_trace);


  entry_trace->out_trace_info = VG_(fopen)(strExpInfo,
					    VKI_O_WRONLY | VKI_O_CREAT | VKI_O_EXCL, // VKI_O_TRUNC,
					    VKI_S_IRUSR|VKI_S_IWUSR|VKI_S_IRGRP|VKI_S_IROTH);

  entry_trace->out_trace = VG_(fopen)(strExpTrace,
				      VKI_O_WRONLY | VKI_O_CREAT | VKI_O_EXCL, // VKI_O_TRUNC,
				      VKI_S_IRUSR|VKI_S_IWUSR|VKI_S_IRGRP|VKI_S_IROTH);

  if(   entry_trace->out_trace_info==NULL || entry_trace->out_trace==NULL){
    VG_(umsg)("Error with %s or %s",strExpInfo,strExpTrace);
    VG_(tool_panic)("trace file initialization failed");
  }
}

static Vr_entry_func* vr_entry_func_trace_get_index(Vr_entry_func_trace* entry_trace, const HChar * fnname, const HChar * sym_name){
  Vr_entry_func* entry=vr_find_entry_func(entry_trace->list, fnname, sym_name);
  if(entry==NULL){
    entry_trace->currentIndex++;
    Vr_entry_func* newEntry=vr_add_entry_func(entry_trace->list, fnname, sym_name, entry_trace->currentIndex );
    entry_trace->list=newEntry;
    VG_(fprintf)(entry_trace->out_trace_info,"%u\t%s\t%s\n",(newEntry->index), (newEntry->fnname),(newEntry->sym_name));
    return newEntry;
  }else{
    return entry;
  }
}

static void vr_dumpBuffer(void){
  for(UInt i=0 ; i< vr.func_trace.bufferIndex; i++){
    VG_(fprintf)(vr.func_trace.out_trace, "%u\n", vr.func_trace.traceBuffer[i]);  
  }
  vr.func_trace.bufferIndex=0;
}


static void vr_dumpBufferCompress(void){
  UInt itab=0;
  UInt size=vr.func_trace.bufferIndex;
  UInt* tab=vr.func_trace.traceBuffer;;
  while( itab< size){
    UInt patternSize=0;
    for(UInt ipattern=1; ipattern < 4; ipattern++){
      if(itab+ipattern >=size){
	patternSize=0;
	break;
      }
      if(tab[itab+ipattern]==tab[itab]){
	patternSize=ipattern;
	break;
      }
    }
    if(patternSize==0){
      VG_(fprintf)(vr.func_trace.out_trace, "%u\n", tab[itab]);
      itab+=1;
    }else{
      //count pattern
      UInt count=1;
      Bool nextPattern=True;
      while (nextPattern){
	for(UInt j=0; j< patternSize; j++){
	  UInt indexj=itab+count*patternSize+j;
	  if( indexj>= size){
	    nextPattern=False;
	    break;
	  }
	  if(tab[indexj]!=tab[itab+j]){
	    nextPattern=False;
	    break;
	  }
	}
	if(nextPattern){
	  count++;
	}
      }//end nextPatternLoop

      //output
      switch(patternSize){
      case 1:
	if(count==1){
	  VG_(fprintf)(vr.func_trace.out_trace, "%u\n", tab[itab]);
	}else{
	  VG_(fprintf)(vr.func_trace.out_trace, "%ux%u\n", tab[itab],count);
	}
	break;
      case 2:
	if(count==1){
	  VG_(fprintf)(vr.func_trace.out_trace, "%u\n%u\n", tab[itab],tab[itab+1]);
	}else{
	  VG_(fprintf)(vr.func_trace.out_trace, "%u,%ux%u\n", tab[itab],  tab[itab+1],count);
	}
	break;
      case 3:
	if(count==1){
	  VG_(fprintf)(vr.func_trace.out_trace, "%u\n%u\n%u\n", tab[itab],tab[itab+1],tab[itab+2]);
	}else{
	  VG_(fprintf)(vr.func_trace.out_trace, "%u,%u,%ux%u\n", tab[itab],  tab[itab+1], tab[itab+2],count);
	}
	  break;
      }
      //update itab
      itab=itab+count*patternSize;
    }
  }//end loop itab

  vr.func_trace.bufferIndex=0;
}




static void vr_entry_free(Vr_entry_func_trace* entry_trace){
  vr_dumpBufferCompress();

  Vr_entry_func* current=entry_trace->list;
  VG_(fclose)(vr.func_trace.out_trace);
  VG_(fclose)(vr.func_trace.out_trace_info);

  while (current != NULL) {
    Vr_entry_func *next = current->next;
    VG_(free)(current->fnname);
    VG_(free)(current->sym_name);
    VG_(free)(current);
    current = next;
  }
}


static void vr_trace_entry_func_dyn(UInt index, Vr_entry_func* ptr){
  vr.func_trace.traceBuffer[vr.func_trace.bufferIndex++]=index;
  if(vr.func_trace.bufferIndex==VERROU_TRACE_BUFFER_SIZE){
    vr_dumpBufferCompress();
  }
}
