//
// Created by Shashank Kaldate on 11/30/18.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cpu.h"

#define CF_IO_SIZE 8

struct CFIOEntry createDummyCFIOEntry() {
  struct CFIOEntry dummyCFIOEntry;

  strcpy(dummyCFIOEntry.opcode, "");
  dummyCFIOEntry.rs1 = -1;
  dummyCFIOEntry.rs2 = -1;
  dummyCFIOEntry.rd = -1;
  dummyCFIOEntry.imm = -1;

  dummyCFIOEntry.isAvailable = 0;
  dummyCFIOEntry.pc = 9999;
  dummyCFIOEntry.cfID = -1;
  return dummyCFIOEntry;
}

struct CFIOEntry createCFIOEntry(APEX_CPU *cpu) {
  CPU_Stage *stage = &cpu->stage[FQ];
  struct CFIOEntry temp;

  strcpy(temp.opcode, stage->opcode);
  temp.rs1 = stage->rs1;
  temp.rs2 = stage->rs2;
  temp.rd = stage->rd;
  temp.imm = stage->imm;

  temp.pc = stage->pc;
  temp.isAvailable = 0;

  temp.cfID = rand() % 1000 + 1; //random UUID;
  cpu->recentCFID = temp.cfID;

  return temp;
}

int insertInCFIO(APEX_CPU *cpu) {
  struct CFIOEntry cfioEntry = createCFIOEntry(cpu);

  if (cpu->cfioRear < CF_IO_SIZE) {
    cpu->CFIO[cpu->cfioRear] = cfioEntry;
    cpu->cfioRear++;
  }

  if (cpu->cfioRear == CF_IO_SIZE) {
    cpu->cfioFull = 1;
  }
  return 0;
}

struct CFIOEntry deleteCFIOEntryByIndex(APEX_CPU *cpu, int index) {

  struct CFIOEntry retiredCFIOEntry = cpu->CFIO[index];

  //shift all elements by 1 left.
  for (int i = index; i <= cpu->cfioRear; i++) {
    cpu->CFIO[i] = cpu->CFIO[i + 1];
  }

  // flush last entry of CFIO
  cpu->CFIO[cpu->cfioRear] = createDummyCFIOEntry();

  cpu->cfioRear--;
  cpu->cfioFull = 0;

  return retiredCFIOEntry;
}

int initializeCFIO(APEX_CPU *cpu) {

  for (int i = 0; i < CF_IO_SIZE; i++) {
    cpu->CFIO[i] = createDummyCFIOEntry();
  }
  cpu->cfioRear = 0;
  cpu->cfioFull = 0;
  cpu->recentCFID = -1;
  cpu->zFlag = -1;

  return 0;
}

void doFlushContainer(APEX_CPU *cpu, int cfID, int deleteSelfCFIQEntry) {
  handleIQFlushByCFID(cpu, cfID);
  handleLSQFlushByCFID(cpu, cfID);
  handleROBFlushByCFID(cpu, cfID);
  if (deleteSelfCFIQEntry) {
    deleteCFIOEntryByIndex(cpu, cpu->cfioRear - 1);
  }
}

int flushAllQuesDueToBranching(APEX_CPU *cpu, struct functionalUnits stage) {
  while (cpu->CFIO[cpu->cfioRear - 1].cfID != stage.cfID) {
    doFlushContainer(cpu, stage.cfID, 1);
  }

  if (cpu->CFIO[cpu->cfioRear - 1].cfID == stage.cfID) {
    int deleteSelfCFIQEntry = 0;
    doFlushContainer(cpu, stage.cfID, deleteSelfCFIQEntry);
  }
  return 0;
}

int updateCFIOFromROBRetire(APEX_CPU *cpu, struct ROBEntry retiredROBEntry) {
  if (
      strcmp(retiredROBEntry.opcode, "BZ") == 0 ||
      strcmp(retiredROBEntry.opcode, "BNZ") == 0 ||
      strcmp(retiredROBEntry.opcode, "JUMP") == 0 ||
      strcmp(retiredROBEntry.opcode, "JAL") == 0
      ) {
    if (retiredROBEntry.cfID == cpu->CFIO[0].cfID) {
      deleteCFIOEntryByIndex(cpu, 0);
    } else {
      printf("\nWarning: at ROB retire for CFIO. You can ignore this.\n");
    }
  }

  return 0;
};
