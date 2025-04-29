#include <stdio.h>
#include <stdlib.h>
#include "../headers/lz.h"

#define MAX_FILENAME_SIZE 256

//./mylz c filename

int main(int argc, char *argv[]) {
    //check if the minimum number of arguments is met
    if (argc < 3) {
        printf("Usage: %s [c|d] filename\n", argv[0]);
        return 1;
    }

    //get the filename and open the according files depending on whether it is
    //compressing (c): input = tests/files/filename, output = tests/compressed/filename
    //decompressing (d): input = tests/compressed/filename, output = tests/decompressed/filename
    char in_filepath[MAX_FILENAME_SIZE];
    char out_filepath[MAX_FILENAME_SIZE];
    if (argv[1][0] == 'c') {
        snprintf(in_filepath, MAX_FILENAME_SIZE, "tests/files/%s", argv[2]);
        snprintf(out_filepath, MAX_FILENAME_SIZE, "tests/compressed/%s", argv[2]);
    } else if (argv[1][0] == 'd') {
        snprintf(in_filepath, MAX_FILENAME_SIZE, "tests/compressed/%s", argv[2]);
        snprintf(out_filepath, MAX_FILENAME_SIZE, "tests/decompressed/%s", argv[2]);
    } else {
        printf("Unknown mode '%s'\n", argv[1]);
        return 1;
    }

    //open the input file and check for any errors
    FILE *in = fopen(in_filepath, "rb");
    if (in == NULL) {
        char msg[MAX_FILENAME_SIZE];
        snprintf(msg, MAX_FILENAME_SIZE + 19, "INPUT: File error %s", in_filepath);
        perror(msg);
        return 1;
    }
    //open the output file and check for any errors
    FILE *out = fopen(out_filepath, "wb");
    if (out == NULL) {
        char msg[MAX_FILENAME_SIZE];
        snprintf(msg, MAX_FILENAME_SIZE + 19, "OUTPUT: File error %s", out_filepath);
        perror(msg);
        fclose(in);
        return 1;
    }

    // check whether compressing or decompressing and call the appropriate function
    // closes if its not 'c' or 'd'
    if (argv[1][0] == 'c') {
        compress_lzss(in, out);
    } else if (argv[1][0] == 'd') {
        decompress_lzss(in, out);
    } else {
        printf("Unknown mode '%s'\n", argv[1]);
        fclose(in);
        fclose(out);
        return 1;
    }

    fclose(in);
    fclose(out);
    return 0;
}
