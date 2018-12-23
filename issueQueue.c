//
// Created by Shashank Kaldate on 11/30/18.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cpu.h"

#define IQ_SIZE 16

void print_iq_instruction(struct IssueQueueEntry stage) {
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
    printf("%5s,R%-2d,#%-2d%4s", stage.opcode, stage.rs1_o, stage.imm, " ");
    printf("\t\t[%5s,U%-2d,#%-4d]%6s", stage.opcode, stage.rs1, stage.imm, " ");
  }

  if (strcmp(stage.opcode, "HALT") == 0) {
    printf("%5s\t\t%31s", stage.opcode, " ");
  }

  if (
      strcmp(stage.opcode, "BZ") == 0 ||
      strcmp(stage.opcode, "BNZ") == 0
      ) {
    printf("%5s,#%-2d\t\t%27s", stage.opcode, stage.imm, " ");
  }

  if (strcmp(stage.opcode, "NOP") == 0) {
    printf("%5s\t\t%31s", stage.opcode, " ");
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

void print_iq_entries(APEX_CPU *cpu) {
  printf(" _______________________________________________________________________________\n");
  printf("|\t\t\t\tDetails of  IQ State:\t\t\t\t|\n");
  for (int i = 0; i < IQ_SIZE; i++) {
    if (!cpu->IssueQueue[i].isAvailableStatusBit) {
      printf("| IQ[%d]:\t\t", i);
      print_iq_instruction(cpu->IssueQueue[i]);
      printf("\t|\n");
    }
  }
  printf(" ¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯\n");

}

int isForIntFU(char *opcode) {
  if (
      strcmp(opcode, "MUL") == 0 ||
      strcmp(opcode, "HALT") == 0
      ) {
    return 0;
  } else {
    return 1;
  }
}

int isForMULFU(char *opcode) {
  return (strcmp(opcode, "MUL") == 0) ? 1 : 0;
}

int initializeIssueQueue(APEX_CPU *cpu) {

  for (int i = 0; i < IQ_SIZE; i++) {
    cpu->IssueQueue[i].isAvailableStatusBit = 1;
    cpu->IssueQueue[i].rs1 = -1;
    cpu->IssueQueue[i].rs1_value = -1;
    cpu->IssueQueue[i].rs1_ready = -1;
    cpu->IssueQueue[i].rs1_o = -1;

    cpu->IssueQueue[i].rs2 = -1;
    cpu->IssueQueue[i].rs2_value = -1;
    cpu->IssueQueue[i].rs2_ready = -1;
    cpu->IssueQueue[i].rs2_o = -1;

    cpu->IssueQueue[i].rd = -1;
    cpu->IssueQueue[i].rd_o = -1;

    cpu->IssueQueue[i].imm = -1;
    cpu->IssueQueue[i].pc = 9999;
    cpu->IssueQueue[i].lsqIndex = -1;
    cpu->IssueQueue[i].dummyEntry = 0;
    cpu->IssueQueue[i].cfID = -1;
  }
  cpu->iqFull = 0;

  return 0;
}

/**
 *
 * @param cpu
 * @param indexIQ
 * @param indexFU : index 0: INT, 1: MUL, 2: Memory
 */
void fillUpFUFromIQ(APEX_CPU *cpu, int indexIQ, int indexFU) {
  cpu->FUs[indexFU].rs1 = cpu->IssueQueue[indexIQ].rs1;
  cpu->FUs[indexFU].rs1_o = cpu->IssueQueue[indexIQ].rs1_o;
  cpu->FUs[indexFU].rs1_value = cpu->IssueQueue[indexIQ].rs1_value;

  cpu->FUs[indexFU].rs2 = cpu->IssueQueue[indexIQ].rs2;
  cpu->FUs[indexFU].rs2_o = cpu->IssueQueue[indexIQ].rs2_o;
  cpu->FUs[indexFU].rs2_value = cpu->IssueQueue[indexIQ].rs2_value;

  strcpy(cpu->FUs[indexFU].opcode, cpu->IssueQueue[indexIQ].opcode);

  cpu->FUs[indexFU].rd = cpu->IssueQueue[indexIQ].rd;
  cpu->FUs[indexFU].rd_o = cpu->IssueQueue[indexIQ].rd_o;
  cpu->FUs[indexFU].imm = cpu->IssueQueue[indexIQ].imm;

  cpu->FUs[indexFU].lsqIndex = cpu->IssueQueue[indexIQ].lsqIndex;
  cpu->FUs[indexFU].pc = cpu->IssueQueue[indexIQ].pc;
  cpu->FUs[indexFU].cfID = cpu->IssueQueue[indexIQ].cfID;
};

void flushIQEntry(APEX_CPU *cpu, int indexIQ) {
  strcpy(cpu->IssueQueue[indexIQ].opcode, "");
  cpu->IssueQueue[indexIQ].rs1 = -1;
  cpu->IssueQueue[indexIQ].rs1_value = -1;
  cpu->IssueQueue[indexIQ].rs1_ready = -1;
  cpu->IssueQueue[indexIQ].rs2 = -1;
  cpu->IssueQueue[indexIQ].rs2_value = -1;
  cpu->IssueQueue[indexIQ].rs2_ready = -1;

  cpu->IssueQueue[indexIQ].rd = -1;
  cpu->IssueQueue[indexIQ].imm = -1;
  cpu->IssueQueue[indexIQ].pc = 9999;
  cpu->IssueQueue[indexIQ].lsqIndex = -1;
  cpu->IssueQueue[indexIQ].isAvailableStatusBit = 1;
  cpu->IssueQueue[indexIQ].dummyEntry = 0;
  cpu->IssueQueue[indexIQ].cfID = -1;

};

struct IssueQueueEntry createIQEntryForInstruction(APEX_CPU *cpu, int lsqIndex) {
  CPU_Stage *stage = &cpu->stage[FQ];

  struct IssueQueueEntry temp;
  temp.isAvailableStatusBit = 0;

  temp.imm = stage->imm;
  temp.rd = stage->rd;
  temp.rd_o = stage->rd_o;

  temp.rs1 = stage->rs1;
  temp.rs1_o = stage->rs1_o;
  if (stage->rs1 != -1) {
    temp.rs1_value = stage->rs1_value;
    temp.rs1_ready = !cpu->unifiedRF[stage->rs1].destLock;
  } else {
    temp.rs1_value = -1;
    temp.rs1_ready = 1;
  }

  temp.rs2 = stage->rs2;
  temp.rs2_o = stage->rs2_o;
  if (stage->rs2 != -1) {
    temp.rs2_value = stage->rs2_value;
    temp.rs2_ready = !cpu->unifiedRF[stage->rs2].destLock;
  } else {
    temp.rs2_value = -1;
    temp.rs2_ready = 1;
  }

  strcpy(temp.opcode, stage->opcode);

  temp.lsqIndex = lsqIndex;
  temp.pc = stage->pc;
  temp.cfID = cpu->recentCFID;

  return temp;
}

int insertInIQ(APEX_CPU *cpu, int lsqIndex) {
  struct IssueQueueEntry iqEntry = createIQEntryForInstruction(cpu, lsqIndex);

  for (int i = 0; i < IQ_SIZE; i++) {
    if (cpu->IssueQueue[i].isAvailableStatusBit) {
      cpu->IssueQueue[i] = iqEntry;
      break;
    }
  }

  /*Update IQ full status bit*/
  int iqFullFLag = 1;
  for (int i = 0; i < IQ_SIZE; i++) {
    if (cpu->IssueQueue[i].isAvailableStatusBit) {
      iqFullFLag = 0;
      break;
    }
  }
  cpu->iqFull = iqFullFLag;

  return 1;
}

void deleteIQEntryByIndex(APEX_CPU *cpu, int desiredIndex) {
  //shift all elements by 1 left.
  for (int i = desiredIndex; i < IQ_SIZE; i++) {
    cpu->IssueQueue[i] = cpu->IssueQueue[i + 1];
  }
  flushIQEntry(cpu, IQ_SIZE - 1);
}

struct functionalUnits fetchInstructionFromIQForIntFU(APEX_CPU *cpu) {
  int desiredIndex = -1;
  int minPC = 9999;
  for (int i = 0; i < IQ_SIZE; i++) {
    if (strcmp(cpu->IssueQueue[i].opcode, "STORE") == 0) {
      // No need of checking SRC1 ready bit in case of STORE instruction.
      if (
          isForIntFU(cpu->IssueQueue[i].opcode) &&
          !cpu->IssueQueue[i].isAvailableStatusBit &&
          cpu->IssueQueue[i].rs2_ready &&
          cpu->IssueQueue[i].pc < minPC
          ) {
        desiredIndex = i;
        minPC = cpu->IssueQueue[i].pc;
      }
    } else {
      if (
          isForIntFU(cpu->IssueQueue[i].opcode) &&
          !cpu->IssueQueue[i].isAvailableStatusBit &&
          cpu->IssueQueue[i].rs1_ready && cpu->IssueQueue[i].rs2_ready &&
          cpu->IssueQueue[i].pc < minPC
          ) {
        desiredIndex = i;
        minPC = cpu->IssueQueue[i].pc;
      }
    }

  }

  if (desiredIndex != -1) {
    int isZFlagBranchInstruction = strcmp(cpu->IssueQueue[desiredIndex].opcode, "BNZ") == 0 ||
                              strcmp(cpu->IssueQueue[desiredIndex].opcode, "BZ") == 0 ;
    if (isZFlagBranchInstruction && strcmp(cpu->FUs[1].opcode, "MUL") == 0) {
      // do nothing.
    } else {
      cpu->iqFull = 0;
      fillUpFUFromIQ(cpu, desiredIndex, 0);
      deleteIQEntryByIndex(cpu, desiredIndex);
    }
  }

  return cpu->FUs[0];
}

struct functionalUnits fetchInstructionFromIQForMULFU(APEX_CPU *cpu) {
  int desiredIndex = -1;
  int minPC = 9999;
  for (int i = 0; i < IQ_SIZE; i++) {
    if (
        isForMULFU(cpu->IssueQueue[i].opcode) &&
        !cpu->IssueQueue[i].isAvailableStatusBit &&
        cpu->IssueQueue[i].rs1_ready && cpu->IssueQueue[i].rs2_ready &&
        cpu->IssueQueue[i].pc < minPC
        ) {
      desiredIndex = i;
      minPC = cpu->IssueQueue[i].pc;
    }
  }

  if (desiredIndex != -1) {
    cpu->iqFull = 0;
    fillUpFUFromIQ(cpu, desiredIndex, 1);

    //shift all elements by 1 left.
    for (int i = desiredIndex; i < IQ_SIZE; i++) {
      cpu->IssueQueue[i] = cpu->IssueQueue[i + 1];
    }
    flushIQEntry(cpu, IQ_SIZE - 1);
  }

  return cpu->FUs[1];
}

int handleIQFlushByCFID(APEX_CPU *cpu, int cfId) {
  for (int i = 0; i < IQ_SIZE; i++) {
    if (cpu->IssueQueue[i].cfID == cfId) {
      deleteIQEntryByIndex(cpu, i);
      i--;
    }
  }

  return 0;
}