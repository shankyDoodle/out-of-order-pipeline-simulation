/*
 *  cpu.c
 *  Contains APEX cpu pipeline implementation
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cpu.h"

/* Set this flag to 1 to enable debug messages */
int ENABLE_DEBUG_MESSAGES = 0;


static void print_instruction(CPU_Stage *stage) {
  if (strcmp(stage->opcode, "STORE") == 0) {
    printf("%s,R%d,R%d,#%d ", stage->opcode, stage->rs1, stage->rs2, stage->imm);
  }

  if (
      strcmp(stage->opcode, "LOAD") == 0 ||
      strcmp(stage->opcode, "ADDL") == 0 ||
      strcmp(stage->opcode, "SUBL") == 0 ||
      strcmp(stage->opcode, "JAL") == 0
      ) {
    printf("%s,R%d,R%d,#%d ", stage->opcode, stage->rd, stage->rs1, stage->imm);
  }

  if (strcmp(stage->opcode, "MOVC") == 0) {
    printf("%s,R%d,#%d ", stage->opcode, stage->rd, stage->imm);
  }

  if (strcmp(stage->opcode, "JUMP") == 0) {
    printf("%s,R%d,#%d ", stage->opcode, stage->rs1, stage->imm);
  }

  if (strcmp(stage->opcode, "HALT") == 0) {
    printf("%s", stage->opcode);
  }

  if (
      strcmp(stage->opcode, "BZ") == 0 ||
      strcmp(stage->opcode, "BNZ") == 0
      ) {
    printf("%s,#%d ", stage->opcode, stage->imm);
  }

  if (strcmp(stage->opcode, "NOP") == 0) {
    printf("%s ", stage->opcode);
  }

  if (
      strcmp(stage->opcode, "ADD") == 0 ||
      strcmp(stage->opcode, "SUB") == 0 ||
      strcmp(stage->opcode, "MUL") == 0 ||
      strcmp(stage->opcode, "AND") == 0 ||
      strcmp(stage->opcode, "EX-OR") == 0 ||
      strcmp(stage->opcode, "OR") == 0
      ) {
    printf("%s,R%d,R%d,R%d ", stage->opcode, stage->rd, stage->rs1, stage->rs2);
  }
}

/* Debug function which dumps the cpu stage content DON'T EDIT*/
static void print_stage_content(char *name, CPU_Stage *stage) {
  printf("%-15s: pc(%d) ", name, stage->pc);
  print_instruction(stage);
  printf("\n");
}

static void print_instruction_o(CPU_Stage *stage) {
  if (strcmp(stage->opcode, "STORE") == 0) {
    printf("%s,R%d,R%d,#%d ", stage->opcode, stage->rs1_o, stage->rs2_o, stage->imm);
  }

  if (
      strcmp(stage->opcode, "LOAD") == 0 ||
      strcmp(stage->opcode, "ADDL") == 0 ||
      strcmp(stage->opcode, "SUBL") == 0 ||
      strcmp(stage->opcode, "JAL") == 0
      ) {
    printf("%s,R%d,R%d,#%d ", stage->opcode, stage->rd_o, stage->rs1_o, stage->imm);
  }

  if (strcmp(stage->opcode, "MOVC") == 0) {
    printf("%s,R%d,#%d ", stage->opcode, stage->rd_o, stage->imm);
  }

  if (strcmp(stage->opcode, "JUMP") == 0) {
    printf("%s,R%d,#%d ", stage->opcode, stage->rs1_o, stage->imm);
  }

  if (strcmp(stage->opcode, "HALT") == 0) {
    printf("%s", stage->opcode);
  }

  if (
      strcmp(stage->opcode, "BZ") == 0 ||
      strcmp(stage->opcode, "BNZ") == 0
      ) {
    printf("%s,#%d ", stage->opcode, stage->imm);
  }

  if (strcmp(stage->opcode, "NOP") == 0) {
    printf("%s ", stage->opcode);
  }

  if (
      strcmp(stage->opcode, "ADD") == 0 ||
      strcmp(stage->opcode, "SUB") == 0 ||
      strcmp(stage->opcode, "MUL") == 0 ||
      strcmp(stage->opcode, "AND") == 0 ||
      strcmp(stage->opcode, "EX-OR") == 0 ||
      strcmp(stage->opcode, "OR") == 0
      ) {
    printf("%s,R%d,R%d,R%d ", stage->opcode, stage->rd_o, stage->rs1_o, stage->rs2_o);
  }
}

