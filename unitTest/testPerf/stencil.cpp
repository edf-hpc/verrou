/*
  Copyright (c) 2010-2014, Intel Corporation
  All rights reserved.

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions are
  met:

    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.

    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.

    * Neither the name of Intel Corporation nor the names of its
      contributors may be used to endorse or promote products derived from
      this software without specific prior written permission.


   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
   IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
   TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
   PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER
   OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
   EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
   PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
   PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
   LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
   NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
   SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.  
*/

#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#define NOMINMAX
#pragma warning (disable: 4244)
#pragma warning (disable: 4305)
#endif

#include <cstdlib>
#include <stdio.h>
#include <algorithm>
#include <string.h>
#include <math.h>
#include "./timing.h"
#include <iostream>
#include <fstream>
#include <iomanip>
//#include "valgrind/verrou.h"
//#include "stencil_ispc.h"
//using namespace ispc;

#ifdef FLOAT
typedef  float RealType;
#else
typedef double RealType;
#endif

extern void loop_stencil_serial(int t0, int t1, int x0, int x1,
                                int y0, int y1, int z0, int z1,
                                int Nx, int Ny, int Nz,
                                const RealType coef[5],
                                const RealType vsq[],
                                RealType Aeven[], RealType Aodd[]);

#ifndef STENCIL_WITH_FMA
static void
stencil_step(int x0, int x1,
             int y0, int y1,
             int z0, int z1,
             int Nx, int Ny, int Nz,
             const RealType coef[4], const RealType vsq[],
             const RealType Ain[], RealType Aout[]) {
    int Nxy = Nx * Ny;

    for (int z = z0; z < z1; ++z) {
        for (int y = y0; y < y1; ++y) {
            for (int x = x0; x < x1; ++x) {
                int index = (z * Nxy) + (y * Nx) + x;
#define A_cur(x, y, z) Ain[index + (x) + ((y) * Nx) + ((z) * Nxy)]
#define A_next(x, y, z) Aout[index + (x) + ((y) * Nx) + ((z) * Nxy)]
                RealType div = coef[0] * A_cur(0, 0, 0) +
                            coef[1] * (A_cur(+1, 0, 0) + A_cur(-1, 0, 0) +
                                       A_cur(0, +1, 0) + A_cur(0, -1, 0) +
                                       A_cur(0, 0, +1) + A_cur(0, 0, -1)) +
                            coef[2] * (A_cur(+2, 0, 0) + A_cur(-2, 0, 0) +
                                       A_cur(0, +2, 0) + A_cur(0, -2, 0) +
                                       A_cur(0, 0, +2) + A_cur(0, 0, -2)) +
                            coef[3] * (A_cur(+3, 0, 0) + A_cur(-3, 0, 0) +
                                       A_cur(0, +3, 0) + A_cur(0, -3, 0) +
                                       A_cur(0, 0, +3) + A_cur(0, 0, -3));

                A_next(0, 0, 0) = 2 * A_cur(0, 0, 0) - A_next(0, 0, 0) +
                    vsq[index] * div;
            }
        }
    }
}
#endif

#ifdef STENCIL_WITH_FMA

#if defined(__x86_64__)
#include  <immintrin.h>
#include  <fmaintrin.h>
#elif defined(__aarch64__)
#include "arm_neon.h"
#else
#error "fma undef"
#endif

  inline double myFma(const double& a, const double& b, const double& c){
    double d;
#if defined(__x86_64__)
    __m128d ai, bi,ci,di;
    ai = _mm_load_sd(&a);
    bi = _mm_load_sd(&b);
    ci = _mm_load_sd(&c);
    di=_mm_fmadd_sd(ai,bi,ci);
    d=_mm_cvtsd_f64(di);

#elif defined(__aarch64__)
  const float64x1_t ai=vld1_f64(&a);
  const float64x1_t bi=vld1_f64(&b);
  const float64x1_t ci=vld1_f64(&c);

  const float64x1_t di=vfma_f64(ci,ai,bi);// warning strange argument order
  // cf doc : https://developer.arm.com/architectures/instruction-set/intrinsics/#q=vfma
  vst1_f64(&d, di);
#else
#error "fma undef"
#endif
    return d;
  }


  inline float myFma(const float& a, const float& b, const float& c){
#if defined(__x86_64__)
    float d;
    __m128 ai, bi,ci,di;
    ai = _mm_load_ss(&a);
    bi = _mm_load_ss(&b);
    ci = _mm_load_ss(&c);
    di=_mm_fmadd_ss(ai,bi,ci);
    d=_mm_cvtss_f32(di);
    return d;
#elif defined(__aarch64__)
  float av[2]={a,0};
  float bv[2]={b,0};
  float cv[2]={c,0};

  float32x2_t ap=vld1_f32(av);
  float32x2_t bp=vld1_f32(bv);
  float32x2_t cp=vld1_f32(cv);

  float32x2_t resp= vfma_f32(cp,ap,bp); // warning strange argument order
  // cf doc : https://developer.arm.com/architectures/instruction-set/intrinsics/#q=vfma
  float res[2];
  vst1_f32(res, resp);
  return res[0];
#else
#error "fma undef"
#endif
  }



