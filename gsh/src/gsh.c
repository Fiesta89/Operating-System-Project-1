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

void exit_command(char *cur_path, char *buffer);

void path_command(char **cur_path, const char *path);

void ls_command();

void run_command(char *path, char **arg, char *error_message);

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
        do {
        command = strsep(&line, " \t\n");
        } while (command != NULL && *command == '\0');

        // If no command is found, continue to the next iteration
        if (command == NULL) {
            continue;
        }

        // Process first token as the command
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
            ls_command();
        }
        else {
            // Make path to game
            char *game_path = malloc(strlen(cur_path) + strlen(command) + 2);
            sprintf(game_path, "%s/%s", cur_path, command);
            // Make argument array
            char *game_arg[2];
            game_arg[0] = (char *)command;
            game_arg[1] = NULL;
            run_command(game_path, game_arg, error_message);
        }
    }
    free(cur_path);
    free(buffer);
    return 0;
}

void exit_command(char *cur_path, char *buffer) {
    free(cur_path);
    free(buffer);
    exit(0);
}

void path_command(char **cur_path, const char *path) {
    free(*cur_path);
    *cur_path = strdup(path);
}

void ls_command() {
    return;
}

void run_command(char *path, char **arg, char *error_message) {
    pid_t pid = fork();
    if (pid == 0) {
        // Make argument array for execvp
        execvp(path, arg);
        // If execvp fails, print an error message
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
        write(STDERR_FILENO, error_message, strlen(error_message));
        fflush(stderr);
    }
}

int is_directory(const char *path) {
    DIR *dir = opendir(path);
    if (dir) {
        closedir(dir);
        return 1;
    } else {
        return 0;
    }
}