/* Debug function which dumps the cpu stage content DON'T EDIT*/
static void print_stage_content_o(char *name, CPU_Stage *stage) {
  printf("%-15s: pc(%d) ", name, stage->pc);
  print_instruction_o(stage);
  printf("\n");
}

/** This function creates and initializes APEX cpu.*/
APEX_CPU *APEX_cpu_init(const char *filename) {
  if (!filename) {
    return NULL;
  }

  APEX_CPU *cpu = malloc(sizeof(*cpu));
  if (!cpu) {
    return NULL;
  }

  /* Initialize PC, Registers and all pipeline stages */
  cpu->pc = 4000;
  memset(cpu->regs, 0, sizeof(int) * 32);
  memset(cpu->regs_valid, 1, sizeof(int) * 32);
  memset(cpu->stage, 0, sizeof(CPU_Stage) * NUM_STAGES);
  memset(cpu->data_memory, -1, sizeof(int) * 4000);

  /* Parse input file and create code memory */
  cpu->code_memory = create_code_memory(filename, &cpu->code_memory_size);

  if (!cpu->code_memory) {
    free(cpu);
    return NULL;
  }

  if (ENABLE_DEBUG_MESSAGES) {
    fprintf(stderr,
            "APEX_CPU : Initialized APEX CPU, loaded %d instructions\n",
            cpu->code_memory_size);
    fprintf(stderr, "APEX_CPU : Printing Code Memory\n");
    printf("%-9s %-9s %-9s %-9s %-9s\n", "opcode", "rd", "rs1", "rs2", "imm");

    for (int i = 0; i < cpu->code_memory_size; ++i) {
      printf("%-9s %-9d %-9d %-9d %-9d\n",
             cpu->code_memory[i].opcode,
             cpu->code_memory[i].rd,
             cpu->code_memory[i].rs1,
             cpu->code_memory[i].rs2,
             cpu->code_memory[i].imm);
    }
  }

  /* Make all stages busy except Fetch stage, initally to start the pipeline */
  for (int i = 1; i < NUM_STAGES; ++i) {
    cpu->stage[i].busy = 1;
  }

  cpu->haltEncountered = 0;
  cpu->haltRetiredFromROB = 0;

  /*CFIO initialization*/
  initializeCFIO(cpu);

  /* initialize rename table*/
  initializeRenameTable(cpu);

  /* initialize rename table*/
  initializeBackEndRenameTable(cpu);

  /* initialize URF*/
  initializeURF(cpu);

  /* initialize FUs*/
  initializeFUs(cpu);

  /*IQ data initialization*/
  initializeIssueQueue(cpu);

  /*LSQ initialization*/
  initializeLSQ(cpu);

  /*ROB initialization*/
  initializeROB(cpu);

  /*renameTableCopy initialization*/
  initializeRenameTableCopy(cpu);

  return cpu;
}

/*This function de-allocates APEX cpu.*/
void APEX_cpu_stop(APEX_CPU *cpu) {
  free(cpu->code_memory);
  free(cpu);
}

/* Converts the PC(4000 series) into array index for code memory. DON'T EDIT*/
int get_code_index(int pc) {
  return (pc - 4000) / 4;
}


