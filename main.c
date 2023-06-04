#include "instructions/Instruction.h"
#include "instructions/Rate.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ARGS_LIMIT 40
#define CHAR_LIMIT 100

enum Modes { Seq, Par };
enum Modes mode = Seq;

/**
 * Limpa o buffer de entrada.
 *
 * @Return - Void.
 */
void clearInputBuffer(void) {
  if ((fseek(stdin, 0, SEEK_END), ftell(stdin)) ==
      0) { // verifica se existe algum caractere no buffer de entrada.
    int c;
    while ((c = getchar()) && (c != '\n') && (c != EOF)) {
    }; // limpa o buffer.
  }
}


/**
 * Recebe uma linha de comando do usuário e speara cada componentes
 * armazenando-os em um vetor.
 *
 * @Return - Vetor dos componentes da linha de comando.
 */
char **getInput(void) {
    
    // ----------
    // Solicita um input e exibe o modo atual do programa.
    
    static const char *MODES_STRING[] = {"seq", "par"};
    printf("%s > ", MODES_STRING[mode]);
    
    // ----------
    // Recebe a linha de comando e limpa o buffer de entrada
    char input[CHAR_LIMIT];
    fgets(input, CHAR_LIMIT, stdin);
    input[strcspn(input, "\n")] = 0; // remove a quebra de linha
    clearInputBuffer();
    
    // -----------
    // Separa a linha de compando por cada espaço (" ").
    char **splitInput = malloc((ARGS_LIMIT + 1) * sizeof(char*));
    
    char *token = strtok(input, " ");
    int idx = 0;
    while (token != NULL) {
        splitInput[idx] = token;
        token = strtok(NULL, " ");
        idx++;
    }

  return splitInput; // retorna o vetor
}

int main(void) {
  while (1) {

    char **commandLine = getInput(); // recebe a linha de comando.
    // -----------
    // seleciona a instrução da linha de comando e convete para lowercase
    char command[20];
    for (int i = 0; i <= strlen(commandLine[0]); i++)
      command[i] = tolower(commandLine[0][i]);
    
    // ----------
    // verifica qual instrução foi selecionada
    struct Instruction inst;
    if (strcmp(command, "rate") == 0) {
      inst = newRate(commandLine).Instruction;
    } else if (strcmp(command, "help") == 0) {
      printf(" - rate: \n");
      continue;
    } else {
      printf("\033[31mComando não reconhecido digite \"help\" para ver a lista "
             "de comandos.\033[0m\n");
      continue;
    }

    // ---------
    // executa a instrução.
    InstructionReturn ret = inst.exec(&inst);
    if (ret.numOfExceptions > 0) {
        printf("\033[31;4;3;1mERRO AO EXECUTAR INSTRUÇÃO\033[0m\033[31m\n");
        for (int i = 0; i < ret.numOfExceptions; i ++) {
            printf("EXEÇÃO: %s\n", ret.exceptions[i].name);
            printf("CÓDIGO: %d\n", ret.exceptions[i].code);
            printf("%s\n\n", ret.exceptions[i].desc);
        }
        printf("\033[0m");
    } else {
        printf("%s\n", ret.msg);
    }
  }

  return 0;
}
