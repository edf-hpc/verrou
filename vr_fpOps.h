#ifndef __VR_FPOPS_H
#define __VR_FPOPS_H

#ifdef __cplusplus
extern "C" {
#endif

  enum vr_RoundingMode {
    VR_NEAREST,
    VR_UPWARD,
    VR_DOWNWARD,
    VR_ZERO,
    VR_RANDOM, // Must be immediately after standard rounding modes
    VR_AVERAGE
  };

  void vr_fpOpsInit (enum vr_RoundingMode mode);
  void vr_fpOpsFini (void);

  void vr_fpOpsSeed (unsigned int seed);
  void vr_fpOpsRandom (void);

  double vr_AddDouble (double a, double b);
  float  vr_AddFloat  (float  a, float  b);
  double vr_MulDouble (double a, double b);
  float  vr_MulFloat  (float  a, float  b);
  double vr_DivDouble (double a, double b);
  float  vr_DivFloat  (float  a, float  b);


#ifdef __cplusplus
}
#endif

#endif /* ndef __VR_FPOPS_H */