/*Fetch Stage of APEX Pipeline*/
int fetch(APEX_CPU *cpu) {
  CPU_Stage *stage = &cpu->stage[F];
  if (!stage->busy && !stage->stalled && !cpu->haltEncountered) {
    /* Store current PC in fetch latch */
    stage->pc = cpu->pc;

    /* Index into code memory using this pc and copy all instruction fields into
     * fetch latch
     */
    APEX_Instruction *current_ins = &cpu->code_memory[get_code_index(cpu->pc)];

    /** Accept only valid instructions*/
    if (
        !(strcmp(current_ins->opcode, "") == 0 ||
          strcmp(current_ins->opcode, "ADD") == 0 ||
          strcmp(current_ins->opcode, "ADDL") == 0 ||
          strcmp(current_ins->opcode, "SUB") == 0 ||
          strcmp(current_ins->opcode, "SUBL") == 0 ||
          strcmp(current_ins->opcode, "MUL") == 0 ||
          strcmp(current_ins->opcode, "AND") == 0 ||
          strcmp(current_ins->opcode, "OR") == 0 ||
          strcmp(current_ins->opcode, "EX-OR") == 0 ||
          strcmp(current_ins->opcode, "MOVC") == 0 ||
          strcmp(current_ins->opcode, "STORE") == 0 ||
          strcmp(current_ins->opcode, "LOAD") == 0 ||
          strcmp(current_ins->opcode, "BZ") == 0 ||
          strcmp(current_ins->opcode, "BNZ") == 0 ||
          strcmp(current_ins->opcode, "JUMP") == 0 ||
          strcmp(current_ins->opcode, "JAL") == 0 ||
          strcmp(current_ins->opcode, "HALT") == 0)
        ) {
      strcpy(current_ins->opcode, "");
      current_ins->rs1 = -1;
      current_ins->rs2 = -1;
      current_ins->rd = -1;
      current_ins->imm = -1;
    }

    strcpy(stage->opcode, current_ins->opcode);
    stage->rd = current_ins->rd;
    stage->rd_o = current_ins->rd;

    stage->rs1 = current_ins->rs1;
    stage->rs1_o = current_ins->rs1;

    stage->rs2 = current_ins->rs2;
    stage->rs2_o = current_ins->rs2;

    stage->rd = current_ins->rd;
    stage->rd_o = current_ins->rd;

    stage->imm = current_ins->imm;

    /* Update PC for next instruction */
    cpu->pc += 4;

    /* Copy data from fetch latch to decode latch*/
    if (!stage->stalled) {
      cpu->stage[DRF] = cpu->stage[F];
    }

  }
  if (ENABLE_DEBUG_MESSAGES) {
    print_stage_content("Fetch", stage);
  }
  return 0;
}

/*Decode Stage of APEX Pipeline*/
int decode(APEX_CPU *cpu) {
  CPU_Stage *stage = &cpu->stage[DRF];

  if (!stage->busy && !stage->stalled && !cpu->haltEncountered) {

    if (strcmp(stage->opcode, "HALT") == 0) {
      cpu->haltEncountered = 1;
    }

    /** Register renaming logic and update the rename table */
    if (
        strcmp(stage->opcode, "") != 0 &&
        strcmp(stage->opcode, "HALT") != 0
        ) {
      registerRenaming(cpu);
    } else {
      goto endOfIf;
    }

    /* Read data from unified register file for store */
    if (strcmp(stage->opcode, "STORE") == 0) {
      stage->rs1_value = cpu->unifiedRF[stage->rs1].value;
      stage->rs2_value = cpu->unifiedRF[stage->rs2].value;
    }

    if (
        strcmp(stage->opcode, "LOAD") == 0 ||
        strcmp(stage->opcode, "ADDL") == 0 ||
        strcmp(stage->opcode, "SUBL") == 0
        ) {
      stage->rs1_value = cpu->unifiedRF[stage->rs1].value;
    }

    if (
        strcmp(stage->opcode, "JUMP") == 0 ||
        strcmp(stage->opcode, "JAL") == 0
        ) {
      stage->rs1_value = cpu->unifiedRF[stage->rs1].value;
    }

    /* No Register file read needed for MOVC */
    if (strcmp(stage->opcode, "MOVC") == 0) {
    }

    /* Read data from unified register file for ADD */
    if (
        strcmp(stage->opcode, "ADD") == 0 ||
        strcmp(stage->opcode, "SUB") == 0 ||
        strcmp(stage->opcode, "MUL") == 0 ||
        strcmp(stage->opcode, "AND") == 0 ||
        strcmp(stage->opcode, "OR") == 0 ||
        strcmp(stage->opcode, "EX-OR") == 0
        ) {
      stage->rs1_value = cpu->unifiedRF[stage->rs1].value;
      stage->rs2_value = cpu->unifiedRF[stage->rs2].value;
    }

    if (strcmp(stage->opcode, "JUMP") == 0 ||
        strcmp(stage->opcode, "JAL") == 0 ||
        strcmp(stage->opcode, "BZ") == 0 ||
        strcmp(stage->opcode, "BNZ") == 0) {
      keepCopyOfRenameTable(cpu, stage);
    }

    endOfIf:;

    /* Copy data from decode latch to execute latch*/
    if (!stage->stalled) {
      cpu->stage[FQ] = cpu->stage[DRF];
    }

  }
  if (ENABLE_DEBUG_MESSAGES) {
    print_rename_table_entries(cpu);
    print_stage_content_o("Decode/RF", stage);
  }
  return 0;
}

