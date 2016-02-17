#ifndef __VERROU_H
#define __VERROU_H

#include "valgrind.h"

/* !! ABIWARNING !! ABIWARNING !! ABIWARNING !! ABIWARNING !!
   This enum comprises an ABI exported by Valgrind to programs
   which use client requests.  DO NOT CHANGE THE ORDER OF THESE
   ENTRIES, NOR DELETE ANY -- add new ones at the end.
 */

typedef
enum {
  VR_USERREQ__START_INSTRUMENTATION = VG_USERREQ_TOOL_BASE('V', 'R'),
  VR_USERREQ__STOP_INSTRUMENTATION,
  VR_USERREQ__START_DETERMINISTIC,
  VR_USERREQ__STOP_DETERMINISTIC
} Vg_VerrouClientRequest;

#define VERROU_START_INSTRUMENTATION                                 \
  VALGRIND_DO_CLIENT_REQUEST_STMT(VR_USERREQ__START_INSTRUMENTATION, \
                                  0, 0, 0, 0, 0)

#define VERROU_STOP_INSTRUMENTATION                                  \
  VALGRIND_DO_CLIENT_REQUEST_STMT(VR_USERREQ__STOP_INSTRUMENTATION,  \
                                  0, 0, 0, 0, 0)

#define VERROU_START_DETERMINISTIC(LEVEL)                            \
  VALGRIND_DO_CLIENT_REQUEST_STMT(VR_USERREQ__START_DETERMINISTIC,   \
                                  LEVEL, 0, 0, 0, 0)

#define VERROU_STOP_DETERMINISTIC(LEVEL)                             \
  VALGRIND_DO_CLIENT_REQUEST_STMT(VR_USERREQ__STOP_DETERMINISTIC,    \
                                  LEVEL, 0, 0, 0, 0)




#endif /* __VERROU_H */
