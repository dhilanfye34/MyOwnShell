#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <dirent.h>
#define max_arguments 20

typedef struct {
  char name[100];
  char value[300];
} shellVariable;

shellVariable variables[500];
int var_count = 0;

void set_var(char* name, char* value) {
  for (int i = 0; i < var_count; i++) {
    if (strcmp(variables[i].name, name) == 0) {
      strcpy(variables[i].value, value);
      return;
    }
  }
  if (var_count < 500) {
    strcpy(variables[var_count].name, name);
    strcpy(variables[var_count].value, value);
    var_count++;
  }
  else {
    printf("Limit reached.\n");
  }
}

char* get_var(char* name) {
  for (int i = 0; i < var_count; i++) {
    if (strcmp(variables[i].name, name) == 0) {
      return variables[i].value;
    }
  }
  return NULL;
}

void print_vars() {
  for (int i = 0; i < var_count; i++) {
    printf("%s %s\n", variables[i].name, variables[i].value);
  }
}

typedef struct {
  char name[100];
  char command[300];
} Alias;

Alias aliases[500];
int alias_count = 0;

void set_alias(char* name, char* command) {
  for (int i = 0; i < alias_count; i++) {
    if (strcmp(aliases[i].name, name) == 0) {
      strcpy(aliases[i].command, command);
      return;
    }
  }
  if (alias_count < 500) {
    strcpy(aliases[alias_count].name, name);
    strcpy(aliases[alias_count].command, command);
    alias_count++;
  }
  else {
    printf("Limit reached.\n");
  }
}

char* get_alias(char* name) {
  for (int i = 0; i < alias_count; i++) {
    if (strcmp(aliases[i].name, name) == 0) {
      return aliases[i].command;
    }
  }
  return NULL;
}

void print_aliases() {
  for (int i = 0; i < alias_count; i++) {
    printf("%s %s\n", aliases[i].name, aliases[i].command);
  }
}

void glob_func(char*** arguments, int* arg_count) {
  for (int i = 0; i < *arg_count; i++) {
    if ((*arguments)[i][0] == '*') {
      DIR* dir = opendir(".");
      struct dirent* entry;
      int count = 1;
      char** new_args = malloc(100 * sizeof(char*));
      new_args[0] = strdup((*arguments)[0]);

      if (dir) {
        while ((entry = readdir(dir)) != NULL) {
          if (entry->d_name[0] == '.') { 
            continue; 
          }

          // if the glob pattern is jsut * or files with specific type
          if ((*arguments)[i][1] == '\0') { 
            new_args[count++] = strdup(entry->d_name); // match all files
          } 
          else {
            int type_file_len = strlen((*arguments)[i]);
            int file_len = strlen(entry->d_name);

            // check if filename ends with whatever type
            if (file_len >= type_file_len - 1 && strcmp(entry->d_name + file_len - (type_file_len - 1), (*arguments)[i] + 1) == 0) {
              new_args[count++] = strdup(entry->d_name); // match files ending with whatever type
            }
          }
        }
        closedir(dir);

        // get rest of args
        for (int j = i + 1; j < *arg_count; j++) {
          new_args[count++] = (*arguments)[j];
        }

        new_args[count] = NULL;

        free(*arguments); // free old arguments

        // update args with new list
        *arguments = new_args;
        *arg_count = count;
      }
    }
  }
}


char** parse_input(char* input) {
  char* str = malloc(500 * sizeof(char));
  if (str == NULL) {
    printf("Malloc failed\n");
    exit(1);
  }

  char** arguments = malloc(max_arguments * sizeof(char*));
  if (arguments == NULL) {
    printf("Malloc failed\n");
    exit(1);
  }

  int index = 0;
  int arg_index = 0;
  bool in_quotes = false;
  
  for (int i = 0; input[i] != '\0'; i++) {
    if (input[i] == '"') {
        in_quotes = !in_quotes; //opposite of what it is right now
        continue;
    }
    
    if (input[i] == ' ' && !in_quotes) { //if we hit space and not in quotes, needs to end current arg
      if (arg_index > 0) {
        str[arg_index] = '\0'; //end current arg
        arguments[index] = strdup(str); //add it to arguments array
        index++; 

        if(index >= max_arguments) {
          printf("Too many args\n");
          break;
        }
        arg_index = 0;
      }
    } 
    else {
      str[arg_index] = input[i]; //or just add character
      arg_index++;
    }
  }

  if (arg_index > 0) { //finish up last arg
    str[arg_index] = '\0';
    arguments[index] = strdup(str);
    index++;
  }

  arguments[index] = NULL;

  free(str);  
  return arguments;
}

