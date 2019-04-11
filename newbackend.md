This document aims to explain shortly how to add a new backend in verrou.


1- add a new rep backend_NEWNAME with the same structure as backend_verrou
   Remark : there are Makefile and test_main.cxx to test the backend without frontend

2- add the new source file  in Makefile.am  (You may need to do again ./autogen.sh && ./configure .... cf. README.md)

#At this step you can test compilation

3- modify vr_main.h :
   - add #include "backend_mcaquad/interflop_NEWNAME.h"
   - add vr_NEWNAME in  enum vr_backend_name
   - add usefull data for backend configuration in Vr_State struct

#At this step you can test compilation

4- modify the file generateBackendInterOperator.py
   - In the __main__ section add NEWNAME in the generateNArgs call.
   - generate a new vr_generated_from_templates.h file : ./generateBackendInterOperator.py
   - Remark : both file are followed in git

#At this step you can test compilation

4- modify vr_main.c :
   - add instanciation of backend_NEWNAME and  backend_NEWNAME_context as for verrou or mcaquad
   - modify vr_instrumentOp with a new switch. If there are missing call in the backend you can
   change the inclusion with #define macro. You may need to customize the file vr_instrumentOp_impl.h.
   - add   interflop_NEWNAME_finalyze(backend_NEWNAME_context) in vr_fini

   - configure the backend in vr_post_clo_init
   - add message to display option taken into account

#At this step you can test compilation

5- modify vr_clo.c :
   - default option in vr_clo_defaults (initialisation of new attribut defined in vr_main.h )
   - add user option in vr_process_clo

#At this step you can test compilation and the use

6- modify vr_clreq.c [if backend use random generator]
   - add NEWBACKEND_set_seed(hash) in  vr_[start/stop]_deterministic_section functions

7- add client-request to configure dynamicly the backend [optionnal]

8- update documentation
  - modify vr-manual.xml
  - compile documentation : make -C docs html-docs man-pages
  - adapt docs/update-vr-clo to install path [no need to commit]
  - generate new vr_clo.txt (to be commited) from valgrind directory : ./verrou/docs/update-vr-clo
