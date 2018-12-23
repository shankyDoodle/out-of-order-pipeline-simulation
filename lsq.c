//
// Created by Shashank Kaldate on 12/1/18.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cpu.h"

#define LSQ_SIZE 20

void print_lsq_instruction(struct LSQEntry stage) {
  if (strcmp(stage.opcode, "STORE") == 0) {
    printf("%5s,R%-2d,R%-2d,#%-2d ", stage.opcode, stage.rs1_o, stage.rs2_o, stage.imm);
    printf("\t\t[%5s,U%-2d,U%-2d,#%-2d] ", stage.opcode, stage.rs1, stage.rs2, stage.imm);
  }

  if (strcmp(stage.opcode, "LOAD") == 0) {
    printf("%5s,R%-2d,R%-2d,#%-2d ", stage.opcode, stage.rd_o, stage.rs1_o, stage.imm);
    printf("\t\t[%5s,U%-2d,U%-2d,#%-2d] ", stage.opcode, stage.rd, stage.rs1, stage.imm);
  }
}


void print_lsq_entries(APEX_CPU *cpu) {
  printf(" _______________________________________________________________________________\n");
  printf("|\t\t\t\tDetails of LSQ State:\t\t\t\t|\n");
  for (int i = 0; i < LSQ_SIZE; i++) {
    if (cpu->LSQ[i].rd != -1 || cpu->LSQ[i].rs1 != -1) {
      printf("|LSQ[%d]:\t\t", i);
      print_lsq_instruction(cpu->LSQ[i]);
      printf("\t|\n");
    }
  }
  printf(" ¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯\n");
}


struct LSQEntry createLSQEntry(APEX_CPU *cpu) {
  CPU_Stage *stage = &cpu->stage[FQ];
  struct LSQEntry temp;

  temp.rd = stage->rd;
  temp.imm = stage->imm;

  temp.imm = stage->imm;
  temp.rd = stage->rd;
  temp.rd_o = stage->rd_o;

  temp.rs1 = stage->rs1;
  temp.rs1_o = stage->rs1_o;
  temp.rs1_value = cpu->unifiedRF[stage->rs1].value;
  temp.rs1_ready = !cpu->unifiedRF[stage->rs1].destLock;

  temp.rs2 = stage->rs2;
  temp.rs2_o = stage->rs2_o;

  strcpy(temp.opcode, stage->opcode);

  temp.pc = stage->pc;

  temp.readyToDispatch = 0;
  temp.mem_address = -1;
  temp.cfID = cpu->recentCFID;

  return temp;
}

int flushLSQEntryByIndex(APEX_CPU *cpu, int i) {
  cpu->LSQ[i].readyToDispatch = 0;
  cpu->LSQ[i].mem_address = -1;
  cpu->LSQ[i].cfID = -1;
  cpu->LSQ[i].pc = 9999; // at which clock cycle instruction entered

  strcpy(cpu->LSQ[i].opcode, "");
  cpu->LSQ[i].rs1 = -1;
  cpu->LSQ[i].rs1_ready = -1;
  cpu->LSQ[i].rs1_value = -1;

  cpu->LSQ[i].rs2 = -1;
  cpu->LSQ[i].rd = -1; //for load. default it should be -1. do take care of this.
  cpu->LSQ[i].imm = -1;

  cpu->LSQ[i].rs1_o = -1; //original reg values
  cpu->LSQ[i].rs2_o = -1;
  cpu->LSQ[i].rd_o = -1;
  return 0;
}

int insertInLSQ(APEX_CPU *cpu) {
  struct LSQEntry lsqEntry = createLSQEntry(cpu);

  int lsqIndex = -1;
  if (cpu->lsqRear < LSQ_SIZE) {
    cpu->LSQ[cpu->lsqRear] = lsqEntry;
    lsqIndex = cpu->lsqRear;
    cpu->lsqRear++;
  }

  if (cpu->lsqRear == LSQ_SIZE) {
    cpu->lsqFull = 1;
  }

  return lsqIndex;
}

void fillUpFUFromLSQ(APEX_CPU *cpu, int indexLSQ) {
  cpu->FUs[2].rs1 = cpu->LSQ[indexLSQ].rs1;
  cpu->FUs[2].rs1_o = cpu->LSQ[indexLSQ].rs1_o;
  cpu->FUs[2].rs1_value = cpu->LSQ[indexLSQ].rs1_value;

  cpu->FUs[2].rs2 = cpu->LSQ[indexLSQ].rs2;
  cpu->FUs[2].rs2_o = cpu->LSQ[indexLSQ].rs2_o;
  cpu->FUs[2].rs2_value = -1;

  strcpy(cpu->FUs[2].opcode, cpu->LSQ[indexLSQ].opcode);

  cpu->FUs[2].rd = cpu->LSQ[indexLSQ].rd;
  cpu->FUs[2].rd_o = cpu->LSQ[indexLSQ].rd_o;
  cpu->FUs[2].imm = cpu->LSQ[indexLSQ].imm;

  cpu->FUs[2].lsqIndex = -1;
  cpu->FUs[2].mem_address = cpu->LSQ[indexLSQ].mem_address;
  cpu->FUs[2].pc = cpu->LSQ[indexLSQ].pc;
}

struct functionalUnits fetchInstructionFromLSQForMemFU(APEX_CPU *cpu) {

  fillUpFUFromLSQ(cpu, 0);
  //shift all elements by 1 left.
  for (int i = 0; i < LSQ_SIZE; i++) {
    cpu->LSQ[i] = cpu->LSQ[i + 1];
  }
  flushLSQEntryByIndex(cpu, LSQ_SIZE - 1);

  cpu->lsqRear--;
  cpu->lsqFull = 0;

  return cpu->FUs[2];
}

int initializeLSQ(APEX_CPU *cpu) {

  for (int i = 0; i < LSQ_SIZE; i++) {
    flushLSQEntryByIndex(cpu, i);
  }
  cpu->lsqRear = 0;
  cpu->lsqFull = 0;

  return 0;
}

void deleteLSQEntryByIndex(APEX_CPU *cpu, int desiredIndex) {
  //shift all elements by 1 left.
  for (int i = desiredIndex; i < LSQ_SIZE; i++) {
    cpu->LSQ[i] = cpu->LSQ[i + 1];
  }
  flushLSQEntryByIndex(cpu, LSQ_SIZE - 1);
}

int handleLSQFlushByCFID(APEX_CPU *cpu, int cfId) {
  while(cpu->LSQ[cpu->lsqRear-1].cfID == cfId){
    deleteLSQEntryByIndex(cpu, cpu->lsqRear-1);
    cpu->lsqRear--;
    cpu->lsqFull = 0;
  }

  return 0;
}
