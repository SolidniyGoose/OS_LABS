#include "os.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUFFER_SIZE 512

int main() {
    int pipe_to_child[2];
    int pipe_from_child[2];
    pid_t child_pid;
    char filename[BUFFER_SIZE];
    int file_fd;

    printf("Enter output filename: ");
    if(!fgets(filename, BUFFER_SIZE, stdin)) {
        perror("Input error");
        exit(1);
    }
    filename[strcspn(filename, "\n")] = 0;

    file_fd = OpenObject(filename, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if(file_fd == -1) {
        perror("File opening failed");
        exit(1);
    }

    if(CreatePipe(pipe_to_child) == -1 || CreatePipe(pipe_from_child) == -1) {
        perror("Pipe creation failed");
        exit(1);
    }

    child_pid = CloneProcess();
    if(child_pid == -1) {
        perror("Process creation failed");
        exit(1);
    }

    if(child_pid == 0) {
        ClosePipe(pipe_to_child[1]);
        ClosePipe(pipe_from_child[0]);
        
        LinkFDtoIN(pipe_to_child[0]);
        ClosePipe(pipe_to_child[0]);

        LinkFDtoOUT(file_fd);
        ClosePipe(file_fd);

        LinkFDtoERR(pipe_from_child[1]);
        ClosePipe(pipe_from_child[1]);

        Exec("./child", (char*[]){"child", NULL});
        perror("Execution failed");
        exit(1);
    } else {
        ClosePipe(pipe_to_child[0]);
        ClosePipe(pipe_from_child[1]);
        ClosePipe(file_fd);

        char buffer[BUFFER_SIZE];
        
        while(fgets(buffer, BUFFER_SIZE, stdin)) {
            WritePipe(pipe_to_child[1], buffer, strlen(buffer));
        }
        ClosePipe(pipe_to_child[1]);

        int bytes_read;
        while((bytes_read = ReadPipe(pipe_from_child[0], buffer, BUFFER_SIZE-1)) > 0) {
            buffer[bytes_read] = '\0';
            printf("Error from child: %s", buffer);
        }
        ClosePipe(pipe_from_child[0]);
        
        WaitProcess();
    }
    return 0;
}