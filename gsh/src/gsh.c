// 10/01/2026 
// Huy Sy Nguyen
// huysynguyen@usf.edu
// U38376150
// Description: A simple shell implementation

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/wait.h>
#include <fcntl.h>

void exit_command(char *cur_path, char *buffer);

void path_command(char **cur_path, const char *path);

void ls_command();

void run_command(char *path, char **arg);

void run_help_command(char *path, char **arg);

void run_redirect_command(char *path, char *input_path, char **arg);

void print_help(const char *game_name, const char *filename);

int is_directory(const char *path);

int main(int argc, char *argv[]) {

    // Variables
    char error_message[30] = "An error has occurred\n";

    char *buffer = NULL;
    size_t bufsize = 0;

    char *cur_token = NULL;
    char *next_token = NULL;
    char *command = NULL;
    char *cur_path = NULL;
    
    // Check argument counts
    if (argc != 2) {
        write(STDERR_FILENO, error_message, strlen(error_message));
        fflush(stderr); 
        exit(1);
    }

    // Check if the argument is a directory and store it
    if (!is_directory(argv[1])) {
        write(STDERR_FILENO, error_message, strlen(error_message));
        fflush(stderr);
        exit(1);
    }
    else {
        cur_path = strdup(argv[1]);
    }
    
    while(1) {

        // Print the prompt
        printf("gsh> ");
        fflush(stdout);

        // Read the user input
        getline(&buffer, &bufsize, stdin);

        // Get the first token as command, skipping empty tokens
        char *line = buffer;

        // Process first token as the command
        do {
        command = strsep(&line, " \t\n");
        } while (command != NULL && *command == '\0');

        // If no command is found, continue to the next iteration
        if (command == NULL) {
            continue;
        }

        // Exit command
        if (strcmp(command, "exit") == 0) {

            // Check if there are any additional arguments, skipping empty ones, should be NULL
            do {
            cur_token=strsep(&line, " \t\n");
            } while (cur_token != NULL && *cur_token == '\0');

            // If there is an additional argument, it's an error
            if (cur_token != NULL) {
                write(STDERR_FILENO, error_message, strlen(error_message));
                fflush(stderr);
                continue;
            }

            // If there are no additional arguments, exit the program
            else {
                exit_command(cur_path, buffer);
            }
        }

        else if (strcmp(command, "path") == 0) {

            // Get the first argument
            do {
            cur_token=strsep(&line, " \t\n");
            } while (cur_token != NULL && *cur_token == '\0');

            // Check for extra arguments, skipping empty ones, should be NULL
            do {
            next_token=strsep(&line, " \t\n");
            } while (next_token != NULL && *next_token == '\0');

            // More than one argument is not allowed and throws error
            if (cur_token == NULL || next_token != NULL) {
                write(STDERR_FILENO, error_message, strlen(error_message));
                fflush(stderr);
                continue;
            }

            // Check if the path is valid
            if (is_directory(cur_token)) {
                path_command(&cur_path, cur_token);
            }
            else {
                write(STDERR_FILENO, error_message, strlen(error_message));
                fflush(stderr);
                continue;
            }
        }

        else if (strcmp(command, "ls") == 0) {

            // Check if there are any additional arguments, skipping empty ones, should be NULL
            do {
            cur_token=strsep(&line, " \t\n");
            } while (cur_token != NULL && *cur_token == '\0');

            // If there is an additional argument, it's an error
            if (cur_token != NULL) {
                write(STDERR_FILENO, error_message, strlen(error_message));
                fflush(stderr);
                continue;
            }

            // If no additional arguments, run ls
            else {
                ls_command(cur_path);
            }
        }
        else {
            // Make path to game
            char *game_path = malloc(strlen(cur_path) + strlen(command) + 2);
            sprintf(game_path, "%s/%s", cur_path, command);

            // Initialize argument array
            char *game_arg[6];
            game_arg[0] = (char *)command;
            game_arg[1] = NULL;     // For --help/--seed
            game_arg[2] = NULL;     // For seed number
            game_arg[3] = NULL;

            // Check for addtional arguments
            do {
                cur_token = strsep(&line, " \t\n");
            } while (cur_token != NULL && *cur_token == '\0');

            // If no additional arguments, run the game
            if (cur_token == NULL) {
                run_command(game_path, game_arg);
            }

            // If arguement is --help, show help
            else if (strcmp(cur_token, "--help") == 0) {

                // Check for additional arguments, should be NULL
                do {
                    cur_token = strsep(&line, " \t\n");
                } while (cur_token != NULL && *cur_token == '\0');

                // If there are additional arguments, it's an error
                if (cur_token != NULL) {
                    write(STDERR_FILENO, error_message, strlen(error_message));
                    fflush(stderr);
                    continue;
                }

                // If no additional arguments, show help
                else {
                    game_arg[1] = "--help";
                    run_command(game_path, game_arg);
                }
            }

            // If argument is --seed, set the seed
            else if (strcmp(cur_token, "--seed") == 0) {
                game_arg[1] = "--seed";

                // Get the next argument as the seed number
                do {
                    cur_token = strsep(&line, " \t\n");
                } while (cur_token != NULL && *cur_token == '\0');
                
                // Only check if it's a valid number as the executable checks otherwise
                if (cur_token != NULL) {
                    game_arg[2] = cur_token;
                }

                // Check for redirection operators
                do {
                    cur_token = strsep(&line, " \t\n");
                } while (cur_token != NULL && *cur_token == '\0');

                // If it is a redirection operator, redirects input
                if (cur_token != NULL && strcmp(cur_token, "<") == 0) {
                    
                    // Get next argument
                    do {
                        cur_token = strsep(&line, " \t\n");
                    } while (cur_token != NULL && *cur_token == '\0');

                    // Multiple redirections operators or no aregument after operator are not allowed
                    if (cur_token == NULL || (cur_token != NULL &&strcmp(cur_token, "<") == 0)) {
                        write(STDERR_FILENO, error_message, strlen(error_message));
                        fflush(stderr);
                        continue;
                    }

                    // If the argument after operator is not NULL, must be file path
                    else {

                        // Store the file path
                        char *redirection_path = cur_token;

                        // Check if there is extra arguments, should be NULL
                        do {
                            cur_token = strsep(&line, " \t\n");
                        } while (cur_token != NULL && *cur_token == '\0');

                        // If there are extra arguments, it's an error
                        if (cur_token != NULL) {
                            write(STDERR_FILENO, error_message, strlen(error_message));
                            fflush(stderr);
                            continue;
                        }

                        // There is only one argument after the redirection operator
                        else {
                            run_redirect_command(game_path, redirection_path, game_arg);
                        }
                    }
        
                }
                // There is no redirection operator
                else if (cur_token == NULL){
                run_command(game_path, game_arg);
                }

                // Not operator after seed number is error
                else {
                    write(STDERR_FILENO, error_message, strlen(error_message));
                    fflush(stderr);
                    continue;
                }
            }
        }
    }
    free(cur_path);
    free(buffer);
    return 0;
}

