#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

// Function Declares for Built-In Shell Functions
int shll_cd(char **args);
int shll_help(char **args);
int shll_exit(char **args);

// Built-In Commands
char *builtin_str[] = {
    "cd",
    "help",
    "exit"
};

int (*builtin_func[]) (char **) = {
    &shll_cd,
    &shll_help,
    &shll_exit
};

int shll_num_builtins() {
    return sizeof(builtin_str) / sizeof(char *);
}

// Change Directory Implementation 
int shll_cd(char **args) {
    if (args[1] == NULL) {
        fprintf(stderr, "shll: expected argument to \"cd\"\n");
    } 
    else {
        if (chdir(args[1]) != 0) {
            perror("shll");
        }
    }
  
    return 1;
}

// Help Implementation
int shll_help(char **args) {
    int i;
    printf("Type program arguments, then hit enter.\n");
    printf("The following features are built in:\n");

    for (i = 0; i < shll_num_builtins(); i++) {
        printf("  %s\n", builtin_str[i]);
    }

    printf("Use the main command for information on other programs.\n");
    return 1;
}

// Exit Command Implementation
int shll_exit(char **args) {
    return 0;
}

// Launch Command
int shll_launch(char **args) {
    pid_t pid, wpid;
    int state;

    pid = fork();
    if (pid == 0) {
        // Child process
        if (execvp(args[0], args) == -1) {
            perror("shll");
        }
        exit(EXIT_FAILURE);
        } else if (pid < 0) {
            // Error forking
            perror("shll");
        } else {
            // Parent process
            do {
                wpid = waitpid(pid, &state, WUNTRACED);
            } while (!WIFEXITED(state) && !WIFSIGNALED(state));
        }

    return 1;
}

// Execute the actual shell launch
int shll_execute(char **args) {
    int i;

    if (args[0] == NULL) {
        // An empty command was entered.
        return 1;
    }

    for (i = 0; i < shll_num_builtins(); i++) {
        if (strcmp(args[0], builtin_str[i]) == 0) {
            return (*builtin_func[i])(args);
        }
    }

    return shll_launch(args);
}

#define SHLL_RL_BUFFSIZE 1024
// Read line from input
char *shll_read_line(void) {
    int c;
    int position = 0;
    int buffsize = SHLL_RL_BUFFSIZE;
    char *buffer = malloc(sizeof(char) * buffsize);

    if (!buffer) {
        fprintf(stderr, "shll: allocation error\n");
        exit(EXIT_FAILURE);
    }

    while (1) {
        // Read a single character
        c = getchar();

        // If we hit EOF, replace it with a null character and return
        if (c == EOF || c == '\n') {
            buffer[position] = '\0';
            return buffer;
        } else {
            buffer[position] = c;
        }
        position++;

        // Reallocate if the buffer is exceeded
        if (position >= buffsize) {
            buffsize += SHLL_RL_BUFFSIZE;
            buffer = realloc(buffer, buffsize);
            
            if (!buffer) {
                fprintf(stderr, "shll: allocation error\n");
                exit(EXIT_FAILURE);
            }
        }
    }
}

#define SHLL_TKN_BUFFSIZE 64
#define SHLL_TKN_DELIM " \t\r\n\a"
// Split a string (line) into multiple tokens
char **shll_split_line(char *line) {
    int buffsize = SHLL_TKN_BUFFSIZE, position = 0;
    char **tokens = malloc(buffsize * sizeof(char*));
    char *token;

    if (!tokens) {
        fprintf(stderr, "shll: allocation error\n");
        exit(EXIT_FAILURE);
    }

    token = strtok(line, SHLL_TKN_DELIM);
    while (token != NULL) {
        tokens[position] = token;
        position++;

        if (position >= buffsize) {
            buffsize +=SHLL_TKN_BUFFSIZE;
            tokens = realloc(tokens, buffsize * sizeof(char*));
            
            if (!tokens) {
                fprintf(stderr, "shll: allocation error\n");
                exit(EXIT_FAILURE);
            }
        }

        token = strtok(NULL, SHLL_TKN_DELIM);
    }
    
    tokens[position] = NULL;
    return tokens;
}

// Loop function for getting Input
void shll_loop(void) {
    char *line;
    char **args;
    int state;

    do {
        printf("> ");
        line = shll_read_line();
        args = shll_split_line(line);
        state = shll_execute(args);

        free(line);
        free(args);
    } while (state);
}

int main(int argc, char **argv) {
    // Load config files

    // Run the Command Loop
    shll_loop();

    // Perform Cleanup or Shutdown

    return EXIT_SUCCESS;
}