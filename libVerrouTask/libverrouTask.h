
/*
   This file is part of Verrou, a FPU instrumentation tool.

   Copyright (C) 2014-2021 EDF
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
#pragma once

/*
List of env variable :
DEBUG_PRINT_TASK
TASK_LIST
GENERATE_TASK_LIST
GENERATE_TASK_FP_LIST
 */


#ifdef __cplusplus
extern "C" {
#endif
  void verrou_task_init();
  void verrou_task(char const*const key, int index);
  void verrou_task_finalyze();
#ifdef __cplusplus
};
#endif


#define VERROU_TASK_INIT verrou_task_init();
#define VERROU_TASK(a,b) verrou_task(a,b);
#define VERROU_TASK_FINALIZE verrou_task_finalyze();
