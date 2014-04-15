#ifndef __VR_FPOPS_H
#define __VR_FPOPS_H

#ifdef __cplusplus
extern "C" {
#endif

  void vr_fpOpsInit (void);

  double vr_AddDouble (double a, double b);
  float  vr_AddFloat  (float  a, float  b);

#ifdef __cplusplus
}
#endif

#endif /* ndef __VR_FPOPS_H */
