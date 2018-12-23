#include <stdio.h>
#include <stdlib.h>

#include "cpu.h"

int main(int argc, char const *argv[]) {
  /*//TODO: remove before commit.
  argc = 2;
  argv[1] = "./../input.asm";*/

  if (argc < 4) {
    fprintf(stderr, "APEX_Help : Usage %s <input_file>\n", argv[0]);
    exit(1);
  }

  APEX_CPU *cpu = APEX_cpu_init(argv[1]);
  if (!cpu) {
    fprintf(stderr, "APEX_Error : Unable to initialize CPU\n");
    exit(1);
  }

  const char* functionality;
  const char* cycleCount;
  if(argc >2){
    functionality = argv[2];
    cycleCount = argv[3];
  }else{
    functionality = "display";
    cycleCount = "0";
  }

  APEX_cpu_run(cpu, functionality, cycleCount);
  APEX_cpu_stop(cpu);
  return 0;
}