#ifndef INSTRUCTIONS_RATE_H
#define INSTRUCTIONS_RATE_H

#include "Instruction.h"

#include <math.h>
#include <regex.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#define NUM_OF_PARAMS 1

enum exceptions {
  PATH_VER_ERR = 0,
  INV_PATH,
  INV_FILE_EXT,
  CANT_OPEN_FILE,
  INV_SCHED_TIME,
  NO_TASKS,
  INS_TASK_ARGS,
  INV_TASK_ARGS,
  MAX_EXCEPTIONS,
  COMPILE_REGEX_ERROR,
  MATCH_FAILED
};

static const char *const exception_names[] = {
    [PATH_VER_ERR] = "PATH_VERIFICATION_ERROR",
    [INV_PATH] = "INVALID_PATH_EXCEPTION",
    [INV_FILE_EXT] = "INVALID_FILE_EXTENSION_EXCEPTION",
    [CANT_OPEN_FILE] = "CANT_OPEN_FILE",
    [INV_SCHED_TIME] = "INVALID_SCHEDULING_TIME",
    [NO_TASKS] = "NO_TASKS",
    [INS_TASK_ARGS] = "INSUFICIENT_TASK_ARGUMENTS",
    [INV_TASK_ARGS] = "INVALID_TASK_ARGUMENTS",
    [COMPILE_REGEX_ERROR] = "COMPILE_REGEX_ERROR",
    [MATCH_FAILED] = "REGEX_MATCH_FAILED"
};

static const char *const exception_desc[] = {
    [PATH_VER_ERR] = "Erro ao verificar o caminho informado! O caminho informado não existe.",
    [INV_PATH] = "O caminho informado é inválido! O caminho deve ser para um arquivo \".txt\".",
    [INV_FILE_EXT] = "O arquivo de input não é um \".txt\"!",
    [CANT_OPEN_FILE] = "O arquivo não pôde ser aberto!",
    [INV_SCHED_TIME] = "Tempo total de simulação inválido!",
    [NO_TASKS] = "Não há tarefas o suficiente no arquivo de input para realizar a simulação!",
    [INS_TASK_ARGS] ="Uma ou mais instruções não possúem parâmetros o suficiente!",
    [INV_TASK_ARGS] = "Uma ou mais instruções possúi parâmetros inválidos!",
    [COMPILE_REGEX_ERROR] = "Erreo ao compilar o Regex",
    [MATCH_FAILED] = "Falha na correspondência Regex"
};

Exception execptionArr[MAX_EXCEPTIONS];
int numOfExceptions = 0;

struct CharList {
  char data;
  struct CharList *next;
};

typedef struct CharList *Node;

struct task {
  const char *name;
  int execTime;
  int period;
};

struct task *tasks;
int numOfTasks;
int totalTimeunits;

struct Action {
  struct task currtask;
  int cont;
  int continuation;
  int reachedDeadLine;
};

struct ActionList {
  struct Action data;
  struct ActionList *next;
};
typedef struct ActionList *ActionNode;

ActionNode simulation;

struct Rate {
  struct Instruction Instruction;
};

/**
 * Organiza um array de tarefas em ordem decrescente em função do tempo de
 * execução.
 *
 * @Param arr - Array de tarefas a ser ordenado.
 * @Param n - Tamanho do array.
 *
 * @Return - None.
 */
void insertionSort(struct task arr[], int n) {
  struct task current;
  int j;

  for (int i = 1; i < n; i++) {
    current = arr[i];
    j = i - 1;
    while (j >= 0 && arr[j].execTime < current.execTime) {
      arr[j + 1] = arr[j];
      j--;
    }
    arr[j + 1] = current;
  }
}

/**
 * Instancia um novo elemento de uma lista de caracteres.
 *
 * @Return - Char node.
 */
Node createNode() {
  Node temp;
  temp = (Node)malloc(sizeof(struct CharList));
  temp->next = NULL;
  return temp;
}

/**
 * Adiciona um novo elemento a uma lista de caracteres.
 *
 * @Return - novo head node.
 */