// Exit shell after freeing memory
void exit_command(char *cur_path, char *buffer) {
    free(cur_path);
    free(buffer);
    exit(0);
}

// Change current path
void path_command(char **cur_path, const char *path) {
    free(*cur_path);
    *cur_path = strdup(path);
}

void ls_command(char *cur_path) {

    // Iniialize variables
    struct dirent **namelist;
    int num_entries;
    char *arg[3];
    
    arg[0] = NULL;
    arg[1] = "--help";
    arg[2] = NULL;

    // Read directory entries, and sort them
    num_entries = scandir(cur_path, &namelist, NULL, alphasort);

    // Loop through the sorted array
    for (int i = 0; i < num_entries; i++) {

        // Skip hidden files or directories
        if (namelist[i]->d_name[0] == '.') {
            free(namelist[i]);
            continue;
        }

        // Create path to the current entry
        char *path = malloc(strlen(cur_path) + strlen(namelist[i]->d_name) + 2);
        sprintf(path, "%s/%s", cur_path, namelist[i]->d_name);

        // Change the first argument to the current entry's name
        arg[0] = namelist[i]->d_name;

        run_help_command(path, arg);

        // Free the allocated memory and reset the argument
        free(namelist[i]);
        arg[0] = NULL;
    }

    
    free(namelist);
}

