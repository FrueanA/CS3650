#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "tokens.h"
#include "vect.h"
#include <fcntl.h>

#define MAXLINELENGTH 256
#define MAXARGS 64


// case-insensitive string comparison
int str_case(const char *a, const char *b) {
  for (; *a && *b; a++, b++) {
    int diff = tolower((unsigned char) *a) - tolower((unsigned char) *b);
    if (diff != 0)
      return diff;
  }
  return *a - *b;
}

// execute command with possible I/O redirection
void command(char *argv[]) {
  int input_direct = -1;
  int output_direct = -1;
  char *input_file = NULL;
  char *output_file = NULL;

  // parse for I/O redirection
  for (int i = 0; argv[i] != NULL; i++) {
    if (strcmp(argv[i], "<") == 0) {
      argv[i] = NULL;
      input_file = argv[i + 1];
    } else if (strcmp(argv[i], ">") == 0) {
      argv[i] = NULL;
      output_file = argv[i + 1];
    }
  }

  // fork process
  pid_t pid = fork();
  if (pid < 0) {
    perror("fork");
    return;
  }

  // child process
  if (pid == 0) {
    // input redirection
    if (input_file) {
      input_direct = open(input_file, O_RDONLY);
      if (input_direct < 0) {
        perror("open input file");
        exit(1);
      }
      dup2(input_direct, STDIN_FILENO);
      close(input_direct);
    }

    // output redirection
    if (output_file) {
      output_direct = open(output_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
      if (output_direct < 0) {
        perror("open output file");
        exit(1);
      }
      dup2(output_direct, STDOUT_FILENO);
      close(output_direct);
    }

    // execute command
    execvp(argv[0], argv);

    // if execvp Fails
    fprintf(stderr, "%s: command not found\n", argv[0]);
    exit(1);
  } else {
    // parent process
    waitpid(pid, NULL, 0);
  }
}

// execute two commands connected by a pipe with possible I/O redirection
void run_pipe(char **argv_left, char **argv_right) {
    int pipefd[2];

    if (pipe(pipefd) == -1) {
        perror("pipe");
        return;
    }

    pid_t pid1 = fork();
    if (pid1 < 0) {
        perror("fork");
        return;
    }

    if (pid1 == 0) {
        // First child: left side of pipe
        close(pipefd[0]);

        // Redirect stdout to pipe write end
        if (dup2(pipefd[1], STDOUT_FILENO) == -1) {
            perror("dup2");
            exit(1);
        }
        close(pipefd[1]);

        // Handle I/O redirection tokens (< and >)  
        for (int i = 0; argv_left[i] != NULL; i++) {
            if (strcmp(argv_left[i], "<") == 0) {
                char *file = argv_left[i + 1];
                argv_left[i] = NULL; 
                if (!file) {
                    fprintf(stderr, "missing filename after '<'\n");
                    exit(1);
                }
                int fd = open(file, O_RDONLY);
                if (fd < 0) {
                    perror("open input file");
                    exit(1);
                }
                if (dup2(fd, STDIN_FILENO) == -1) {
                    perror("dup2");
                    close(fd);
                    exit(1);
                }
                close(fd);
                break; 
            } else if (strcmp(argv_left[i], ">") == 0) {
                char *file = argv_left[i + 1];
                argv_left[i] = NULL;
                if (!file) {
                    fprintf(stderr, "missing filename after '>'\n");
                    exit(1);
                }
                int fd = open(file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
                if (fd < 0) {
                    perror("open output file");
                    exit(1);
                }
                if (dup2(fd, STDOUT_FILENO) == -1) {
                    perror("dup2");
                    close(fd);
                    exit(1);
                }
                close(fd);
                break;
            }
        }

        execvp(argv_left[0], argv_left);
        fprintf(stderr, "%s: command not found\n", argv_left[0]);
        exit(1);
    }

    pid_t pid2 = fork();
    if (pid2 < 0) {
        perror("fork");
        return;
    }

    if (pid2 == 0) {
        // Second child: right side of pipe
        close(pipefd[1]);

        // Redirect stdin to pipe read end
        if (dup2(pipefd[0], STDIN_FILENO) == -1) {
            perror("dup2");
            exit(1);
        }
        close(pipefd[0]);

        // Handle I/O redirection tokens (< and >) 
        for (int i = 0; argv_right[i] != NULL; i++) {
            if (strcmp(argv_right[i], "<") == 0) {
                char *file = argv_right[i + 1];
                argv_right[i] = NULL;
                if (!file) {
                    fprintf(stderr, "missing filename after '<'\n");
                    exit(1);
                }
                int fd = open(file, O_RDONLY);
                if (fd < 0) {
                    perror("open input file");
                    exit(1);
                }
                if (dup2(fd, STDIN_FILENO) == -1) {
                    perror("dup2");
                    close(fd);
                    exit(1);
                }
                close(fd);
                break;
            } else if (strcmp(argv_right[i], ">") == 0) {
                char *file = argv_right[i + 1];
                argv_right[i] = NULL;
                if (!file) {
                    fprintf(stderr, "missing filename after '>'\n");
                    exit(1);
                }
                int fd = open(file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
                if (fd < 0) {
                    perror("open output file");
                    exit(1);
                }
                if (dup2(fd, STDOUT_FILENO) == -1) {
                    perror("dup2");
                    close(fd);
                    exit(1);
                }
                close(fd);
                break;
            }
        }

        execvp(argv_right[0], argv_right);
        fprintf(stderr, "%s: command not found\n", argv_right[0]);
        exit(1);
    }

    // paent
    close(pipefd[0]);
    close(pipefd[1]);
    waitpid(pid1, NULL, 0);
    waitpid(pid2, NULL, 0);
}


// Prints the shell welcome message
void print_welcome(void) {
  printf("Welcome to mini-shell.\n");
}


// Prints prompt, reads a line from stdin into 'line', returns -1 if EOF was encountered, 0 otherwise.
int read_input_line(char *line, size_t maxlen) {
  printf("shell $ ");
  fflush(stdout);

  if (fgets(line, maxlen, stdin) == NULL) {
    // EOF (Ctrl-D)
    return -1;
  }

  // remove trailing newline
  line[strcspn(line, "\n")] = '\0';
  return 0;
}

// Handle commands and 'prev' and 'help', if command is handled, return 1 to continue main loop, if command is exit, return -1 to break, otherwise return 0 to indicate not handled. 
int handle_special_commands(char *line, char *prev_line) {
  // skip empty lines
  if (strlen(line) == 0)
    return 1;

  // check for "exit"
  if (strcmp(line, "exit") == 0) {
    printf("Bye bye.\n");
    return -1; 
  }

  // handle prev
  if (strcmp(line, "prev") == 0) {
    if (strlen(prev_line) == 0) {
      printf("No previous command.\n");
      return 1; 
    }
    printf("%s\n", prev_line);
    strcpy(line, prev_line);
    return 0; 
  }

  // handle help
  if (strcmp(line, "help") == 0) {
    printf("cd: Change directory to specified path.\n");
    printf("source: Execute a script. Takes a filename as an argument.\n");
    printf("prev: Re-execute the previous command.\n");
    printf("exit: Exit the shell.\n");
    return 1;
  }

  return 0;
}

// Safely store the most recent command into prev_line unless the command is "prev"
void save_prev_line(const char *line, char *prev_line) {
  if (strcmp(line, "prev") != 0) {
    strncpy(prev_line, line, MAXLINELENGTH - 1);
    prev_line[MAXLINELENGTH - 1] = '\0';
  }
}

// Given a token vector, handle separator, pipe, cd, and source.
void process_tokens(vect_t *tokens, char *line, char *prev_line) {
  // build argv array for execvp
  char *args[MAXARGS];
  int argi = 0;

  // handle coMmands separated by ;
  for (unsigned int i = 0; i < vect_size(tokens); i++) {
    const char *tok = vect_get(tokens, i);

    // check for separator
    if (strcmp(tok, ";") == 0) {
      if (argi > 0) {
        args[argi] = NULL;
        command(args);
        argi = 0;
      }
    } else {
      args[argi++] = (char *)tok;
    }
  }

  // handle last command if any
  if (argi > 0) {
    args[argi] = NULL;

    // check for pipe
    int pipe_index = -1;
    for (int i = 0; args[i] != NULL; i++) {
        if (strcmp(args[i], "|") == 0) {
            pipe_index = i;
            break;
        }
    }
    // split and run pipe
    if (pipe_index != -1) {
        args[pipe_index] = NULL;
        char **argv_left = args;
        char **argv_right = &args[pipe_index + 1];

        run_pipe(argv_left, argv_right);
        vect_delete(tokens);
        return;
    }

    // implement cd
    if (strcmp(args[0], "cd") == 0) {
      if (args[1] == NULL) {
        char *home = getenv("HOME");
        if (home == NULL) home = "/";
        if (chdir(home) != 0) {
          perror("cd");
        }
      } else {
        if (chdir(args[1]) != 0) {
          perror("cd");
        }
      }
    }
    // implement source (execute script)
    else if (strcmp(args[0], "source") == 0) {
      if (args[1] == NULL) {
        fprintf(stderr, "source: missing filename\n");
      } else {
        FILE *f = fopen(args[1], "r");
        if (!f) {
          perror("source");
        } else {
          char script_line[MAXLINELENGTH];
          while (fgets(script_line, sizeof(script_line), f)) {
            script_line[strcspn(script_line, "\n")] = '\0';
            if (strlen(script_line) == 0) continue;

            vect_t *toks2 = tokenize(script_line);
            if (toks2 == NULL || vect_size(toks2) == 0) {
              vect_delete(toks2);
              continue;
            }


            // build argv array for script line
            char *args2[MAXARGS];
            int ai = 0;
            for (unsigned int j = 0; j < vect_size(toks2); j++) {
              args2[ai++] = (char *)vect_get(toks2, j);
            }
            args2[ai] = NULL;

            if (strcmp(args2[0], "cd") == 0) {
              if (args2[1] == NULL) {
                char *home = getenv("HOME");
                if (home == NULL) home = "/";
                if (chdir(home) != 0) {
                  perror("cd");
                }
              } else {
                if (chdir(args2[1]) != 0) {
                  perror("cd");
                }
              }
            } else {
              command(args2);
            }

            vect_delete(toks2);
          }
          fclose(f);
        }
      }
    } else {
      command(args);
    }

    // save the command line as the previous command line
    save_prev_line(line, prev_line);
  }

  // free vector memory
  vect_delete(tokens);
}

// main shell loop
int main() {
  char line[MAXLINELENGTH];
  char prev_line[MAXLINELENGTH] = "";

  print_welcome();


  // main loop
  while (1) {
    // read input line (prompt + fgets)
    if (read_input_line(line, sizeof(line)) == -1) {
      // EOF (Ctrl-D)
      printf("\nBye bye.\n");
      break;
    }

    // skip empty lines
    if (strlen(line) == 0)
      continue;

    // handle commands exit, help, prev
    int special = handle_special_commands(line, prev_line);
    if (special == -1) {
      break;
    } else if (special == 1) {
      continue;
    }

    // tokenize the line to get a vector of string token
    vect_t *tokens = tokenize(line);
    if (tokens == NULL || vect_size(tokens) == 0) {
      vect_delete(tokens);
      continue;
    }

    // build args, handle ;, |, cd, source, command, and save prev
    process_tokens(tokens, line, prev_line);
  }

  return 0;
}
