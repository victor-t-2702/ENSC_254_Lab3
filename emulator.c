#include <stdio.h> // for stderr
#include <stdlib.h> // for exit()
#include "types.h"
#include "utils.h"
#include "riscv.h"

void execute_rtype(Instruction, Processor *);
void execute_itype_except_load(Instruction, Processor *);
void execute_branch(Instruction, Processor *);
void execute_jal(Instruction, Processor *);
void execute_load(Instruction, Processor *, Byte *);
void execute_store(Instruction, Processor *, Byte *);
void execute_ecall(Processor *, Byte *);
void execute_lui(Instruction, Processor *);

void execute_instruction(uint32_t instruction_bits, Processor *processor,Byte *memory) {    
    Instruction instruction = parse_instruction(instruction_bits);
    switch(instruction.opcode) {
        case 0x33:
            execute_rtype(instruction, processor);
            break;
        case 0x13:
            execute_itype_except_load(instruction, processor);
            break;
        case 0x73:
            execute_ecall(processor, memory);
            break;
        case 0x63:
            execute_branch(instruction, processor);
            break;
        case 0x6F:
            execute_jal(instruction, processor);
            break;
        case 0x23:
            execute_store(instruction, processor, memory);
            break;
        case 0x03:
            execute_load(instruction, processor, memory);
            break;
        case 0x37:
            execute_lui(instruction, processor);
            break;
        default: // undefined opcode
            handle_invalid_instruction(instruction);
            exit(-1);
            break;
    }
}

void execute_rtype(Instruction instruction, Processor *processor) {
    switch (instruction.rtype.funct3){
        case 0x0:
            switch (instruction.rtype.funct7) {
                case 0x0:
                    // Add
                    processor->R[instruction.rtype.rd] =               //  Access the "instruction.rtype.rd" indexed member of the Processor's array of registers (as a signed 32-bit number)
                        ((sWord)processor->R[instruction.rtype.rs1]) + //  Access the "instruction.rtype.rs1" indexed member of the Processor's array of registers (as a signed 32-bit number)
                        ((sWord)processor->R[instruction.rtype.rs2]);  //  Access the "instruction.rtype.rs2" indexed member of the Processor's array of registers (as a signed 32-bit number)
                    break;
                case 0x1:
                    // Mul
                    processor->R[instruction.rtype.rd] =
                        ((sWord)processor->R[instruction.rtype.rs1]) *
                        ((sWord)processor->R[instruction.rtype.rs2]);
                    break;
                case 0x20:
                    // Sub
                    processor->R[instruction.rtype.rd] =
                        ((sWord)processor->R[instruction.rtype.rs1]) -
                        ((sWord)processor->R[instruction.rtype.rs2]);
                    break;
                default:
                    handle_invalid_instruction(instruction);
                    exit(-1);
                    break;
            }
            break;
        /* YOUR CODE HERE */
	/* deal with other cases */
        default:
            handle_invalid_instruction(instruction);
            exit(-1);
            break;
    }
    // update PC
    processor->PC += 4;
}

void execute_itype_except_load(Instruction instruction, Processor *processor) {
    switch (instruction.itype.funct3) {
        /* YOUR CODE HERE */
        default:
            handle_invalid_instruction(instruction);
            break;
    }
}

void execute_ecall(Processor *p, Byte *memory) {
    Register i;
    
    // syscall number is given by a0 (x10)
    // argument is given by a1
    switch(p->R[10]) {
        case 1: // print an integer
            printf("%d",p->R[11]);
            p->PC += 4;
            break;
        case 4: // print a string
            for(i=p->R[11];i<MEMORY_SPACE && load(memory,i,LENGTH_BYTE);i++) {
                printf("%c",load(memory,i,LENGTH_BYTE));
            }
            p->PC += 4;
            break;
        case 10: // exit
            printf("exiting the simulator\n");
            exit(0);
            break;
        case 11: // print a character
            printf("%c",p->R[11]);
            p->PC += 4;
            break;
        default: // undefined ecall
            printf("Illegal ecall number %d\n", p->R[10]);
            exit(-1);
            break;
    }
}