// Run exe at path with arguments
void run_command(char *path, char **arg) {
    pid_t pid = fork();
    if (pid == 0) {

        // Run the executable
        execvp(path, arg);

        // If execvp fails, print an error message
        char error_message[30] = "An error has occurred\n";
        write(STDERR_FILENO, error_message, strlen(error_message));
        fflush(stderr);

        // Child has to die if execvp fails or else it runs another shell
        exit(1);
    }

    else if (pid > 0) {
        waitpid(pid, NULL, 0);
        free(path);
    }

    else {
        // If fork fails, print an error message
        char error_message[30] = "An error has occurred\n";
        write(STDERR_FILENO, error_message, strlen(error_message));
        fflush(stderr);
        free(path);
    }
}

// Run executable with redirection and printing
void run_help_command(char *path, char **arg) {
    pid_t pid = fork();
    if (pid == 0) {

        // Open redirection file
        int fd = open("temp.txt", O_WRONLY | O_CREAT | O_TRUNC, 0644);

        // If open fails, print an error message
        if (fd == -1) {
            char error_message[30] = "An error has occurred\n";
            write(STDERR_FILENO, error_message, strlen(error_message));
            fflush(stderr);
            free(path);
            exit(1);
        }

        // Redirect output to the file
        dup2(fd, STDOUT_FILENO);
        close(fd);

        // Run the executable
        execvp(path, arg);

        // Exit if execvp fails
        exit(1);
    }
    else if (pid > 0) {
        waitpid(pid, NULL, 0);
        free(path);
        print_help(arg[0],"temp.txt");
        unlink("temp.txt");
    }

    else {
        // If fork fails, print an error message
        char error_message[30] = "An error has occurred\n";
        write(STDERR_FILENO, error_message, strlen(error_message));
        fflush(stderr);
        free(path);
    }
}

void run_redirect_command(char *path, char *input_path, char **arg) {
    pid_t pid = fork();
    if (pid == 0) {

        // Open redirection file
        int fd = open(input_path, O_RDONLY);

        // If open fails, print an error message
        if (fd == -1) {
            char error_message[30] = "An error has occurred\n";
            write(STDERR_FILENO, error_message, strlen(error_message));
            fflush(stderr);
            free(path);
            exit(1);
        }

        // Redirect input from the file
        dup2(fd, STDIN_FILENO);
        close(fd);

        // Run the executable
        execvp(path, arg);

        // If execvp fails, error and exit
        char error_message[30] = "An error has occurred\n";
        write(STDERR_FILENO, error_message, strlen(error_message));
        fflush(stderr);
        exit(1);
    }
    else if (pid > 0) {
        waitpid(pid, NULL, 0);
        free(path);
    }

    else {
        // If fork fails, print an error message
        char error_message[30] = "An error has occurred\n";
        write(STDERR_FILENO, error_message, strlen(error_message));
        fflush(stderr);
        free(path);
    }
}

// Print help information for a game
void print_help(const char *game_name, const char *filename) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        printf("%s: (empty)\n", game_name);
        return;
    }

    char *line = NULL;
    size_t len = 0;
    ssize_t nread = getline(&line, &len, file);

    while (nread > 0 && (line[nread - 1] == '\n' || line[nread - 1] == '\r')) {
        line[nread - 1] = '\0';
        nread--;
    }

    // If file is empty, execution failed, or nothing was read
    if (nread <= 0) {
        printf("%s: (empty)\n", game_name);
    } else {
        printf("%s: %s\n", game_name, line);
    }

    free(line);
    fclose(file);
}

// Check if a path is a directory
int is_directory(const char *path) {
    DIR *dir = opendir(path);
    if (dir) {
        closedir(dir);
        return 1;
    } else {
        return 0;
    }
}
