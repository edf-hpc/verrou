#ifndef __VR_ERROR_H
#define __VR_ERROR_H

typedef enum {
  VR_ERROR_UNCOUNTED,
  VR_ERROR_SCALAR,
  VR_ERROR
} Vr_ErrorKind;

const HChar* vr_get_error_name (Error* err);
Bool vr_recognised_suppression (const HChar* name, Supp* su);
void vr_before_pp_Error (Error* err) ;
void vr_pp_Error (Error* err);
Bool vr_eq_Error (VgRes res, Error* e1, Error* e2);
UInt vr_update_extra (Error* err);
Bool vr_error_matches_suppression (Error* err, Supp* su);
Bool vr_read_extra_suppression_info (Int fd, HChar** bufpp, SizeT* nBuf,
                                     Int* lineno, Supp* su);
Bool vr_print_extra_suppression_info (Error* er,
                                      /*OUT*/HChar* buf, Int nBuf);
Bool vr_print_extra_suppression_use (Supp* s,
                                     /*OUT*/HChar* buf, Int nBuf);
void vr_update_extra_suppression_use (Error* err, Supp* su);


void vr_maybe_record_ErrorOp (Vr_ErrorKind kind, IROp op);


#endif /*ndef __VR_ERROR_H*/
