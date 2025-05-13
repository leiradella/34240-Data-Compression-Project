#include "../headers/utils.h"

#include <stdlib.h>
#include <string.h>
#include <strings.h>

FILE *open_file(const char *filename, const char *mode) {
    FILE *file = fopen(filename, mode);
    if (file == NULL) {
        printf("error opening file: %s\n", filename);
        exit(1);
    }
    return file;
}

ProgramOptions parse_arguments(int argc, char *argv[]) {
    ProgramOptions options;
    //initialize the options to default values
    options.compress = 0;
    options.decompress = 0;
    options.use_huffman = 1; //use huffman by default
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
                                printf("Input filename not set\n");
                                exit(1);
                            }
                        } else {
                            printf("Input filename not set\n");
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
        printf("Input filename not set\n");
        exit(1);
    }

    //check if there is a valid option set
    if (options.compress == 0 && options.decompress == 0) {
        printf("Either compress or decompress must be set\n");
        exit(1);
    }

    //check if both compress and decompress are set (not allowed)
    if (options.compress == 1 && options.decompress == 1) {
        printf("Cannot set both compress and decompress\n");
        exit(1);
    }

    if (options.use_lossy == 1 && options.quantization_factor == 0) {
        options.quantization_factor = 10;
    }

    return options;
}

void copy_header(FILE *in, FILE *out, char *filename) {
    //first find out the file type
    char *dot_extension = strrchr(filename, '.');

    if (dot_extension == NULL) {
        printf("Invalid file type\n");
        exit(1);
    }

    //move 1 byte forward to skip the dot
    char *extension = dot_extension + 1;

    if (strcasecmp(extension, "bmp") == 0) {
        int header_size = 54; 
        for (int i = 0; i < header_size; i++) {
            int c = fgetc(in);
            if (c == EOF) {
                printf("Error reading BMP header\n");
                exit(1);
            }
            fputc(c, out);
        }
    } else if (strcasecmp(extension, "tiff") == 0) {
        int header_size = 8;
        for (int i = 0; i < header_size; i++) {
            int c = fgetc(in);
            if (c == EOF) {
                printf("Error reading TIFF header\n");
                exit(1);
            }
            fputc(c, out);
        }
    } else {
        printf("Only .bmp and .tiff are supported for lossy compression\n");
        exit(1);
    }
}

void quantize(FILE *in, FILE *out, char *filename, int quantization_factor) {
    int c;

    //copy header
    copy_header(in, out, filename);

    //loop through the input file and quantize the data
    while ((c = fgetc(in)) != EOF) {
        //quantize the data
        c = (c / quantization_factor) * quantization_factor;
        fputc(c, out);
    }
}