void execute_branch(Instruction instruction, Processor *processor) {
    switch (instruction.sbtype.funct3) {
        /* YOUR CODE HERE */
        default:
            handle_invalid_instruction(instruction);
            exit(-1);
            break;
    }
}

void execute_load(Instruction instruction, Processor *processor, Byte *memory) {
    Address offsetAddress = (processor->R[instruction.itype.rs1]) + instruction.itype.imm;   // Calculate offsetted address for load (with base address in rs1)
    switch (instruction.itype.funct3) {
        case 0x0:
            processor->R[instruction.itype.rd] = sign_extend_number(load(memory, offsetAddress, LENGTH_BYTE), 8);  // Load in signed extended byte from offsetted address
            break;
        case 0x1:
            processor->R[instruction.itype.rd] = sign_extend_number(load(memory, offsetAddress, LENGTH_HALF_WORD), 16);  // Load in signed extended half-word from offsetted address
            break;
        case 0x2:
            processor->R[instruction.itype.rd] = load(memory, offsetAddress, LENGTH_WORD);  // Load in word from offsetted address
            break;
        default:
            handle_invalid_instruction(instruction);
            break;
    }
    // update PC
    processor->PC += 4;
}

void execute_store(Instruction instruction, Processor *processor, Byte *memory) {
    Address offsetAddress = (processor->R[instruction.stype.rs1]) + get_store_offset(instruction);   // Calculate offsetted address for load (with base address in rs1)
    switch (instruction.stype.funct3) {
        case 0x0:
            store(memory, offsetAddress, LENGTH_BYTE, processor->R[instruction.stype.rs2]);
            break;
        case 0x1:
            store(memory, offsetAddress, LENGTH_HALF_WORD, processor->R[instruction.stype.rs2]);
            break;
        case 0x2:
            store(memory, offsetAddress, LENGTH_WORD, processor->R[instruction.stype.rs2]);
            break;
        default:
            handle_invalid_instruction(instruction);
            exit(-1);
            break;
    }
    // update PC
    processor->PC += 4;
}

void execute_jal(Instruction instruction, Processor *processor) {
    processor->R[instruction.ujtype.rd] = (processor->PC + 4);
    processor->PC += get_jump_offset(instruction);
}

void execute_lui(Instruction instruction, Processor *processor) {
    processor->R[instruction.utype.rd] = instruction.utype.imm << 12;
    // update PC
    processor->PC += 4;
}

void store(Byte *memory, Address address, Alignment alignment, Word value) { // "memory" is a pointer a place in memory (i.e. base address)
    if(alignment == LENGTH_BYTE) {
        memory[address] = value & ((1U << 8) - 1);
    } else if(alignment == LENGTH_HALF_WORD) {
        memory[address] = value & ((1U << 8) - 1);
        memory[address+1] = (value << 8) & ((1U << 8) - 1);  // Address +1 is address with 1 byte offset
    } else if(alignment == LENGTH_WORD) {
        memory[address] = value & ((1U << 8) - 1);
        memory[address+1] = (value << 8) & ((1U << 8) - 1);
        memory[address+2] = (value << 16) & ((1U << 8) - 1);
        memory[address+3] = (value << 24) & ((1U << 8) - 1);
    } else {
        printf("Error: Unrecognized alignment %d\n", alignment);
        exit(-1);
    }
}

Word load(Byte *memory, Address address, Alignment alignment) {
    if(alignment == LENGTH_BYTE) {
        return memory[address];
    } else if(alignment == LENGTH_HALF_WORD) {
        return (memory[address+1] << 8) + memory[address];  // Address +1 is address with 1 byte offset
    } else if(alignment == LENGTH_WORD) {
        return (memory[address+3] << 24) + (memory[address+2] << 16)
               + (memory[address+1] << 8) + memory[address];
    } else {
        printf("Error: Unrecognized alignment %d\n", alignment);
        exit(-1);
    }
}
