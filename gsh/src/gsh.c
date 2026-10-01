#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>

void exit_command();

int is_directory(const char *path);

int main(int argc, char *argv[]) {

    // Variables
    char error_message[30] = "An error has occurred\n";
    char *buffer = NULL;
    size_t bufsize = 0;
    
    // Check argument counts
    if (argc != 2) {
        write(STDERR_FILENO, error_message, strlen(error_message)); 
        exit(1);
    }
    // Check if the argument is a directory
    if (!is_directory(argv[1])) {
        write(STDERR_FILENO, error_message, strlen(error_message));
        exit(1);
    }

    
    while(1) {
        printf("gsh> ");
        getline(&buffer, &bufsize, stdin);
        if (strcmp(buffer, "exit\n") == 0) {
            exit_command();
        }
        else {
            printf("User said: %s", buffer);
        }
    }
    return 0;
}

void exit_command() {
    exit(0);
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