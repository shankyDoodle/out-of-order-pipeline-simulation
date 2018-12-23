#ifndef _APEX_CPU_H_
#define _APEX_CPU_H_
/**
 *  cpu.h
 *  Contains various CPU and Pipeline Data structures
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum StagesEnum{
    F,
    DRF,
    FQ, //fill queues
    EX,
    RETIRE,
    NUM_STAGES
};

/* Format of an APEX instruction  */
typedef struct APEX_Instruction {
    char opcode[128];  // Operation Code
    int rd;        // Destination Register Address
    int rs1;        // Source-1 Register Address
    int rs2;        // Source-2 Register Address
    int imm;        // Literal Value
} APEX_Instruction;

/* Model of CPU stage latch */
typedef struct CPU_Stage {
    int pc;        // Program Counter
    char opcode[128];  // Operation Code
    int rs1;        // Source-1 Register Address
    int rs2;        // Source-2 Register Address
    int rd;        // Destination Register Address
    int imm;        // Literal Value
    int rs1_value;  // Source-1 Register Value
    int rs2_value;  // Source-2 Register Value
    int buffer;    // Latch to hold some value
    int mem_address;  // Computed Memory Address
    int busy;        // Flag to indicate, stage is performing some action
    int stalled;    // Flag to indicate, stage is stalled


    int rs1_o; //original reg values
    int rs2_o;
    int rd_o;
} CPU_Stage;

struct IssueQueueEntry {
    char opcode[128];  // Operation Code

    int rs1;
    int rs1_ready;
    int rs1_value;

    int rs2;
    int rs2_ready;
    int rs2_value;

    int rd;
    int imm;
    int lsqIndex;
    int pc; // Program Counter
    int isAvailableStatusBit;  //indicates if this IQ entry is allocated or free
    int dummyEntry;
    int cfID;

    int rs1_o; //original reg values
    int rs2_o;
    int rd_o;
} IssueQueueEntry;


struct ROBEntry {
    int pc;
    int ar_address;
    int result;
    int excodes;
    int isValidsResultStatus;
    int cfID;

    int rs1;
    int rs2;
    int rd;
    char opcode[128];  // Operation Code
    int imm;

    int rs1_o; //original reg values
    int rs2_o;
    int rd_o;
} ROBEntry;

struct LSQEntry {
    int readyToDispatch;
    int mem_address;
    int pc; // at which clock cycle instruction entered
    int cfID;

    char opcode[128];  // Operation Code
    int rs1;
    int rs1_value;// needed for store
    int rs1_ready;// needed for store

    int rs2;
    int rd; //for load. default it should be -1. do take care of this.
    int imm;

    int rs1_o; //original reg values
    int rs2_o;
    int rd_o;
} LSQEntry;

struct functionalUnits {
    char opcode[128];  // Operation Code

    int rs1;
    int rs1_value;

    int rs2;
    int rs2_value;

    int buffer; //store the calculated value.
    int mem_address; //store the calculated value.

    int rd;
    int imm;
    int lsqIndex;
    int pc;
    int cfID;

    int dummyEntry;

    int rs1_o; //original reg values
    int rs2_o;
    int rd_o;
};

struct renameTable {
    int value;
    int zFlag;
};

struct backEndRenameTable {
    int value;
    int zFlag;
};

struct URFEntry {
    int value;
    int isAvailable;
    int destLock;
};

struct CFIOEntry { //control flow instruction order.
    char opcode[128];  // Operation Code

    int rs1;
    int rs2;
    int rd;
    int imm;

    int pc; // Program Counter
    int isAvailable;
    int cfID;
};

struct copyOfRenameTableEntriesEntry{
    struct renameTable renameTable_o[16];
    int pc;
};

/* Model of APEX CPU */
typedef struct APEX_CPU {
    /* Clock cycles elapsed */
    int clock;

    /* Current program counter */
    int pc;

    /* Integer register file */
    int regs[32];
    int regs_valid[32];

    /* Array of 5 CPU_stage */
    CPU_Stage stage[5];

    /* Code Memory where instructions are stored */
    APEX_Instruction *code_memory;
    int code_memory_size;

    /* Data Memory */
    int data_memory[4096];

    /* Some stats */
    int ins_completed;

    /*Rename Table*/
    struct renameTable renameTable[16];

    /*R-RAT Table*/
    struct backEndRenameTable backEndRenameTable[16];

    /*URF*/
    struct URFEntry unifiedRF[40];

    /* IQ data*/
    struct IssueQueueEntry IssueQueue[16];
    int iqFull;

    /* IQ data*/
    struct LSQEntry LSQ[20];
    int lsqFull;
    int lsqRear;

    /* ROB data*/
    struct ROBEntry ROB[32];
    int robRear;
    int robFull;

    struct functionalUnits FUs[3]; // 3 FUS-> Int, MUL, Memory.
    int justFilledMUL; //to check stalling for MUL FU.
    int flushIntFU;
    int flushMULFU;
    int flushMemFU;
    int memCycleCounter;

    struct CFIOEntry CFIO[8];
    int cfioRear;
    int cfioFull;
    int recentCFID;

    int zFlag;

    int memFetchStore;
    int flushBranchTakenData;
    int newPCBranchTaken;

    int haltEncountered;
    int haltRetiredFromROB;

    struct copyOfRenameTableEntriesEntry renameTableCopy[16];

} APEX_CPU;

