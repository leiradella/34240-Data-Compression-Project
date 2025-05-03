#include "../headers/utils.h"

#include <stdlib.h>

FILE *open_file(const char *filename, const char *mode) {
    FILE *file = fopen(filename, mode);
    if (file == NULL) {
        char error_message[MAX_FILENAME_SIZE];
        snprintf(error_message, MAX_FILENAME_SIZE, "Error opening file: %s", filename);
        perror(error_message);
        exit(1);
    }
    return file;
}

ProgramOptions parse_arguments(int argc, char *argv[]) {
    ProgramOptions options;
    //initialize the options to default values
    options.compress = 0;
    options.decompress = 0;
    options.use_huffman = 1; // use huffman by default
    options.use_lossy = 0;
    options.quantization_factor = 0;
    options.input_filename = NULL;

    //loop through the arguments and set the options accordingly
    for (int i = 1; i < argc; i++) {
        if (argv[i][0] == '-') {
            for (int j = 1; argv[i][j] != '\0'; j++) {
                switch (argv[i][j]) {
                    //compress
                    case 'c': options.compress = 1; break;
                    //decompress
                    case 'd': options.decompress = 1; break;
                    //input file
                    case 'i': 
                        if (i+1 < argc) {
                            if (argv[i+1][0] != '-') {
                                options.input_filename = argv[i+1];
                            } else {
                                perror("Input filename not set\n");
                                exit(1);
                            }
                        } else {
                            perror("Input filename not set\n");
                            exit(1);
                        }
                        break;
                    //no huffman
                    case 'n': if (argv[i][j+1] == 'h') { options.use_huffman = 0; j++; } break;
                    //lossy
                    case 'l':
                        options.use_lossy = 1; 
                        if (atoi(argv[i+1]) > 0) {
                            options.quantization_factor = atoi(argv[i+1]);
                        }
                        break;
                    default:
                        printf("Unknown option '%c' in %s\n", argv[i][j], argv[i]);
                        break;
                }
            }
        }
    }

    //check if the input filename is set
    if (options.input_filename == NULL) {
        perror("Input filename not set\n");
        exit(1);
    }

    //check if there is a valid option set
    if (options.compress == 0 && options.decompress == 0) {
        perror("Either compress or decompress must be set\n");
        exit(1);
    }

    //check if both compress and decompress are set (not allowed)
    if (options.compress == 1 && options.decompress == 1) {
        perror("Cannot set both compress and decompress\n");
        exit(1);
    }

    if (options.use_lossy == 1 && options.quantization_factor == 0) {
        options.quantization_factor = 10;
    }

    return options;
}

void quantize(FILE *in, FILE *out, int quantization_factor) {
    int c;

    //loop through the input file and quantize the data
    while ((c = fgetc(in)) != EOF) {
        //quantize the data
        c = (c / quantization_factor) * quantization_factor;
        fputc(c, out);
    }
}