Node addNode(Node head, char value) {
  Node temp, p;
  temp = createNode();
  temp->data = value;
  if (head == NULL) {
    head = temp;
  } else {
    p = head;
    while (p->next != NULL)
      p = p->next;
    p->next = temp;
  }
  return head;
}

/**
 * Instancia um novo elemento de uma lista de ações.
 *
 * @Return - novo Action node.
 */
ActionNode createActionNode() {
  ActionNode temp;
  temp = (ActionNode)malloc(sizeof(struct ActionList));
  temp->next = NULL;
  return temp;
}

/**
 * Adiciona um novo elemento a uma lista de ações.
 *
 * @Return - novo head node.
 */
ActionNode addAction(ActionNode head, struct Action value) {
  ActionNode temp, p;
  temp = createActionNode();
  temp->data = value;
  if (head == NULL) {
    head = temp;
  } else {
    p = head;
    while (p->next != NULL)
      p = p->next;
    p->next = temp;
  }
  return head;
}

/**
 * Adiciona uma exceção ao array de exceções.
 *
 * @Return - none.
 */
static void addException(enum exceptions exception_type) {
  execptionArr[numOfExceptions].code = exception_type;
  execptionArr[numOfExceptions].name = exception_names[exception_type];
  execptionArr[numOfExceptions].desc = exception_desc[exception_type];
  numOfExceptions++;
}

/**
 * Verifica se uma string terimina com determinada substring.
 *
 * @Return - retorno da verificação.
 */
static int endsWith(char *str, const char *substr) {
  if (!str || !substr)
    return 0;

  size_t lenStr = strlen(str);
  size_t lenSubstr = strlen(substr);

  if (lenSubstr > lenStr)
    return 0;

  int endsWith = 1;
  for (int i = 0; i < lenSubstr; i++) {
    if (str[i + (lenStr - lenSubstr)] != substr[i]) {
      endsWith = 0;
      break;
    }
  }

  return endsWith;
}

static int checkRegex(char *regex, char *textToCheck) {
  regex_t ptrn_bf;
  int reti;

  reti = regcomp(&ptrn_bf, regex, REG_EXTENDED);
  if (reti) {
    addException(COMPILE_REGEX_ERROR);
  }

  reti = regexec(&ptrn_bf, textToCheck, 0, NULL, 0);
  if (!reti) {
    return 1;
  } else if (reti == REG_NOMATCH) {
    return 0;
  } else {
    addException(MATCH_FAILED);
  }

  regfree(&ptrn_bf);
}

static void readInputFile(char *filePath) {
  FILE *fp;
  char *line = NULL;
  size_t len = 0;
  ssize_t read;

  fp = fopen(filePath, "r");
  if (fp == NULL) {
    addException(CANT_OPEN_FILE);
    return;
  }

  int lineIdx = 0;
  while ((read = getline(&line, &len, fp)) != -1) {
    if (lineIdx == 0) {
      if(!checkRegex("^[0-9]+$", line)) {
        addException(INV_SCHED_TIME);
        return;
      }
      
    }
    lineIdx++;
  }
  
  //idx = 0;
  

  char *token = strtok(txt, "\r\n");
  char **lines;
  while (token != NULL) {
    char *aux[idx + 1];
    for (int i = 0; i < idx; i++) {
      aux[i] = lines[i];
    }
    free(lines);
    aux[idx] = token;
    lines = (char **)malloc(sizeof(aux));
    for (int i = 0; i <= idx; i++) {
      lines[i] = aux[i];
    }
    token = strtok(NULL, "\r\n");
    idx++;
  }
  printf("%s\n%s\n%s\n", lines[0], lines[1], lines[2]);
  /*totalTimeunits = atoi(lines[0]);
  if (totalTimeunits <= 0) {
    addException(INV_SCHED_TIME);
  }
  numOfTasks = idx - 1;
  if (numOfTasks <= 0) {
    addException(NO_TASKS);
  }

  tasks = (struct task *)malloc(sizeof(struct task) * numOfTasks);
  for (int i = 0; i < idx - 1; i++) {
    tasks[i].name = strtok(lines[i + 1], " ");
    char *aux = strtok(NULL, " ");
    char *aux1 = strtok(NULL, " ");
    if (aux == NULL || aux1 == NULL) {
      addException(INS_TASK_ARGS);
    }
    tasks[i].period = atoi(aux);
    tasks[i].execTime = atoi(aux1);
    if (tasks[i].period <= 0 || tasks[i].execTime <= 0 ||
        tasks[i].execTime > tasks[i].period) {
      addException(INV_TASK_ARGS);
    }
  } */

  fclose(fp);
}

