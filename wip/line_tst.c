#include <stdio.h>
#include <stdlib.h>

#define LINE_IMPLEMENTATION
#include "../line.h"

void my_compgen(char* line, size_t point) {
    // not called yet
    (void)line;
    (void)point;
}

int main(void) {
    puts("hi :3");

    line_compgen(my_compgen);
    char* line;
    while (printf(">> "), line = line_read()) {
        printf("echo: '%s'\n", line);
    }
    //line_free(); // crashes

    puts("bye o/");
    return 0;
}