static void
stencil_step(int x0, int x1,
             int y0, int y1,
             int z0, int z1,
             int Nx, int Ny, int Nz,
             const RealType coef[4], const RealType vsq[],
             const RealType Ain[], RealType Aout[]) {
    int Nxy = Nx * Ny;

    for (int z = z0; z < z1; ++z) {
        for (int y = y0; y < y1; ++y) {
            for (int x = x0; x < x1; ++x) {
                int index = (z * Nxy) + (y * Nx) + x;
#define A_cur(x, y, z) Ain[index + (x) + ((y) * Nx) + ((z) * Nxy)]
#define A_next(x, y, z) Aout[index + (x) + ((y) * Nx) + ((z) * Nxy)]
                RealType div = coef[0] * A_cur(0, 0, 0);
		const RealType v1=A_cur(+1, 0, 0) + A_cur(-1, 0, 0) +
		  A_cur(0, +1, 0) + A_cur(0, -1, 0) +
		  A_cur(0, 0, +1) + A_cur(0, 0, -1);

		const RealType v2=(A_cur(+2, 0, 0) + A_cur(-2, 0, 0) +
				   A_cur(0, +2, 0) + A_cur(0, -2, 0) +
				   A_cur(0, 0, +2) + A_cur(0, 0, -2));
		const RealType v3=(A_cur(+3, 0, 0) + A_cur(-3, 0, 0) +
				   A_cur(0, +3, 0) + A_cur(0, -3, 0) +
				   A_cur(0, 0, +3) + A_cur(0, 0, -3));
		div=std::fma(coef[1],v1,div);
		div=std::fma(coef[2],v2,div);
		div=std::fma(coef[3],v3,div);

		A_next(0, 0, 0) = 2 * A_cur(0, 0, 0) - A_next(0, 0, 0) +
		  vsq[index] * div;
	    }
        }
    }
}
#endif


void loop_stencil_serial(int t0, int t1,
                         int x0, int x1,
                         int y0, int y1,
                         int z0, int z1,
                         int Nx, int Ny, int Nz,
                         const RealType coef[4],
                         const RealType vsq[],
                         RealType Aeven[], RealType Aodd[])
{
    for (int t = t0; t < t1; ++t) {
      if ((t & 1) == 0){
	stencil_step(x0, x1, y0, y1, z0, z1, Nx, Ny, Nz, coef, vsq, Aeven, Aodd);
      }
      else{
	stencil_step(x0, x1, y0, y1, z0, z1, Nx, Ny, Nz, coef, vsq, Aodd, Aeven);
      }
    }
}



void InitData(int Nx, int Ny, int Nz, RealType *A[2], RealType *vsq) {
    int offset = 0;
    for (int z = 0; z < Nz; ++z)
        for (int y = 0; y < Ny; ++y)
            for (int x = 0; x < Nx; ++x, ++offset) {
                A[0][offset] = (x < Nx / 2) ? x / RealType(Nx) : y / RealType(Ny);
                A[1][offset] = 0;
                vsq[offset] = x*y*z / RealType(Nx * Ny * Nz);
            }
}


int main(int argc, char *argv[]) {
    static unsigned int test_iterations=1;
    int Nx = 256, Ny = 256, Nz = 256;
    int width = 4;

    if (argc > 1) {
        if (strncmp(argv[1], "--scale=", 8) == 0) {
            RealType scale = atof(argv[1] + 8);
            Nx = Nx * scale;
            Ny = Ny * scale;
            Nz = Nz * scale;
        }
    }
    if ((argc == 3)) {
      test_iterations = atoi(argv[2]);
    }

    //Allocation and initialisation
    RealType *Aserial[2];
    Aserial[0] = new RealType [Nx * Ny * Nz];
    Aserial[1] = new RealType [Nx * Ny * Nz];
    RealType *vsq = new RealType [Nx * Ny * Nz];

    RealType coeff[4] = { 0.5, -.25, .125, -.0625 };


    double minTimeSerial = 1e30;
    for (unsigned int i = 0; i < test_iterations; ++i) {

      InitData(Nx, Ny, Nz, Aserial, vsq);

      reset_and_start_timer();
      //	VERROU_PRINT_PROFILING_EXACT;
      //VERROU_START_INSTRUMENTATION;
      loop_stencil_serial(0, 6, width, Nx-width, width, Ny - width,
			  width, Nz - width, Nx, Ny, Nz, coeff, vsq,
			  Aserial[0], Aserial[1]);
      //VERROU_STOP_INSTRUMENTATION;
	//	VERROU_PRINT_PROFILING_EXACT;
      double dt = get_elapsed_sec();
      printf("@time of serial run:\t\t\t[%.3f] secondes\n", dt);
      minTimeSerial = std::min(minTimeSerial, dt);
    }
    printf("@mintime of serial run:\t\t\t[%.3f] secondes\n", minTimeSerial);


    // printf("\t\t\t\t(%.2fx speedup from ISPC, %.2fx speedup from ISPC + tasks)\n", 
    //        minTimeSerial / minTimeISPC, minTimeSerial / minTimeISPCTasks);

    // Check for agreement
    int offset = 0;
    RealType norm=0;
    for (int z = 0; z < Nz; ++z){
      for (int y = 0; y < Ny; ++y){
	for (int x = 0; x < Nx; ++x, ++offset) {
	  RealType value= Aserial[1][offset];
	  norm += value*value;
	}
      }
    }
    std::cout << std::setprecision(16)<< "norm: " << sqrt(norm)<<std::endl;

    delete[] Aserial[0];
    delete[] Aserial[1];
    delete[] vsq;
}
