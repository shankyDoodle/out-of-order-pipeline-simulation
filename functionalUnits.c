//
// Created by Shashank Kaldate on 12/8/18.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cpu.h"

#define IQ_SIZE 16
#define LSQ_SIZE 20
#define ROB_SIZE 32

void print_fu_stage(struct functionalUnits stage) {
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
    printf("%5s,R%-2d,#%-4d%2s", stage.opcode, stage.rs1_o, stage.imm, " ");
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

  if (strcmp(stage.opcode, "") == 0) {
    printf("\tEmpty\t%32s", " ");
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

void print_all_FU_entries(APEX_CPU *cpu) {
  printf(" _______________________________________________________________________________\n");
  printf("|\t\t\t\tDetails of All FUs:  \t\t\t\t|\n");
  printf("|MEM FU:\t\t");
  print_fu_stage(cpu->FUs[2]);
  if (strcmp(cpu->FUs[2].opcode, "") != 0) {
    int counter = cpu->memCycleCounter == 0 ? 3 : cpu->memCycleCounter;
    printf(" (%d)|\n", counter);
  } else {
    printf("\t|\n");
  }

  printf("|MUL FU:\t\t");
  print_fu_stage(cpu->FUs[1]);
  if (strcmp(cpu->FUs[1].opcode, "") != 0) {
    int mulCounter = cpu->justFilledMUL == 1 ? 1 : 2;
    printf(" (%d)|\n", mulCounter);
  } else {
    printf("\t|\n");
  }

  printf("|INT FU:\t\t");
  print_fu_stage(cpu->FUs[0]);
  printf("\t|\n");
  printf(" ¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯\n");
}

void updateURF(APEX_CPU *cpu, struct functionalUnits stage) {
  cpu->unifiedRF[stage.rd].value = stage.buffer;
  cpu->unifiedRF[stage.rd].destLock = 0;
};

void updateIQ(APEX_CPU *cpu, struct functionalUnits stage) {
  for (int i = 0; i < IQ_SIZE; i++) {
    if (cpu->IssueQueue[i].rs1 == stage.rd) {
      cpu->IssueQueue[i].rs1_value = stage.buffer;
      cpu->IssueQueue[i].rs1_ready = 1;
    }

    if (cpu->IssueQueue[i].rs2 == stage.rd) {
      cpu->IssueQueue[i].rs2_value = stage.buffer;
      cpu->IssueQueue[i].rs2_ready = 1;
    }
  }
};

void updateFQStage(APEX_CPU *cpu, struct functionalUnits FUStage) {
  CPU_Stage *stage = &cpu->stage[FQ];
  if (stage->rs1 == FUStage.rd) {
    stage->rs1_value = FUStage.buffer;
  }

  if (stage->rs2 == FUStage.rd) {
    stage->rs2_value = FUStage.buffer;
  }
}

void updateIntFUForStoreSrc1(APEX_CPU *cpu, struct functionalUnits stage) {
  if (strcmp(cpu->FUs[0].opcode, "STORE") == 0 && cpu->FUs[0].rs1 == stage.rd) {
    cpu->FUs[0].rs1_value = stage.buffer;
  }
}

void updateROB(APEX_CPU *cpu, struct functionalUnits stage) {
  for (int i = 0; i < ROB_SIZE; i++) {
    if (
        cpu->ROB[i].pc == stage.pc &&
        cpu->ROB[i].rd == stage.rd &&
        cpu->ROB[i].rs1 == stage.rs1 &&
        cpu->ROB[i].rs2 == stage.rs2
        ) {
      //same instruction is found in ROB.
      cpu->ROB[i].result = stage.buffer;
      cpu->ROB[i].isValidsResultStatus = 1;
      break;
    }
  }
};

/** Special Handling for STORE instruction*/
int retireStoreROBAsSoonAsItIsReadyForMemFU(APEX_CPU *cpu) {
  if (
      strcmp(cpu->ROB[0].opcode, "STORE") == 0 &&
      strcmp(cpu->LSQ[0].opcode, "STORE") == 0 &&
      cpu->LSQ[0].pc == cpu->ROB[0].pc && cpu->LSQ[0].mem_address > -1
      ) {
    cpu->memFetchStore = 1;
    cpu->ROB[0].result = -1;
    cpu->ROB[0].isValidsResultStatus = 1;
  }
  return 0;
}

void updateLSQ(APEX_CPU *cpu, struct functionalUnits stage) {
  int desiredLSQIndex = -1;
  for (int i = 0; i < LSQ_SIZE; i++) {
    if (stage.pc == cpu->LSQ[i].pc) {
      desiredLSQIndex = i;
      break;
    }
  }

  cpu->LSQ[desiredLSQIndex].mem_address = stage.mem_address;
  if (strcmp(cpu->LSQ[desiredLSQIndex].opcode, "LOAD") == 0 ||
      (strcmp(cpu->LSQ[desiredLSQIndex].opcode, "STORE") == 0 && cpu->LSQ[desiredLSQIndex].rs1_ready)) {
    cpu->LSQ[desiredLSQIndex].readyToDispatch = 1;
  }
}

void updateLSQForStoreSrc1(APEX_CPU *cpu, struct functionalUnits stage) {
  for (int i = 0; i < LSQ_SIZE; i++) {
    if (strcmp(cpu->LSQ[i].opcode, "STORE") == 0 && cpu->LSQ[i].rs1 == stage.rd) {
      cpu->LSQ[i].rs1_value = stage.buffer;
      cpu->LSQ[i].rs1_ready = 1;
      if (cpu->LSQ[i].mem_address > -1) {
        cpu->LSQ[i].readyToDispatch = 1;
      }
    }
  }
}

void updateZFlag(APEX_CPU *cpu, struct functionalUnits stage) {
  if (
      strcmp(stage.opcode, "ADD") == 0 ||
      strcmp(stage.opcode, "ADDL") == 0 ||
      strcmp(stage.opcode, "SUB") == 0 ||
      strcmp(stage.opcode, "SUBL") == 0 ||
      strcmp(stage.opcode, "MUL") == 0
      ) {
    cpu->zFlag = stage.buffer == 0 ? 0 : 1;
  }
}

void flushStageByStageId(APEX_CPU *cpu, enum StagesEnum stageName) {
  CPU_Stage *stage = &cpu->stage[stageName];

  /*reset all stage related data*/
  strcpy(stage->opcode, "NOP");
  stage->rd = -1;
  stage->rd_o = -1;

  stage->rs1 = -1;
  stage->rs1_value = -1;
  stage->rs1_o = -1;

  stage->rs2 = -1;
  stage->rs2_value = -1;
  stage->rs2_o = -1;

  stage->imm = -1;
  stage->pc = 999;
  stage->buffer = -1;
  stage->mem_address = -1;
  stage->busy = -1;
  stage->stalled = -1;
}

int flushBranchTakenData(APEX_CPU *cpu, struct functionalUnits stage) {
  restoreRATDataState(cpu, stage);
  flushAllQuesDueToBranching(cpu, stage);
  flushStageByStageId(cpu, FQ);
  flushStageByStageId(cpu, DRF);

  int instrSkipped = (cpu->newPCBranchTaken - stage.pc) / 4 - 1;
  cpu->ins_completed += instrSkipped;

  cpu->pc = cpu->newPCBranchTaken;

  cpu->flushBranchTakenData = 0;
  cpu->newPCBranchTaken = -1;
  return 0;
}

int handleBranchInstructionInIntFU(APEX_CPU *cpu, struct functionalUnits stage) {
  int takeBranch = 0;
  int newPC = -1;
  int bufferValue = -1;

  if (strcmp(stage.opcode, "BNZ") == 0 && cpu->zFlag == 1) {
    takeBranch = 1;
    newPC = stage.imm;
  } else if (strcmp(stage.opcode, "BZ") == 0 && cpu->zFlag == 0) {
    takeBranch = 1;
    newPC = stage.imm;
  } else if (strcmp(stage.opcode, "JUMP") == 0) {
    takeBranch = 1;
    newPC = stage.rs1_value + stage.imm;
  } else if (strcmp(stage.opcode, "JAL") == 0) {
    takeBranch = 1;
    newPC = stage.rs1_value + stage.imm;
    stage.buffer = stage.pc + 4;
    cpu->FUs[0].buffer = stage.buffer;
    bufferValue = stage.buffer;
  }

  //updateROBStatus
  for (int i = 0; i < 32; i++) {
    if (cpu->ROB[i].pc == stage.pc) {
      cpu->ROB[i].isValidsResultStatus = 1;
    }
  }

  if (takeBranch) {
    cpu->flushBranchTakenData = 1;
    cpu->newPCBranchTaken = newPC;

  }

  return bufferValue;
};

void flushFUByIndex(APEX_CPU *cpu, int index) {
  cpu->FUs[index].rs1 = -1;
  cpu->FUs[index].rs1_value = -1;
  cpu->FUs[index].rs1_o = -1;

  cpu->FUs[index].rs2 = -1;
  cpu->FUs[index].rs2_value = -1;
  cpu->FUs[index].rs2_o = -1;

  strcpy(cpu->FUs[index].opcode, "");

  cpu->FUs[index].rd = -1;
  cpu->FUs[index].rd_o = -1;
  cpu->FUs[index].imm = -1;

  cpu->FUs[index].lsqIndex = -1;
  cpu->FUs[index].pc = -1;
  cpu->FUs[index].buffer = -1;
  cpu->FUs[index].mem_address = -1;

}

void flushIntFU(APEX_CPU *cpu) {
  flushFUByIndex(cpu, 0);
}

void flushMULFU(APEX_CPU *cpu) {
  flushFUByIndex(cpu, 1);
}

void flushMemFU(APEX_CPU *cpu) {
  flushFUByIndex(cpu, 2);
}

int intFUExecute(APEX_CPU *cpu) {

  struct functionalUnits stage = fetchInstructionFromIQForIntFU(cpu);
  if (strcmp(stage.opcode, "") == 0) {
    // No any instruction is in FU
    return 0;
  }

  int isBranchInstruction = strcmp(stage.opcode, "BNZ") == 0 || strcmp(stage.opcode, "BZ") == 0 ||
                            strcmp(stage.opcode, "JUMP") == 0 || strcmp(stage.opcode, "JAL") == 0;
  if (isBranchInstruction) {
    int newBufferValue = handleBranchInstructionInIntFU(cpu, stage);
    stage.buffer = newBufferValue;
    updateURF(cpu, stage);
    updateIQ(cpu, stage);
    updateFQStage(cpu, stage);
  } else {
    /* Store */
    if (strcmp(stage.opcode, "STORE") == 0) {
      //calculate (src2 + literal)
      stage.mem_address = stage.rs2_value + stage.imm;
    }

      /* LOAD */
    else if (strcmp(stage.opcode, "LOAD") == 0) {
      //calculate (src1 + literal)
      stage.mem_address = stage.rs1_value + stage.imm;
    }

      /*No Reg op req for MOVC*/
    else if (strcmp(stage.opcode, "MOVC") == 0) {
      stage.buffer = stage.imm;
    }

      /* ADD */
    else if (strcmp(stage.opcode, "ADD") == 0) {
      stage.buffer = stage.rs1_value + stage.rs2_value;
    }

      /* ADDL */
    else if (strcmp(stage.opcode, "ADDL") == 0) {
      stage.buffer = stage.rs1_value + stage.imm;
    }

      /* SUB */
    else if (strcmp(stage.opcode, "SUB") == 0) {
      stage.buffer = stage.rs1_value - stage.rs2_value;
    }

      /* SUBL */
    else if (strcmp(stage.opcode, "SUBL") == 0) {
      stage.buffer = stage.rs1_value - stage.imm;
    }

      /* AND */
    else if (strcmp(stage.opcode, "AND") == 0) {
      stage.buffer = stage.rs1_value & stage.rs2_value;
    }

      /* OR */
    else if (strcmp(stage.opcode, "OR") == 0) {
      stage.buffer = stage.rs1_value | stage.rs2_value;
    }

      /* EX-OR */
    else if (strcmp(stage.opcode, "EX-OR") == 0) {
      stage.buffer = stage.rs1_value ^ stage.rs2_value;
    }

    //now send this value to all locations.
    updateZFlag(cpu, stage);
    updateURF(cpu, stage);
    updateIQ(cpu, stage);
    updateLSQForStoreSrc1(cpu, stage);
    updateFQStage(cpu, stage);
    if (strcmp(stage.opcode, "STORE") == 0 || strcmp(stage.opcode, "LOAD") == 0) {
      updateLSQ(cpu, stage);
    } else {
      updateROB(cpu, stage);
    }
  }

  cpu->flushIntFU = 1;

  return 0;
}

int mulFuExecute(APEX_CPU *cpu) {
  if (cpu->justFilledMUL) {
    //wait for one cycle.
    //now send this value to all locations.
    updateZFlag(cpu, cpu->FUs[1]);
    updateURF(cpu, cpu->FUs[1]);
    updateIQ(cpu, cpu->FUs[1]);
    updateIntFUForStoreSrc1(cpu, cpu->FUs[1]);
    updateROB(cpu, cpu->FUs[1]);
    updateLSQForStoreSrc1(cpu, cpu->FUs[1]);
    updateFQStage(cpu, cpu->FUs[1]);
    cpu->flushMULFU = 1;
    cpu->justFilledMUL = 0;
    return 0;
  }

  struct functionalUnits stage = fetchInstructionFromIQForMULFU(cpu);
  if (strcmp(stage.opcode, "") == 0) {
    // No any instruction is in FU
    return 0;
  }

  /* MUL */
  if (strcmp(stage.opcode, "MUL") == 0) {
    stage.buffer = stage.rs1_value * stage.rs2_value;
    cpu->FUs[1].buffer = stage.buffer;
  }
  cpu->justFilledMUL = 1;
  return 0;
}

int memFUExecute(APEX_CPU *cpu) {
  if (cpu->memCycleCounter == 2) {
    cpu->memCycleCounter = 0;
    cpu->flushMemFU = 1;
    if (strcmp(cpu->FUs[2].opcode, "LOAD") == 0) {
      updateURF(cpu, cpu->FUs[2]);
      updateIQ(cpu, cpu->FUs[2]);
      updateIntFUForStoreSrc1(cpu, cpu->FUs[2]);
      updateLSQForStoreSrc1(cpu, cpu->FUs[2]);
      updateROB(cpu, cpu->FUs[2]);
    }
    return 0;
  } else if (cpu->memCycleCounter != 0) {
    cpu->memCycleCounter++;
    return 0;
  }

  if (cpu->memCycleCounter == 0) {
    if (cpu->LSQ[0].readyToDispatch != 1) {
      return 0;
    }

    if (strcmp(cpu->LSQ[0].opcode, "STORE") == 0 && cpu->memFetchStore != 1) {
      //if LSQ fetch is STORE then this store should also be present at ROB head. If not then return.
      return 0;
    }
    cpu->flushMemFU = 0;
    struct functionalUnits stage = fetchInstructionFromLSQForMemFU(cpu);

    /* Store */
    if (strcmp(stage.opcode, "STORE") == 0) {
      cpu->data_memory[stage.mem_address] = stage.rs1_value;
      stage.buffer = -1;
      cpu->FUs[2].buffer = -1;

      updateURF(cpu, stage);
      updateIQ(cpu, stage);
      updateIntFUForStoreSrc1(cpu, stage);
      updateLSQForStoreSrc1(cpu, stage);
      updateROB(cpu, stage);
      cpu->memFetchStore = 0;
    }

    /* Load */
    if (strcmp(stage.opcode, "LOAD") == 0) {
      cpu->FUs[2].buffer = cpu->data_memory[stage.mem_address];
      stage.buffer = cpu->data_memory[stage.mem_address];
    }


    cpu->memCycleCounter = 1;
  }


  return 0;
}

int initializeFUs(APEX_CPU *cpu) {
  for (int i = 0; i < 3; i++) {
    flushFUByIndex(cpu, i);
  }

  return 0;
}