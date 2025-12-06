#include "os.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>

int CreatePipe(pipe_t new_pipe[2]) { return pipe(new_pipe); }

pid_t CloneProcess() { return fork(); }

int Exec(const char* path, char* argv[]) {
    return execv(path, argv);
}

int LinkFDtoIN(int fd) {
    return dup2(fd, STDIN_FILENO);
}

int LinkFDtoOUT(int fd) {
    return dup2(fd, STDOUT_FILENO);
}

int LinkFDtoERR(int fd) {
    return dup2(fd, STDERR_FILENO);
}

int OpenObject(const char* path, int flags, int mod) {
    return open(path, flags, mod);
}

int WaitProcess() { return wait(NULL); }

int ClosePipe(pipe_t pipe) { return close(pipe); }

int WritePipe(pipe_t pipe, void* buffer, int bytes) {
    return write(pipe, buffer, bytes);
}

int ReadPipe(pipe_t pipe, void* buffer, int bytes) {
    return read(pipe, buffer, bytes);
}