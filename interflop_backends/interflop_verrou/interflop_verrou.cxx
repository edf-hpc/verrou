
/*--------------------------------------------------------------------*/
/*--- Verrou: a FPU instrumentation tool.                          ---*/
/*--- Interface for floating-point operations overloading.         ---*/
/*---                                         interflop_verrou.cxx ---*/
/*--------------------------------------------------------------------*/

/*
   This file is part of Verrou, a FPU instrumentation tool.

   Copyright (C) 2014-2021 EDF
     F. Févotte     <francois.fevotte@edf.fr>
     B. Lathuilière <bruno.lathuiliere@edf.fr>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public License as
   published by the Free Software Foundation; either version 2.1 of the
   License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU Lesser General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307, USA.

   The GNU Lesser General Public License is contained in the file COPYING.
*/


#include "interflop_verrou.h"
#include "vr_nextUlp.hxx"
#include "vr_isNan.hxx"
#include "vr_fma.hxx"
#include <stddef.h>
//extern "C" {
#include "vr_rand.h"
//}




#include "vr_roundingOp.hxx"
#include "vr_op.hxx"


// * Global variables & parameters
int CHECK_C  = 0;
vr_RoundingMode DEFAULTROUNDINGMODE;
vr_RoundingMode ROUNDINGMODE;
unsigned int vr_seed;

#ifdef PROFILING_EXACT
unsigned int vr_NumOp;
unsigned int vr_NumExactOp;
#endif

void (*vr_panicHandler)(const char*)=NULL;
void (*vr_nanHandler)()=NULL;
void (*vr_infHandler)()=NULL;


void verrou_init_profiling_exact(void){
#ifdef PROFILING_EXACT
  vr_NumOp=0;
  vr_NumExactOp=0;
#endif
}

void verrou_get_profiling_exact(unsigned int* num, unsigned int* numExact){
#ifdef PROFILING_EXACT
  *num=vr_NumOp;
  *numExact=vr_NumExactOp;
#endif
}




void verrou_set_panic_handler(void (*panicHandler)(const char*)){
  vr_panicHandler=panicHandler;
}

void verrou_set_nan_handler(void (*nanHandler)()){
  vr_nanHandler=nanHandler;
}

void verrou_set_inf_handler(void (*infHandler)()){
  vr_infHandler=infHandler;
}

void (*vr_debug_print_op)(int,const char*, const double*, const double*)=NULL;
void verrou_set_debug_print_op(void (*printOpHandler)(int nbArg,const char*name, const double* args,const double* res)){
  vr_debug_print_op=printOpHandler;
};


// * Operation implementation
const char*  verrou_rounding_mode_name (enum vr_RoundingMode mode) {
  switch (mode) {
  case VR_NEAREST:
    return "NEAREST";
  case VR_UPWARD:
    return "UPWARD";
  case VR_DOWNWARD:
    return "DOWNWARD";
  case VR_ZERO:
    return "TOWARD_ZERO";
  case VR_RANDOM:
    return "RANDOM";
  case VR_RANDOM_DET:
    return "RANDOM_DET";
  case VR_RANDOM_COMDET:
    return "RANDOM_COMDET";
  case VR_RANDOM_SCOMDET:
    return "RANDOM_SCOMDET";
  case VR_AVERAGE:
    return "AVERAGE";
  case VR_AVERAGE_DET:
    return "AVERAGE_DET";
  case VR_AVERAGE_COMDET:
    return "AVERAGE_COMDET";
  case VR_AVERAGE_SCOMDET:
    return "AVERAGE_SCOMDET";
  case VR_PRANDOM:
    return "PRANDOM";
  case VR_PRANDOM_DET:
    return "PRANDOM_DET";
  case VR_PRANDOM_COMDET:
    return "PRANDOM_COMDET";
  case VR_FARTHEST:
    return "FARTHEST";
  case VR_FLOAT:
    return "FLOAT";
  case VR_NATIVE:
    return "NATIVE";
  case VR_FTZ:
    return "FTZ";
  }

  return "undefined";
}




// * C interface
void IFV_FCTNAME(configure)(vr_RoundingMode mode,void* context) {
  DEFAULTROUNDINGMODE = mode;
  ROUNDINGMODE=mode;
}

void IFV_FCTNAME(finalize)(void* context){
}

const char* IFV_FCTNAME(get_backend_name)() {
  return "verrou";
}

const char* IFV_FCTNAME(get_backend_version)() {
  return "1.x-dev";
}

void verrou_begin_instr(){
  ROUNDINGMODE=DEFAULTROUNDINGMODE;
}

void verrou_end_instr(){
  ROUNDINGMODE= VR_NEAREST;
}

void verrou_set_seed (unsigned int seed) {
  vr_seed = vr_rand_next(&vr_rand);
  vr_rand_setSeed (&vr_rand, seed);
}

void verrou_set_random_seed () {
  vr_rand_setSeed(&vr_rand, vr_seed);
}

void verrou_updatep_prandom (void) {
  const double p=tinymt64_generate_double(&(vr_rand.gen_) );
  vr_rand.p=p;
}

void verrou_updatep_prandom_double (double p) {
  vr_rand.p=p;
}

double verrou_prandom_pvalue (void) {
  return vr_rand.p;
}


#include "interflop_verrou_flop_impl.hxx"


struct interflop_backend_interface_t IFV_FCTNAME(init)(void ** context){
  struct interflop_backend_interface_t config=interflop_backend_empty_interface;

  config.add_float = & IFV_FCTNAME(add_float);
  config.sub_float = & IFV_FCTNAME(sub_float);
  config.mul_float = & IFV_FCTNAME(mul_float);
  config.div_float = & IFV_FCTNAME(div_float);