/*Fill queues stage*/
int fillQueues(APEX_CPU *cpu) {
  CPU_Stage *stage = &cpu->stage[FQ];

  if (!stage->busy && !stage->stalled) {

    if (strcmp(stage->opcode, "") == 0) {
      goto endOfIf;
    }

    int isLoadStoreInstruction = strcmp(stage->opcode, "LOAD") == 0 || strcmp(stage->opcode, "STORE") == 0;
    int isBranchInstruction = strcmp(stage->opcode, "BNZ") == 0 || strcmp(stage->opcode, "BZ") == 0 ||
                              strcmp(stage->opcode, "JUMP") == 0 || strcmp(stage->opcode, "JAL") == 0;

    if (cpu->haltEncountered) {
      //insert entry in ROB
      flushStageByStageId(cpu, F);
      flushStageByStageId(cpu, DRF);
      insertInROB(cpu);
      flushStageByStageId(cpu, FQ);

    } else if (isLoadStoreInstruction && !cpu->lsqFull && !cpu->iqFull && !cpu->robFull) {
      //insert entry in LSQ
      int lsqIndex = insertInLSQ(cpu);

      //insert entry in ROB
      insertInROB(cpu);

      //insert entry in IQ
      insertInIQ(cpu, lsqIndex);

    } else if (!isLoadStoreInstruction && isBranchInstruction && !cpu->cfioFull && !cpu->iqFull && !cpu->robFull) {
      // insert in CFIO
      insertInCFIO(cpu);

      //insert entry in ROB
      insertInROB(cpu);

      //insert entry in IQ
      insertInIQ(cpu, -1);

    } else if (!isLoadStoreInstruction && !isBranchInstruction && !cpu->iqFull && !cpu->robFull) {
      //insert entry in ROB
      insertInROB(cpu);

      //insert entry in IQ
      insertInIQ(cpu, -1);

    } else {
      stage->stalled = 1;
    }

    endOfIf:;

    /* Copy data from decode latch to execute latch*/
    if (!stage->stalled) {
      cpu->stage[EX] = cpu->stage[FQ];
    }

  }
  if (ENABLE_DEBUG_MESSAGES) {
    print_iq_entries(cpu);
    print_rob_entries(cpu);
    print_lsq_entries(cpu);
  }

  return 0;
}

/*Execute Stage of APEX Pipeline*/
int execute(APEX_CPU *cpu) {
  CPU_Stage *stage = &cpu->stage[EX];

  if (!stage->busy && !stage->stalled) {
    memFUExecute(cpu);
    intFUExecute(cpu);
    mulFuExecute(cpu);

    if (!stage->stalled) {
      cpu->stage[RETIRE] = cpu->stage[EX];
    }

  }
  if (ENABLE_DEBUG_MESSAGES) {
    print_all_FU_entries(cpu);
  }

  return 0;
};

