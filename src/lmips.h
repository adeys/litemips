#ifndef LMIPS_MIPS
#define LMIPS_MIPS

#include "common.h"
#include "lmips_registers.h"

#define STACK_SIZE (1024 * 64) // 64K
#define MEMORY_SIZE (UINT16_MAX * 4) // 512K

struct lm {
    uint8_t* program;
    int32_t regs[REG_COUNT];
    uint32_t ip;
    int32_t hi, lo;
    int32_t memory[MEMORY_SIZE];
    bool stop;
};

typedef enum {
    EXEC_SUCCESS,
    EXEC_FAILURE,
    EXEC_ERR_INT_OVERFLOW,
    EXEC_ERR_MEMORY_ADDR
} ExecutionResult ;

typedef struct lm LMips;

void initSimulator(LMips* mips, uint8_t* program);
void freeSimulator(LMips* mips);
ExecutionResult runSimulator(LMips* mips);
ExecutionResult execInstruction(LMips* mips);

int32_t zero_extend(uint16_t x, int bit_count);
int32_t sign_extend(int16_t x, int bit_count);
int16_t zero_extend_byte(uint8_t x, int bit_count);
int16_t sign_extend_byte(int8_t x, int bit_count);

void handleException(ExecutionResult);

int32_t mem_read(LMips* mips, uint32_t address);
int16_t mem_read_byte(LMips* mips, uint32_t address, bool sign);
int32_t mem_read_half(LMips* mips, uint32_t address, bool sign);

void mem_write(LMips* mips, uint32_t address, int32_t value);
void mem_write_byte(LMips* mips, uint32_t address, int32_t value);
void mem_write_half(LMips* mips, uint32_t address, int32_t value);

#endif // LMIPS_MIPS
