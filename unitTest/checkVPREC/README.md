
This test is largely based on  https://github.com/verificarlo/verificarlo/tree/master/tests/test_vprec_backend

Remarks:

- references are stored in mpfr_reference.tar.bz2 to avoid mpfr dependency. If you want to rebuild new reference, make build_reference.
By default the seed of the prng is random. To fix the seed you need to modify  generate_input.py.


- the reference generation is modified to be equal to vprec:
      - compute_mpfr_rounding generates the reference files. All loop (type float or double, precision, range, vprec-mode, fpPoint ) are done in this program, to be coherent with check_vprec_rounding. 
      - The discussion https://stackoverflow.com/questions/38664778/subnormal-numbers-in-different-precisions-with-mpfr was used to set mpfr range and precision.
      - The operation +/*,-... are done with the same precision as native type (float or double)
      - The sqrt and fma are added
      - A reference line add a char '~' or '=' if the tie break rule is important or not.


- the loop over floating point is done in a compiled program (see check_vprec_rounding) to reduce verrou instrumentation cost. We need to pay attention to deactivate verrou/vprec instrumentation for during check operation. The loop size can be reduced by an additional parameter argv[4]. 


- the check is more strict:
  - if the reference line is marked with =, a strict equality between mpfr and vprec is expected.
  - with ob mode and a line with ~ mark, an exact  ulp (with VPREC range/precision) difference is accepted
  - with ib and all mode, a ulp tolerance is accepted.
  - in practice, strict equality is observed. 


- check_vprec_rounding is written is c++ to use template (float ou double) but keep lot of C style language (code legacy + proximity with compute_mpfr_rounding). 

- the loop test is done in check.sh. The parameters (type float or double, precision, range, vprec-mode) need to be a subset of reference parameter. To reduce the amount of test, inc can be increased.

- the program fpProp can be used to compute characteristic float-point number (useful to debug).


- if you discover a problem, you can extract the line data and modify the file ../../interflop_backends/interflop-backend-vprec/exemple/debugCheckVPREC.c. You will see immediately if the problem come from the instrumentation or VPREC backend.
