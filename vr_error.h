#pragma once

typedef enum {
  VR_ERROR_UNCOUNTED,
  VR_ERROR_SCALAR,
  VR_ERROR_NAN,
  VR_ERROR_INF,
  VR_ERROR_CC,
  VR_ERROR_CD,
  VR_ERROR_FLT_MAX,
  VR_ERROR
} Vr_ErrorKind;


const HChar* vr_get_error_name (const Error* err);
Bool vr_recognised_suppression (const HChar* name, Supp* su);
void vr_before_pp_Error (const Error* err) ;
void vr_pp_Error (const Error* err);
Bool vr_eq_Error (VgRes res, const Error* e1, const Error* e2);
UInt vr_update_extra (const Error* err);
Bool vr_error_matches_suppression (const Error* err, const Supp* su);
Bool vr_read_extra_suppression_info (Int fd, HChar** bufpp, SizeT* nBuf,
                                     Int* lineno, Supp* su);
SizeT vr_print_extra_suppression_info (const Error* er,
                                      /*OUT*/HChar* buf, Int nBuf);
SizeT vr_print_extra_suppression_use (const Supp* s,
                                     /*OUT*/HChar* buf, Int nBuf);
void vr_update_extra_suppression_use (const Error* err, const Supp* su);


void vr_maybe_record_ErrorOp (Vr_ErrorKind kind, IROp op);
void vr_maybe_record_ErrorRt (Vr_ErrorKind kind);



void vr_handle_NaN (void);
void vr_handle_Inf (void);
void vr_handle_CC (int);
void vr_handle_CD (void);
void vr_handle_FLT_MAX (void);