  config.add_double = & IFV_FCTNAME(add_double);
  config.sub_double = & IFV_FCTNAME(sub_double);
  config.mul_double = & IFV_FCTNAME(mul_double);
  config.div_double = & IFV_FCTNAME(div_double);

  config.cast_double_to_float=& IFV_FCTNAME(cast_double_to_float);

  config.madd_float  = & IFV_FCTNAME(madd_float);
  config.madd_double = & IFV_FCTNAME(madd_double);

  config.finalize = & IFV_FCTNAME(finalize);

  return config;
}


//#ifdef INTERFLOP_DYN_INTERFACE_ENABLED
#ifndef INTERFLOP_STATIC_INTERFACE_ENABLED

#include <argp.h>
#include <string.h>
#include <strings.h>
#include <iostream>
#include <sys/time.h>
#include <unistd.h>

typedef struct verrou_conf{
  vr_RoundingMode mode;
  unsigned int seed;
} verrou_conf_t;

typedef enum {
  KEY_ROUNDING_MODE,
  KEY_SEED
} key_args;

static const char key_rounding_mode_str[] = "rounding-mode";
static const char key_seed_str[] = "seed";


static struct argp_option options[] = {
  {key_rounding_mode_str, KEY_ROUNDING_MODE, "ROUNDING MODE", 0,
   "select rounding mode among {nearest, upward, downward, toward_zero, random, random_det, random_comdet, random_scomdet, average, average_det, average_comdet, average_scomdet, prandom, prandom_det, prandom_comdet, farthest,float,native,ftz}", 0},
  {key_seed_str, KEY_SEED, "SEED", 0, "fix the random generator seed", 0},
  {0}};


static error_t parse_opt(int key, char *arg, struct argp_state *state) {
  verrou_conf_t *conf = (verrou_conf_t *)state->input;

  switch (key) {
  case KEY_ROUNDING_MODE:
    if (strcasecmp("nearest", arg) == 0) {
      conf->mode=VR_NEAREST;
    } else if (strcasecmp("upward", arg) == 0) {
      conf->mode=VR_UPWARD;
    } else if (strcasecmp("downward", arg) == 0) {
      conf->mode=VR_DOWNWARD;
    } else if (strcasecmp("toward_zero", arg) == 0) {
      conf->mode=VR_ZERO;
    } else if (strcasecmp("random", arg) == 0) {
      conf->mode=VR_RANDOM;
    } else if (strcasecmp("random_det", arg) == 0) {
      conf->mode=VR_RANDOM_DET;
    } else if (strcasecmp("random_comdet", arg) == 0) {
      conf->mode=VR_RANDOM_COMDET;
    } else if (strcasecmp("random_scomdet", arg) == 0) {
      conf->mode=VR_RANDOM_SCOMDET;
    } else if (strcasecmp("average", arg) == 0) {
      conf->mode=VR_AVERAGE;
    } else if (strcasecmp("average_det", arg) == 0) {
      conf->mode=VR_AVERAGE_DET;
    } else if (strcasecmp("average_comdet", arg) == 0) {
      conf->mode=VR_AVERAGE_COMDET;
    } else if (strcasecmp("average_scomdet", arg) == 0) {
      conf->mode=VR_AVERAGE_SCOMDET;
    } else if (strcasecmp("prandom", arg) == 0) {
      conf->mode=VR_PRANDOM;
    } else if (strcasecmp("prandom_det", arg) == 0) {
      conf->mode=VR_PRANDOM_DET;
    } else if (strcasecmp("prandom_comdet", arg) == 0) {
      conf->mode=VR_PRANDOM_COMDET;
    } else if (strcasecmp("farthest", arg) == 0) {
      conf->mode=VR_FARTHEST;
    } else if (strcasecmp("float", arg) == 0) {
      conf->mode=VR_FLOAT;
    } else if (strcasecmp("native", arg) == 0) {
      conf->mode=VR_NATIVE;
    } else if (strcasecmp("ftz", arg) == 0) {
      conf->mode=VR_FTZ;
    } else {
      std::cerr << key_rounding_mode_str <<" invalid value provided, must be one of: "
		<<" nearest, upward, downward, toward_zero, random, random_det, random_comdet,average, average_det, average_comdet,farthest,float,native,ftz."
		<<std::endl;
      exit(42);
    }
    break;

  case KEY_SEED:
    /* seed */
    errno = 0;
    char *endptr;
    conf->seed = strtoull(arg, &endptr, 10);
    if (errno != 0) {
      std::cerr << key_seed_str <<" invalid value provided, must be an integer"
		<< std::endl;
      exit(42);
    }
    break;

  default:
    return ARGP_ERR_UNKNOWN;
  }
  return 0;

}

static struct argp argp = {options, parse_opt, "", "", NULL, NULL, NULL};


verrou_conf_t IFV_FCTNAME(parse)(int argc, char** argv, void* context){

  verrou_conf_t conf;
  conf.mode=VR_NEAREST ; //default value
  conf.seed=(unsigned int) -1;

  argp_parse(&argp, argc, argv, 0, 0, &conf);
  std::cerr << "VERROU ROUNDING MODE : " << verrou_rounding_mode_name(conf.mode)<<std::endl;
  return conf;
}


struct interflop_backend_interface_t interflop_init(int argc, char **argv,
                                                    void **context){
  struct interflop_backend_interface_t config=IFV_FCTNAME(init)(context);


  verrou_conf_t conf=IFV_FCTNAME(parse)(argc, argv, *context);
  IFV_FCTNAME(configure)(conf.mode, *context);

  if(conf.seed==(unsigned int) -1){
    struct timeval t1;
    gettimeofday(&t1, NULL);
    conf.seed =  t1.tv_usec + getpid();
  }
  verrou_set_seed (conf.seed);

  return config;
}
#endif
