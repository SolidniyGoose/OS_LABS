#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define LINE_BUFFER 512

int main() {
    char line[LINE_BUFFER];
    
    while(fgets(line, LINE_BUFFER, stdin)) {
        int len = strlen(line);
        if(len > 0 && line[len-1] == '\n') {
            line[len-1] = '\0';
            len--;
        }
        
        if(len > 0 && (line[len-1] == '.' || line[len-1] == ';')) {
            printf("%s\n", line);
        } else {
            fprintf(stderr, "Invalid line: '%s' (must end with '.' or ';')\n", line);
        }
    }
    
    return 0;
}