/**
 * Exibe a mensagem de ajuda do comando.
 *
 * @Return - None.
 */
static InstructionReturn help(struct Instruction *self) {
  InstructionReturn ret;
  ret.msg = "AJUDA EXIBIDA";
  return ret;
}

static InstructionReturn exec(struct Instruction *self) {
  simulation = createActionNode();
  InstructionReturn ret;
  ret.numOfExceptions = numOfExceptions;
  if (ret.numOfExceptions > 0) {
    ret.exceptions = (Exception *)malloc(sizeof(Exception) * numOfExceptions);

    for (int i = 0; i < numOfExceptions; i++)
      ret.exceptions[i] = execptionArr[i];
    return ret;
  }

  /*insertionSort(tasks, numOfTasks);
  struct Action *pendingActons[] = {NULL, NULL, NULL, NULL};
  int actCont = 0;
  int idx = 0;

for (int i = 1; i <= numOfTasks; i++) {
  int numOfCycles = (int)floor(totalTimeunits / (double)tasks[i].period);
  for (int j = 0; j < numOfCycles; j++) {

    int deadLine =
        tasks[j].period * (1 + ((int)floor(idx / ((double)tasks[i].period))));
    ActionNode search = simulation;
    int sumExec = 0;
    for (int i = 0;
         i < actCont && sumExec + search->data.currtask.execTime < deadLine;
         i++) {
      sumExec += search->data.currtask.execTime;
      search = search->next;
    }

    if ((deadLine - idx) >= tasks[j].execTime && search == NULL) {
      struct Action current = {.currtask = tasks[i],
                               .cont = tasks[i].execTime};
      if (idx == 0) {
        simulation->data = current;
      } else {
        simulation = addAction(simulation, current);
      }
      actCont++;
      idx += tasks[i].execTime;
    }
  }
  idx = 0;
}
printf("%d\n", actCont);
ActionNode search = simulation;
for (int i = 0; i < actCont; i++) {
  printf("%d\n", search->data.cont);
  search = search->next;
}*/
  // ret.msg = "O escalonamento das instruções foi concluído!";
  char ret_msg[] = "O escalonamento das instruções foi concluído!";
  ret.msg = (char *)malloc(sizeof(ret_msg));
  // ret.msg[1] = '\0';
  strcpy(ret.msg, ret_msg);
  return ret;
}

extern struct Rate newRate(char **parameters) {
  char desc[] =
      "Realiza a simulação de um algorítimo \033[3mrate-monotonic\033[0m.";
  struct Instruction inst = {
      .params = (Parameter *)malloc(sizeof(Parameter) * NUM_OF_PARAMS),
      .description = (char *)malloc(sizeof(desc)),
      .help = &help,
      .exec = &exec};
  strcpy(inst.description, desc);
  // defineParam(&inst, 0, "nameFile", "Caminho para o arquivo de input",
  // parameters[0], 0);

  struct stat s;
  char path[strlen(parameters[1]) + 1];
  strcpy(path, parameters[1]);
  if (stat(parameters[1], &s) == 0) {
    if (s.st_mode & S_IFREG) {
      if (endsWith(path, ".txt")) {
        readInputFile(path);
      } else {
        addException(INV_FILE_EXT);
      }
    } else {
      addException(INV_PATH);
    }
  } else {
    addException(PATH_VER_ERR);
  }

  struct Rate ret = {.Instruction = inst};
  return ret;
}

#endif // INSTRUCTIONS_RATE_H