/*Retire Stage of Pipeline*/
int retire(APEX_CPU *cpu) {
  CPU_Stage *stage = &cpu->stage[RETIRE];

  /** FLush FUs*/
  if (cpu->flushIntFU) {
    if (cpu->flushBranchTakenData) {
      flushBranchTakenData(cpu, cpu->FUs[0]);
    }

    flushIntFU(cpu);
    cpu->flushIntFU = 0;
  }
  if (cpu->flushMULFU) {
    flushMULFU(cpu);
    cpu->flushMULFU = 0;
  }
  if (cpu->flushMemFU) {
    flushMemFU(cpu);
    cpu->flushMemFU = 0;
  }

  if (strcmp(cpu->FUs[2].opcode, "") == 0) {
    retireStoreROBAsSoonAsItIsReadyForMemFU(cpu);
  }
  struct ROBEntry firstRetiredROBEntry = createDummyROBEntry();
  struct ROBEntry secondRetiredROBEntry = createDummyROBEntry();

  if (!stage->busy && !stage->stalled) {
    int showFirstRetire = 0;
    int showSecondRetire = 0;
    if (cpu->ROB[0].isValidsResultStatus == 1) {
      firstRetiredROBEntry = retireROBEntry(cpu);
      showFirstRetire = 1;

      if (strcmp(cpu->FUs[2].opcode, "") == 0) {
        retireStoreROBAsSoonAsItIsReadyForMemFU(cpu);
      }
      /** As we can remove two instructions from ROB*/
      if (cpu->ROB[0].isValidsResultStatus == 1) {
        showSecondRetire = 1;
        secondRetiredROBEntry = retireROBEntry(cpu);
      }
    }


    if (showFirstRetire) {
      cpu->ins_completed++;
      if (showSecondRetire) {
        cpu->ins_completed++;
      }
    }
  }

  if (ENABLE_DEBUG_MESSAGES) {
    print_retired_rob_entries(firstRetiredROBEntry, secondRetiredROBEntry);
    print_backend_rename_table_entries(cpu);
  }

  return 0;
}

int checkIfMemWILLEmpty(APEX_CPU *cpu) {
  int bAllFUsEmpty = 0;
  if (strcmp(cpu->FUs[2].opcode, "") == 0 || cpu->flushMemFU == 1) {
    bAllFUsEmpty = 1;
  }
  return bAllFUsEmpty;
}

/*APEX CPU simulation loop*/
int APEX_cpu_run(APEX_CPU *cpu, const char* functionality, const char* cycleCount) {

  if(strcmp(functionality, "display") == 0){
    ENABLE_DEBUG_MESSAGES = 1;
  }

  int desiredCycleCount = atoi(cycleCount);
  int brokeDueToCycleCount = 0;
  while (1) {

    /* All the instructions committed, so exit */
    if (((cpu->ins_completed == cpu->code_memory_size || cpu->haltRetiredFromROB) && checkIfMemWILLEmpty(cpu))
        || cpu->clock == 500
        ) {
      printf("\n(apex) >> Simulation Complete");
      break;
    }

    if (ENABLE_DEBUG_MESSAGES) {
      printf("======================================================================================\n");
      printf("\t\t\t\tClock Cycle #: %d\n", cpu->clock + 1);
      printf("======================================================================================\n");
    }

    retire(cpu);
    execute(cpu);
    fillQueues(cpu);
    decode(cpu);
    fetch(cpu);
    cpu->clock++;

    if(desiredCycleCount == cpu->clock){
      brokeDueToCycleCount = 1;
      break;
    }
  }

  printf("\n\n =========== STATE OF UNIFIED REGISTER FILE ============================ \n");
  for (int i = 0; i < 40; i++) {
    if (cpu->unifiedRF[i].value != -1) {
      char *validStr = "Valid";
      if (cpu->unifiedRF[i].destLock == 1) { //check i!=0
        validStr = "Invalid";
      }
      printf("|\tREG[%d]\t|\tValue = %-5d\t|\tStatus = %-8s\t|\n", i, cpu->unifiedRF[i].value, validStr);
    }
  }
  printf(" ¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯\n");


  printf("\n ====== STATE OF DATA MEMORY ===========\n");
  for (int i = 0; i < 100; i++) {
    if (cpu->data_memory[i] != -1) {
      printf("|\tMEM[%d]\t|\tValue = %d\t|\n", i, cpu->data_memory[i]);
    }
  }
  printf(" ¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯\n");

  return brokeDueToCycleCount;
}