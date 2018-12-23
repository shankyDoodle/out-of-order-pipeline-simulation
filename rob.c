//
// Created by Shashank Kaldate on 12/1/18.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cpu.h"

#define ROB_SIZE 32

void print_rob_instruction(struct ROBEntry stage) {
  if (strcmp(stage.opcode, "STORE") == 0) {
    printf("%5s,R%-2d,R%-2d,#%-2d ", stage.opcode, stage.rs1_o, stage.rs2_o, stage.imm);
    printf("\t\t[%5s,U%-2d,U%-2d,#%-2d] ", stage.opcode, stage.rs1, stage.rs2, stage.imm);
  }

  if (
      strcmp(stage.opcode, "LOAD") == 0 ||
      strcmp(stage.opcode, "ADDL") == 0 ||
      strcmp(stage.opcode, "SUBL") == 0 ||
      strcmp(stage.opcode, "JAL") == 0
      ) {
    printf("%5s,R%-2d,R%-2d,#%-2d ", stage.opcode, stage.rd_o, stage.rs1_o, stage.imm);
    printf("\t\t[%5s,U%-2d,U%-2d,#%-2d] ", stage.opcode, stage.rd, stage.rs1, stage.imm);
  }

  if (strcmp(stage.opcode, "MOVC") == 0) {
    printf("%5s,R%-2d,#%-2d%4s", stage.opcode, stage.rd_o, stage.imm, " ");
    printf("\t\t[%5s,U%-2d,#%-2d]%8s", stage.opcode, stage.rd, stage.imm, " ");
  }

  if (strcmp(stage.opcode, "JUMP") == 0) {
    printf("%5s,R%-2d,#%-4d%4s", stage.opcode, stage.rs1_o, stage.imm, " ");
    printf("\t\t[%5s,U%-2d,#%-4d]%6s", stage.opcode, stage.rs1, stage.imm, " ");
  }

  if (strcmp(stage.opcode, "HALT") == 0) {
    printf("%5s\t\t\t%31s", stage.opcode, " ");
  }

  if (
      strcmp(stage.opcode, "BZ") == 0 ||
      strcmp(stage.opcode, "BNZ") == 0
      ) {
    printf("%5s,#%-2d\t\t%27s", stage.opcode, stage.imm, " ");
  }

  if (strcmp(stage.opcode, "NOP") == 0) {
    printf("%5s\t\t\t%31s", stage.opcode, " ");
  }

  if (
      strcmp(stage.opcode, "ADD") == 0 ||
      strcmp(stage.opcode, "SUB") == 0 ||
      strcmp(stage.opcode, "MUL") == 0 ||
      strcmp(stage.opcode, "AND") == 0 ||
      strcmp(stage.opcode, "EX-OR") == 0 ||
      strcmp(stage.opcode, "OR") == 0
      ) {
    printf("%5s,R%-2d,R%-2d,R%-2d ", stage.opcode, stage.rd_o, stage.rs1_o, stage.rs2_o);
    printf("\t\t[%5s,U%-2d,U%-2d,U%-2d] ", stage.opcode, stage.rd, stage.rs1, stage.rs2);
  }
}

void print_rob_entries(APEX_CPU *cpu) {
  printf(" _______________________________________________________________________________\n");
  printf("|\t\t\t\tDetails of ROB State:\t\t\t\t|\n");
  for (int i = 0; i < ROB_SIZE; i++) {
    if (cpu->ROB[i].rd != -1 || cpu->ROB[i].rs1 != -1) {
      printf("|ROB[%d]:\t\t", i);
      print_rob_instruction(cpu->ROB[i]);
      printf("\t|\n");
    }
  }
  printf(" ¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯\n");

}

void print_retired_rob_entries(struct ROBEntry firstRetiredROBEntry, struct ROBEntry secondRetiredROBEntry) {

  printf(" _______________________________________________________________________\n");
  printf("|\t\tDetails of ROB Retired Instructions:\t\t\t|\n");
  if (strcmp(firstRetiredROBEntry.opcode, "") != 0) {
    printf("|\t\t");
    print_rob_instruction(firstRetiredROBEntry);
    printf("\t|\n");
  }
  if (strcmp(secondRetiredROBEntry.opcode, "") != 0) {
    printf("|\t\t");
    print_rob_instruction(secondRetiredROBEntry);
    printf("\t|\n");
  }
  printf(" ¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯\n");

}

struct ROBEntry createDummyROBEntry() {
  struct ROBEntry dummyROBEntry;

