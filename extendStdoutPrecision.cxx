#include <stdio.h>
#include <iostream>
#include <iomanip>
#include <unistd.h>
#include <sys/time.h>
#include <dlfcn.h>

/*extern "C" {
  int __libc_start_main(
			int (*main)(int, char **, char **),
			int argc, char **argv,
			int (*init)(int, char **, char **),
			void (*fini)(void),
			void (*rtld_fini)(void),
			void (*stack_end)){
    std::cout << std::setprecision(17);
    typeof(&__libc_start_main)  startMainPtr = (typeof(&__libc_start_main))dlsym(RTLD_NEXT, "__libc_start_main");
    return startMainPtr(main, argc, argv, init, fini, rtld_fini, stack_end);
  }
  }*/


extern "C" {
  typedef void(*VoidFuncPtr)();
  void _ZNSt8ios_base4InitC1Ev(void){
   VoidFuncPtr  startIOBasePtr = (VoidFuncPtr)dlsym(RTLD_NEXT, "_ZNSt8ios_base4InitC1Ev");
   if(startIOBasePtr!=NULL){
     startIOBasePtr();
     std::cout << std::setprecision(17);
   }
  }
}
