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

//#include "stencil_ispc.h"
//using namespace ispc;
void* context;
#include "../../interflop_verrou.h"

#ifdef FLOAT
typedef  float RealType;
#define add(a,b,c)  interflop_verrou_add_float(a,b,c,context);
#define sub(a,b,c)  interflop_verrou_sub_float(a,b,c,context);
#define mul(a,b,c)  interflop_verrou_mul_float(a,b,c,context);
#define div(a,b,c)  interflop_verrou_div_float(a,b,c,context);
#else
typedef double RealType;
#define add(a,b,c)  interflop_verrou_add_double(a,b,c,context);
#define sub(a,b,c)  interflop_verrou_sub_double(a,b,c,context);
#define mul(a,b,c)  interflop_verrou_mul_double(a,b,c,context);
#define div(a,b,c)  interflop_verrou_div_double(a,b,c,context);
#endif

extern void loop_stencil_serial(int t0, int t1, int x0, int x1,
                                int y0, int y1, int z0, int z1,
                                int Nx, int Ny, int Nz,
                                const RealType coef[5], 
                                const RealType vsq[],
                                RealType Aeven[], RealType Aodd[]);


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
                RealType div = coef[0];
		mul(div,A_cur(0, 0, 0),&div);		  

		RealType acc1=A_cur(+1, 0, 0);
		add(acc1,A_cur(-1, 0, 0),&acc1);
		add(acc1,A_cur(0, +1, 0),&acc1);
		add(acc1,A_cur(0, -1, 0),&acc1);
		add(acc1,A_cur(0, 0, +1),&acc1);
		add(acc1,A_cur(0, 0, -1),&acc1);
		
		mul(acc1,coef[1],&acc1);
		
		RealType acc2=A_cur(+2, 0, 0);
		add(acc2,A_cur(-2, 0, 0),&acc2);
		add(acc2,A_cur(0, +2, 0),&acc2);
		add(acc2,A_cur(0, -2, 0),&acc2);
		add(acc2,A_cur(0, 0, +2),&acc2);
		add(acc2,A_cur(0, 0, -2),&acc2);
		
		mul(acc2,coef[2],&acc2);
		
		RealType acc3=A_cur(+3, 0, 0);
		add(acc3,A_cur(-3, 0, 0),&acc3);
		add(acc3,A_cur(0, +3, 0),&acc3);
		add(acc3,A_cur(0, -3, 0),&acc3);
		add(acc3,A_cur(0, 0, +3),&acc3);
		add(acc3,A_cur(0, 0, -3),&acc3);
		
		mul(acc3,coef[3],&acc3);

		add(div,acc1,&div);		  
		add(div,acc2,&div);		  
		add(div,acc2,&div);		  


		

                //A_next(0, 0, 0) = 2 * A_cur(0, 0, 0) - A_next(0, 0, 0) + 
		//                    vsq[index] * div;
		RealType localAcc=2.;
		add(localAcc,  A_cur(0, 0, 0),&localAcc);
		sub(localAcc,  A_next(0, 0, 0),&localAcc);
		mul(div,vsq[index],&div);		  

		add(localAcc, div,&A_next(0, 0, 0));		      
            }
        }
    }
}


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
        if ((t & 1) == 0)
            stencil_step(x0, x1, y0, y1, z0, z1, Nx, Ny, Nz, coef, vsq, 
                         Aeven, Aodd);
        else
            stencil_step(x0, x1, y0, y1, z0, z1, Nx, Ny, Nz, coef, vsq, 
                         Aodd, Aeven);
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

    //struct interflop_backend_interface_t ifverrou=interflop_verrou_init(&context);
    interflop_verrou_init(&context);
    interflop_verrou_configure(VR_AVERAGE, context);
    //    interflop_verrou_configure(VR_NEAREST, context);
    uint64_t seed(42);
    verrou_set_seed (seed);
    
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

      loop_stencil_serial(0, 6, width, Nx-width, width, Ny - width,
			  width, Nz - width, Nx, Ny, Nz, coeff, vsq,
			  Aserial[0], Aserial[1]);

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
}