void execute_command(char** arguments) {
  int file;
  pid_t p = fork();
  if (p == 0) {
    for (int i = 0; arguments[i] != NULL; i++) {
      if (strcmp(arguments[i], ">") == 0) {
        file = open(arguments[i + 1], O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if (file == -1) {
          printf("Error opening file for output\n");
          exit(1);
        }
        dup2(file, STDOUT_FILENO);
        close(file);
        arguments[i] = NULL;
        arguments[i+1] = NULL;
        break;
      }
      if (strcmp(arguments[i], "<") == 0) {
        file = open(arguments[i + 1], O_RDONLY);
        if (file == -1) {
          printf("Error opening file for input\n");
          exit(1);
        }
        dup2(file, STDIN_FILENO);
        close(file);
        arguments[i] = NULL;
        arguments[i+1] = NULL;
        break;
      }
      if (strcmp(arguments[i], "|") == 0) {
        arguments[i] = NULL;
        int pipe_f[2];
        if (pipe(pipe_f) == -1) {
          printf("Pipe Error\n");
          exit(1);
        }
        pid_t pipe_p = fork();
        if (pipe_p == 0) {
          dup2(pipe_f[1], STDOUT_FILENO);
          close(pipe_f[0]);
          close(pipe_f[1]);
          execvp(arguments[0], arguments);
          printf("Error with first command\n");
          exit(1);
        } 
        else if (pipe_p > 0) {
          wait(NULL);
          dup2(pipe_f[0], STDIN_FILENO);
          close(pipe_f[0]);
          close(pipe_f[1]);
          execvp(arguments[i + 1], &arguments[i + 1]);
          printf("Error with second command\n");
          exit(1);
        }
      }
    }

    int exec_result = execvp(arguments[0], arguments);
    if (exec_result == -1) {
      printf("Error executing command: %s\n", arguments[0]);
      exit(1);
    }
  } 
  else if (p > 0) {
    wait(NULL);
  } 
  else {
    printf("Error forking\n");
  }
}

void exp_alias(char*** arguments, int* arg_count) {
  char* alias_command = get_alias((*arguments)[0]);

  if (alias_command != NULL) {
    char** alias_args = parse_input(alias_command);
    int alias_arg_count = 0;

    while (alias_args[alias_arg_count] != NULL) {
      alias_arg_count++;
    }

    char** new_args = malloc((*arg_count + alias_arg_count) * sizeof(char*));

    for (int i = 0; i < alias_arg_count; i++) {
      new_args[i] = strdup(alias_args[i]);
    }

    for (int i = 1; i < *arg_count; i++) {
      new_args[alias_arg_count + i - 1] = (*arguments)[i];
    }

    new_args[alias_arg_count + *arg_count - 1] = NULL;

    for (int i = 0; i < *arg_count; i++) {
      free((*arguments)[i]);
    }
    free(*arguments);
    
    for (int i = 0; i < alias_arg_count; i++) {
      free(alias_args[i]);
    }
    free(alias_args);

    *arguments = new_args;
    *arg_count = alias_arg_count + *arg_count - 1;
  }
}



void echo_func(char** arguments) {
  for (int i = 1; arguments[i] != NULL; i++) {
    printf("%s ", arguments[i]);
  }
  printf("\n");
}

int main() {
  char input[500];
  char** arguments;
  int arg_count;

  while (true) {
    printf("dmf_shell> ");
    if (fgets(input, sizeof(input), stdin) == NULL) {
      continue;
    }
    strtok(input, "\n");
    arguments = parse_input(input);

    if (arguments[0] == NULL) {
      free(arguments);
      continue;
    }

    if (strcmp(arguments[0], "exit") == 0) {
      free(arguments);
      break;
    }

    // alias command, if no name, list all
    if (strcmp(arguments[0], "alias") == 0) {
      if (arguments[1] == NULL) {
        print_aliases();
      } 
      else if (arguments[2] != NULL) {
        char command[300] = "";
        for (int j = 2; arguments[j] != NULL; j++) {
          strcat(command, arguments[j]);
          if (arguments[j + 1] != NULL){
            strcat(command, " ");
          }
        }
        set_alias(arguments[1], command); 
      } 
      else {
        printf("error, invalid alias\n");
      }
      free(arguments);
      continue;
    }

    //set with shell vars
    if (strcmp(arguments[0], "set") == 0) {
      if (arguments[1] == NULL) {
        print_vars();
      } 
      else if (arguments[2] != NULL) {
        set_var(arguments[1], arguments[2]);
      } 
      else {
        char* value = get_var(arguments[1]);
        if (value){ 
          printf("%s\n", value);
        }
        else {
          printf("Variable not found\n");
        }
      }
      free(arguments);
      continue;
    }

    // handling $ 
    arg_count = 0;
    while (arguments[arg_count] != NULL) {
      if (arguments[arg_count][0] == '$') {
        char* var_value = get_var(arguments[arg_count] + 1);
        if (var_value) {
          free(arguments[arg_count]);
          arguments[arg_count] = strdup(var_value);
        }
      }
      arg_count++;
    }

    exp_alias(&arguments, &arg_count);  //expands alias 
    glob_func(&arguments, &arg_count);  //handles *. file types

    execute_command(arguments);

    for (int i = 0; i < arg_count; i++) {
      free(arguments[i]);
    }
    free(arguments);
  }
  return 0;
}