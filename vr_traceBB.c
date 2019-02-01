


struct traceBB_T {
  IRSB* irsb;
};

typedef struct traceBB_T traceBB_t;

VgFile * vr_out_bb_info = NULL;
VgFile * vr_out_bb_info_backtrace= NULL;
VgFile * vr_out_bb_trace= NULL;



/*Trace*/
static void vr_trace_dyn_IRSB(HWord irsb, HWord ptr){
  VG_(fprintf)(vr_out_bb_trace,"%p\n",(void*)irsb);
}



static void vr_traceIRSB (IRSB* out, IRSB* in, void* ptr) {
  IRExpr** argv = mkIRExprVec_2 (mkIRExpr_HWord ((HWord)in),
				 mkIRExpr_HWord ((HWord)ptr));
  IRDirty* di;
  di = unsafeIRDirty_0_N(2,
                         "vr_trace_dyn_IRSB",
                         VG_(fnptr_to_fnentry)( &vr_trace_dyn_IRSB ),                   
                         argv);
  addStmtToIRSB (out, IRStmt_Dirty (di));
}


void vr_traceBB_initialize(void);
void vr_traceBB_initialize(void){
  const HChar * strInfo="trace_bb_info.log-%p";
  const HChar * strTrace="trace_bb_trace.log-%p";
  const HChar * strInfoBack="trace_bb_info_backtrace.log-%p";
  const HChar * strExpInfo=   VG_(expand_file_name)("vr.traceBB.strInfo",  strInfo);
  const HChar * strExpTrace=  VG_(expand_file_name)("vr.traceBB.strTrace", strTrace);
  const HChar * strExpBack=  VG_(expand_file_name)("vr.traceBB.strBack", strInfoBack);

  vr_out_bb_info = VG_(fopen)(strExpInfo,
			      VKI_O_WRONLY | VKI_O_CREAT | VKI_O_TRUNC,
			      VKI_S_IRUSR|VKI_S_IWUSR|VKI_S_IRGRP|VKI_S_IROTH);
  vr_out_bb_trace = VG_(fopen)(strExpTrace,
			       VKI_O_WRONLY | VKI_O_CREAT | VKI_O_TRUNC,
			       VKI_S_IRUSR|VKI_S_IWUSR|VKI_S_IRGRP|VKI_S_IROTH);
  vr_out_bb_info_backtrace = VG_(fopen)(strExpBack,
			       VKI_O_WRONLY | VKI_O_CREAT | VKI_O_TRUNC,
			       VKI_S_IRUSR|VKI_S_IWUSR|VKI_S_IRGRP|VKI_S_IROTH);

  if(vr_out_bb_trace==NULL || vr_out_bb_info==NULL || vr_out_bb_info_backtrace==NULL){
    VG_(tool_panic)("trace file initialization failed\n");
  }
};

void vr_traceBB_finalyze(void);
void vr_traceBB_finalyze(void){
  if(vr_out_bb_info!=NULL){
    VG_(fclose)(vr_out_bb_info);
  }
  if(vr_out_bb_trace!=NULL){
    VG_(fclose)(vr_out_bb_trace);
  }
}

void vr_traceBB_initBB(traceBB_t* tr,IRSB* sbIn);
void vr_traceBB_initBB(traceBB_t* tr,IRSB* sbIn){
  tr->irsb=sbIn;
}


void vr_traceBB_trace_imark(traceBB_t* tr,const HChar * fnname,const HChar * filename,UInt lineNum);
void vr_traceBB_trace_imark(traceBB_t* tr,const HChar * fnname,const HChar * filename,UInt lineNum){
  VG_(fprintf)(vr_out_bb_info, "%p : %s : %s : %d\n", (void*)(tr->irsb), fnname, filename, lineNum);
}


void vr_traceBB_trace_backtrace(traceBB_t* tr){
  Addr ips[256];

  const HChar * fnname;
  const HChar * objname;

  int n_ips=VG_(get_StackTrace)(VG_(get_running_tid)(),
				ips, 256,
				NULL, NULL,
				0);
  DiEpoch de = VG_(current_DiEpoch)();
  VG_(fprintf)(vr_out_bb_info_backtrace, "begin: %p\n",  (void*)(tr->irsb));

  int i;
  for (i = n_ips - 1; i >= 0; i--) {
    Vg_FnNameKind kind = VG_(get_fnname_kind_from_IP)(de, ips[i]);
    if (Vg_FnNameMain == kind || Vg_FnNameBelowMain == kind)
      n_ips = i + 1;
    if (Vg_FnNameMain == kind)
      break;
  }

  for(i=0; i<n_ips;i++){
    Addr addr = ips[i];
    VG_(get_fnname)(de, addr, &fnname);
    VG_(get_objname)(de, addr, &objname);
    VG_(fprintf)(vr_out_bb_info_backtrace, "%p : %s - %s\n", (void*)addr, fnname, objname);
  }
}

void vr_traceBB_closeBB(traceBB_t* tr);
void vr_traceBB_closeBB(traceBB_t* tr){
}
