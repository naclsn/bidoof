#include <stdio.h>
#include <stdlib.h>

#define LINE_IMPLEMENTATION
#include "../line.h"

char const* const words[] = {
    "one",
    "two",
    "three",
    "four",
    "five",
    "six",
    "seven",
    "eight",
    "nine",
    "ten",
    "twenty",
    "thirty",
    //..
};
#define words_count  (sizeof(words)/sizeof(words[0]))

char const* const* my_compgen(char const* line, size_t point) {
    char const* pfx = line+point-1;
    while ('a' <= *pfx && *pfx <= 'z' && line < pfx) pfx--;
    size_t pfx_len = *pfx ? line+point-pfx : 0;

    static char const* ret[words_count+1];
    {
        char const** head = &ret[0];
        for (size_t k = 0; k < words_count; k++) {
            if (0 == memcmp(words[k], pfx, pfx_len))
                *head++ = words[k] + pfx_len;
        }
        *head = NULL;
    }
    return ret;
}

int main(void) {
    puts("hi :3");

    line_compgen(my_compgen, NULL);
    char* line;
    while (printf(">> "), line = line_read()) {
        if (0 == strcmp("history", line)) {
            size_t o;
            char** h = line_hist(&o);
            for (o-- ;o; --o)
                printf(" %3zu %s\n", o, h[o]);
        } else printf("echo: '%s'\n", line);
    }
    line_free();

    puts("bye o/");
    return 0;
}