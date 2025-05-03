#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../headers/lz.h"
#include "../headers/huffman.h"
#include "../headers/utils.h"

int main(int argc, char *argv[]) {
    //parse the command line arguments
    ProgramOptions options = parse_arguments(argc, argv);

    printf("Compress: %d\n", options.compress);
    printf("Decompress: %d\n", options.decompress);
    printf("Use Huffman: %d\n", options.use_huffman);
    printf("Use Lossy: %d\n", options.use_lossy);
    printf("Quantization Factor: %d\n", options.quantization_factor);
    printf("Input Filename: %s\n", options.input_filename);
    
    //open the input file
    FILE *in = open_file(options.input_filename, "rb");

    //quantization
    if (options.use_lossy == 1) {
        if (options.compress == 0) {
            perror("Quantization can only be used with compression. Use -c option.");
            exit(1);
        }

        FILE *temp = open_file("temp.qz", "wb");
        //quantize the input file
        quantize(in, temp, options.quantization_factor);
        fclose(in);
        fclose(temp);

        //reopen in as the quantized file
        in = open_file("temp.qz", "rb");
    }

    //compression and decompression
    if (options.compress == 1) {
        if (options.use_huffman == 1) {
            //create temp file
            FILE *temp = open_file("temp.lz", "wb");

            //compress the input using lzss
            compress_lzss(in, temp);
            fclose(in);
            fclose(temp);

            //create output file
            char output_filename[MAX_FILENAME_SIZE];
            snprintf(output_filename, MAX_FILENAME_SIZE, "%s.lz.huff", options.input_filename);
            FILE *out = open_file(output_filename, "wb");

            temp = open_file("temp.lz", "rb");

            //compress the lzss file using huffman
            compress_huffman(temp, out);

            //close the files
            fclose(temp);
            fclose(out);

            //remove temp file
            remove("temp.lz");
        } else {
            //create output file
            char output_filename[MAX_FILENAME_SIZE];
            snprintf(output_filename, MAX_FILENAME_SIZE, "%s.lz", options.input_filename);
            FILE *out = open_file(output_filename, "wb");

            //compress the input using lzss
            compress_lzss(in, out);

            //close the files
            fclose(in);
            fclose(out);
        }
    } else if (options.decompress == 1) {
        //first we get the file extension to determine the decompression method
        char *dot_extension = strrchr(options.input_filename, '.');
        if (dot_extension == NULL) {
            perror("Invalid file extension for decompression. Expected .lz or .lz.huff");
            exit(1);
        }

        //if its just .lz we decompress using lzss
        if (strcmp(dot_extension, ".lz") == 0) {
            //remove the extension
            dot_extension[0] = '\0';

            //now create the output file
            FILE *out = open_file(options.input_filename, "wb");

            //decompress the input using lzss
            decompress_lzss(in, out);
            fclose(in);
            fclose(out);
        } else if (strcmp(dot_extension, ".huff") == 0) {
            //remove the .huff extension
            dot_extension[0] = '\0';
            //get the .lz extension
            dot_extension = strrchr(options.input_filename, '.');
            //remove it as well
            dot_extension[0] = '\0';

            //create a temp file for the huffman decompression
            FILE *temp = open_file("temp.lz", "wb");

            //decompress the input using huffman and lzss
            decompress_huffman(in, temp);

            //close the temp file
            fclose(temp);

            //now create the output file
            FILE *out = open_file(options.input_filename, "wb");       

            //open the temp file for reading
            temp = open_file("temp.lz", "rb");
            
            //decompress the temp file using lzss
            decompress_lzss(temp, out);

            //close the files
            fclose(in);
            fclose(temp);
            fclose(out);

            //remove the temp file
            remove("temp.lz");
        }
    } else {
        perror("Invalid option. Use -c for compression or -d for decompression.");
        return 1;
    }

    if (options.use_lossy == 1) {
        //remove the quantized file
        remove("temp.qz");
    }
    return 0;
}
