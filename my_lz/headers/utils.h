#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>

#define MAX_FILENAME_SIZE 256

//used to store the command line arguments for the program
typedef struct {
    int compress;
    int decompress;
    int use_huffman;
    int use_lossy;
    int quantization_factor;
    char *input_filename;
} ProgramOptions;

FILE *open_file(const char *filename, const char *mode); 

ProgramOptions parse_arguments(int argc, char *argv[]);

void quantize(FILE *in, FILE *out, int quantization_factor);

#endif