  strcpy(dummyROBEntry.opcode, "");
  dummyROBEntry.rs1 = -1;
  dummyROBEntry.rs2 = -1;
  dummyROBEntry.rd = -1;
  dummyROBEntry.imm = -1;

  dummyROBEntry.result = -1;
  dummyROBEntry.isValidsResultStatus = 0;
  dummyROBEntry.cfID = -1;
  return dummyROBEntry;
}

struct ROBEntry createROBEntry(APEX_CPU *cpu) {
  CPU_Stage *stage = &cpu->stage[FQ];
  struct ROBEntry temp;

  strcpy(temp.opcode, stage->opcode);
  temp.rs1 = stage->rs1;
  temp.rs1_o = stage->rs1_o;

  temp.rs2 = stage->rs2;
  temp.rs2_o = stage->rs2_o;

  temp.rd = stage->rd;
  temp.rd_o = stage->rd_o;

  temp.imm = stage->imm;

  temp.pc = stage->pc;
  temp.ar_address = stage->rd;
  temp.excodes = -1;

  temp.isValidsResultStatus = -1;
  if(strcmp(temp.opcode, "HALT") == 0){
    //special handling for HALT.
    temp.isValidsResultStatus = 1;
  }

  temp.result = -1;
  temp.cfID = cpu->recentCFID;

  return temp;
}

int insertInROB(APEX_CPU *cpu) {
  struct ROBEntry robEntry = createROBEntry(cpu);

  if (cpu->robRear < ROB_SIZE) {
    cpu->ROB[cpu->robRear] = robEntry;
    cpu->robRear++;
  }

  if (cpu->robRear == ROB_SIZE) {
    cpu->robFull = 1;
  }
  return 0;
}

void flushRenameTableCopy(APEX_CPU *cpu, int PC) {
  int desiredIndex = -1;
  for (int i = 0; i < 17; i++) {
    if (cpu->renameTableCopy[i].pc == PC) {
      desiredIndex = i;
      break;
    }
  }

  if (desiredIndex > -1) {
    flushRenameTableCopyByIndex(cpu, desiredIndex);
  }
}

struct ROBEntry retireROBEntry(APEX_CPU *cpu) {

  struct ROBEntry retiredROBEntry = cpu->ROB[0];

  //shift all elements by 1 left.
  for (int i = 0; i <= cpu->robRear; i++) {
    cpu->ROB[i] = cpu->ROB[i + 1];
  }

  // flush last entry of ROB
  cpu->ROB[cpu->robRear] = createDummyROBEntry();

  cpu->robRear--;
  cpu->robFull = 0;

  updateCFIOFromROBRetire(cpu, retiredROBEntry);
  updateBackEndRenameTable(cpu, retiredROBEntry);

  if(strcmp(retiredROBEntry.opcode, "HALT") == 0){
    cpu->haltRetiredFromROB = 1;
  }

  int isBranchInstruction = strcmp(retiredROBEntry.opcode, "BNZ") == 0 || strcmp(retiredROBEntry.opcode, "BZ") == 0 ||
                            strcmp(retiredROBEntry.opcode, "JUMP") == 0 || strcmp(retiredROBEntry.opcode, "JAL") == 0;

  if(isBranchInstruction){
    flushRenameTableCopy(cpu, retiredROBEntry.pc);
  }

  return retiredROBEntry;
}

int initializeROB(APEX_CPU *cpu) {

  for (int i = 0; i < ROB_SIZE; i++) {
    cpu->ROB[i] = createDummyROBEntry();
  }
  cpu->robRear = 0;
  cpu->robFull = 0;

  return 0;
}

void deleteROBEntryByIndex(APEX_CPU *cpu, int desiredIndex) {
  //shift all elements by 1 left.
  for (int i = desiredIndex; i <= cpu->robRear; i++) {
    cpu->ROB[i] = cpu->ROB[i + 1];
  }
  // flush last entry of ROB
  cpu->ROB[cpu->robRear] = createDummyROBEntry();
}

int handleROBFlushByCFID(APEX_CPU *cpu, int cfId) {

  while(cpu->ROB[cpu->robRear-1].cfID == cfId && cpu->ROB[cpu->robRear-1].pc != cpu->FUs[0].pc){
    // new condition: do not delete self in this block if you are a branch.
    deleteROBEntryByIndex(cpu, cpu->robRear-1);
    cpu->robRear--;
    cpu->robFull = 0;
  }

  return 0;
}