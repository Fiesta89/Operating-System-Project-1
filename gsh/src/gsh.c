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

void exit_command(char *cur_path, char *buffer);

void path_command(char **cur_path, const char *path);

void ls_command();

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
    // Check if the argument is a directory
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
        // Parse the input
        char *line = buffer;
        command = strsep(&line, " \t\n");

        // Process first token as the command
        if (strcmp(command, "exit") == 0) {
            // Check if there are any additional arguments
            cur_token=strsep(&buffer, " \t\n");
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
            cur_token = strsep(&buffer, " \t\n");
            next_token = strsep(&buffer, " \t\n");
            // More than one argument is not allowed
            if (cur_token == NULL ||next_token != NULL) {
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
            }
        }

        else if (strcmp(command, "ls") == 0) {
            ls_command();
        }
        else {
            continue;
        }
    }
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
    // Implementation for ls command
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