/**************************** Generic functionality Start **************************************/
APEX_Instruction *create_code_memory(const char *filename, int *size);

APEX_CPU *APEX_cpu_init(const char *filename);

int APEX_cpu_run(APEX_CPU *cpu, const char* functionality, const char* cycleCount);

void APEX_cpu_stop(APEX_CPU *cpu);

int fetch(APEX_CPU *cpu);

int decode(APEX_CPU *cpu);

int execute(APEX_CPU *cpu);

/***************************************** RenameTable functions *********************************************/
int initializeRenameTable(APEX_CPU *cpu);

int initializeURF(APEX_CPU *cpu);

int registerRenaming(APEX_CPU *cpu);

void print_rename_table_entries(APEX_CPU *cpu);

void restoreRATDataState(APEX_CPU *cpu, struct functionalUnits stage);

void initializeRenameTableCopy(APEX_CPU *cpu);

void keepCopyOfRenameTable(APEX_CPU *cpu, CPU_Stage *stage);

void flushRenameTableCopyByIndex(APEX_CPU *cpu, int index);

/***************************************** Backend RenameTable functions *********************************************/
int initializeBackEndRenameTable(APEX_CPU *cpu);

void updateBackEndRenameTable(APEX_CPU *cpu, struct ROBEntry retiredROBEntry);

void print_backend_rename_table_entries(APEX_CPU *cpu);

/********************************************** IQ Functions **************************************************/
int initializeIssueQueue(APEX_CPU *cpu);

int insertInIQ(APEX_CPU *cpu, int lsqIndex);

struct functionalUnits fetchInstructionFromIQForIntFU(APEX_CPU *cpu);

struct functionalUnits fetchInstructionFromIQForMULFU(APEX_CPU *cpu);

void print_iq_entries(APEX_CPU *cpu);

int handleIQFlushByCFID(APEX_CPU *cpu, int cfId);

/*********************************************** LSQ Functions *************************************************/
int initializeLSQ(APEX_CPU *cpu);

int insertInLSQ(APEX_CPU *cpu);

struct functionalUnits fetchInstructionFromLSQForMemFU(APEX_CPU *cpu);

void print_lsq_entries(APEX_CPU *cpu);

int handleLSQFlushByCFID(APEX_CPU *cpu, int cfId);

/*********************************************** ROB Functions *************************************************/
int initializeROB(APEX_CPU *cpu);

int insertInROB(APEX_CPU *cpu);

struct ROBEntry retireROBEntry(APEX_CPU *cpu);

void print_rob_entries(APEX_CPU *cpu);

struct ROBEntry createDummyROBEntry();

void print_retired_rob_entries(struct ROBEntry firstRetiredROBEntry, struct ROBEntry secondRetiredROBEntry);

int handleROBFlushByCFID(APEX_CPU *cpu, int cfId);

/*********************************************** Functional Units ************************************************/
void print_all_FU_entries(APEX_CPU *cpu);

int initializeFUs(APEX_CPU *cpu);

int intFUExecute(APEX_CPU *cpu);

void flushIntFU(APEX_CPU *cpu);

int mulFuExecute(APEX_CPU *cpu);

void flushMULFU(APEX_CPU *cpu);

int memFUExecute(APEX_CPU *cpu);

void flushMemFU(APEX_CPU *cpu);

int retireStoreROBAsSoonAsItIsReadyForMemFU(APEX_CPU *cpu);

int flushBranchTakenData(APEX_CPU *cpu, struct functionalUnits stage);

void flushStageByStageId(APEX_CPU *cpu, enum StagesEnum stageName);

/*********************************************** CFIO        ************************************************/
int initializeCFIO(APEX_CPU *cpu);

int insertInCFIO(APEX_CPU *cpu);

struct CFIOEntry deleteCFIOEntryByIndex(APEX_CPU *cpu, int index);

int flushAllQuesDueToBranching(APEX_CPU *cpu, struct functionalUnits stage);

int updateCFIOFromROBRetire(APEX_CPU *cpu, struct ROBEntry retiredROBEntry);

#endif
