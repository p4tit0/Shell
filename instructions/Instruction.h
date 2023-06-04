#ifndef INSTRUCTIONS_INSTRUCTION_H
#define INSTRUCTIONS_INSTRUCTION_H

typedef struct{
    int code;
    const char *name;
    const char *desc;
} Exception;

typedef struct{
    char *code;
    char *description;
    char *value;
    int optional;
    int present;
} Parameter;

typedef struct {
    int numOfExceptions;
    Exception *exceptions;
    char *msg;
} InstructionReturn;

struct Instruction {
    Parameter *params;
    char *description;
    /*abstract*/ InstructionReturn (*help)(struct Instruction *self);
    /*abstract*/ InstructionReturn (*exec)(struct Instruction *self);
};

#endif // INSTRUCTIONS_INSTRUCTION_H
