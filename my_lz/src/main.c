#include <stdio.h>
#include <stdlib.h>
#include "../headers/lz.h"

int main(int argc, char *argv[]) {
    if (argc < 4) {
        printf("Usage: %s [c|d] input output\n", argv[0]);
        return 1;
    }

    FILE *in = fopen(argv[2], "r");

    if (in == NULL) {
        char msg[256];
        snprintf(msg, 256, "File error %s", argv[2]);
        perror(msg);
        return 1;
    }

    FILE *out = fopen(argv[3], "w");

    if (out == NULL) {
        char msg[256];
        snprintf(msg, 256, "File error %s", argv[3]);
        perror(msg);
        return 1;
    }

    if (argv[1][0] == 'c') {
        compress_lzss(in, out);
    } else if (argv[1][0] == 'd') {
        decompress_lzss(in, out);
    } else {
        printf("Unknown mode '%s'\n", argv[1]);
    }

    fclose(in);
    fclose(out);
    return 0;
}
