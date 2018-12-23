//
// Created by Shashank Kaldate on 12/1/18.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cpu.h"

#define RAT_SIZE 16
#define R_RAT_SIZE 16
#define URF_SIZE 40

/***************** URF functions *****************/
int initializeURF(APEX_CPU *cpu) {
  for (int i = 0; i < URF_SIZE; i++) {
    cpu->unifiedRF[i].value = -1;
    cpu->unifiedRF[i].isAvailable = 1;
    cpu->unifiedRF[i].destLock = 0;
  }

  return 0;
}

/***************** R-RAT functions **********************/
void updateBackEndRenameTable(APEX_CPU *cpu, struct ROBEntry retiredROBEntry) {

  // Free URF reg when re-namer instruction is encountered.
  if (cpu->backEndRenameTable[retiredROBEntry.rd_o].value != -1) {
    int oldURF = cpu->backEndRenameTable[retiredROBEntry.rd_o].value;
    cpu->unifiedRF[oldURF].destLock = 0;
    cpu->unifiedRF[oldURF].value = -1;
    cpu->unifiedRF[oldURF].isAvailable = 1;
  }

  cpu->backEndRenameTable[retiredROBEntry.rd_o].value = retiredROBEntry.rd;
}

int initializeBackEndRenameTable(APEX_CPU *cpu) {
  for (int i = 0; i < R_RAT_SIZE; i++) {
    cpu->backEndRenameTable[i].value = -1;
    cpu->backEndRenameTable[i].zFlag = -1;
  }

  return 0;
}

void print_backend_rename_table_entries(APEX_CPU *cpu) {
  printf(" _______________________________________________\n");
  printf("|\t\tDetails of R-RAT State:\t\t|\n");
  for (int i = 0; i < R_RAT_SIZE; i++) {
    if (cpu->backEndRenameTable[i].value != -1) {
      printf("|\t  R-RAT[%d]\t\tU%d\t\t|\n", i, cpu->backEndRenameTable[i].value);
    }
  }
  printf(" ¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯\n");
};

/***************** Rename Table functions *****************/
void print_rename_table_entries(APEX_CPU *cpu) {
  printf(" _______________________________________________\n");
  printf("|\t\tDetails of RAT State:\t\t|\n");
  for (int i = 0; i < RAT_SIZE; i++) {
    if (cpu->renameTable[i].value != -1) {
      printf("|\t\tRAT[%d]\t\tU%d\t\t|\n", i, cpu->renameTable[i].value);
    }
  }
  printf(" ¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯\n");

};


int initializeRenameTable(APEX_CPU *cpu) {
  for (int i = 0; i < RAT_SIZE; i++) {
    cpu->renameTable[i].value = -1;
    cpu->renameTable[i].zFlag = -1;
  }

  return 0;
}

int getRenamedRegisterForSrcReg(int rs, APEX_CPU *cpu) {
  if (cpu->renameTable[rs].value == -1) {
    for (int j = 0; j < URF_SIZE; j++) {
      if (cpu->unifiedRF[j].isAvailable) {
        cpu->renameTable[rs].value = j;
        cpu->unifiedRF[j].isAvailable = 0;
        break;
      }
    }
  }

  return cpu->renameTable[rs].value;
}

int getRenamedRegisterForDestReg(int rd, APEX_CPU *cpu) {
  for (int j = 0; j < URF_SIZE; j++) {
    if (cpu->unifiedRF[j].isAvailable) {
      cpu->renameTable[rd].value = j;
      cpu->unifiedRF[j].isAvailable = 0;
      cpu->unifiedRF[j].destLock = 1;
      break;
    }
  }

  return cpu->renameTable[rd].value;
}

int registerRenaming(APEX_CPU *cpu) {
  CPU_Stage *stage = &cpu->stage[DRF];
  stage->rs1 = stage->rs1 != -1 ? getRenamedRegisterForSrcReg(stage->rs1, cpu) : -1;
  stage->rs2 = stage->rs2 != -1 ? getRenamedRegisterForSrcReg(stage->rs2, cpu) : -1;
  stage->rd = stage->rd != -1 ? getRenamedRegisterForDestReg(stage->rd, cpu) : -1;

  return 0;
}

void flushRenameTableCopyByIndex(APEX_CPU *cpu, int index){
  cpu->renameTableCopy[index].pc = -1;

  for (int j = 0; j < RAT_SIZE; j++) {
    cpu->renameTableCopy[index].renameTable_o[j].zFlag = -1;
    cpu->renameTableCopy[index].renameTable_o[j].value = -1;
  }
}

void restoreRATDataState(APEX_CPU *cpu, struct functionalUnits stage) {
  //update URF state via ROB.
  int robLastIndex = cpu->robRear - 1;
  while (cpu->ROB[robLastIndex].pc > stage.pc) {
    cpu->unifiedRF[cpu->ROB[robLastIndex].rd].destLock = 0;
    cpu->unifiedRF[cpu->ROB[robLastIndex].rd].value = -1;
    cpu->unifiedRF[cpu->ROB[robLastIndex].rd].isAvailable = 1;
    robLastIndex--;
  }

  /*int skipIndex = -1;
  if (strcmp(stage.opcode, "JAL") == 0) {
    skipIndex = stage.rd;
  }

  for (int i = 0; i < RAT_SIZE; i++) {
    if (skipIndex == -1) {
      cpu->renameTable[i].value = cpu->backEndRenameTable[i].value;
      cpu->renameTable[i].zFlag = cpu->backEndRenameTable[i].zFlag;
    }
  }*/

  //restore renameTable from renameCopy
  int desiredIndex = -1;
  for (int i = 0; i < 17; i++) {
    if (cpu->renameTableCopy[i].pc == stage.pc) {
      desiredIndex = i;
      break;
    }
  }

  for (int i = 0; i < RAT_SIZE; i++) {
      cpu->renameTable[i].value = cpu->renameTableCopy[desiredIndex].renameTable_o[i].value;
      cpu->renameTable[i].zFlag = cpu->renameTableCopy[desiredIndex].renameTable_o[i].zFlag;
  }
  flushRenameTableCopyByIndex(cpu, desiredIndex);

}

void initializeRenameTableCopy(APEX_CPU *cpu) {
  for (int i = 0; i < RAT_SIZE; i++) {
    cpu->renameTableCopy[i].pc = -1;

    for (int j = 0; j < RAT_SIZE; j++) {
      cpu->renameTableCopy[i].renameTable_o[j].zFlag = -1;
      cpu->renameTableCopy[i].renameTable_o[j].value = -1;
    }
  }
}

void keepCopyOfRenameTable(APEX_CPU *cpu, CPU_Stage *stage) {

  int emptyLocationIndex = -1;
  for (int i = 0; i < 17; i++) {
    if (cpu->renameTableCopy[i].pc == -1) {
      emptyLocationIndex = i;
      break;
    }
  }

  cpu->renameTableCopy[emptyLocationIndex].pc = stage->pc;

  for (int j = 0; j < 16; j++) {
    cpu->renameTableCopy[emptyLocationIndex].renameTable_o[j] = cpu->renameTable[j];
  }
}