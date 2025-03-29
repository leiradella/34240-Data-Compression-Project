#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "../headers/lz.h"

#define WINDOW_SIZE 4096
#define LOOKAHEAD_SIZE 18
#define MIN_MATCH_LENGTH 3


void compress_lzss(FILE *in, FILE *out) {

    //total buffer size is sliding window size + lookahead size
    uint8_t sliding_window[WINDOW_SIZE] = {0};
    uint8_t lookahead[LOOKAHEAD_SIZE] = {0};

    //initialize the position which tells us how much data is in the sliding window
    size_t bytes_read;
    int position = 0;
    int count = 0;
    int output_index = 0;
    uint8_t flag = 0;
    uint8_t output[16];

    int lookahead_filled = fread(lookahead, 1, LOOKAHEAD_SIZE, in);

    while (lookahead_filled > 0) {
        int best_length = 0;
        int best_offset = 0;

        for (int i = 0; i < position; i++) {
            if (sliding_window[i] == lookahead[0]) {
                //match!!
                int current_length = 1;
                int current_offset = i;
                for (int j = 1; j < lookahead_filled; j++) {
                    if (i+j < position) {
                        if (sliding_window[i+j] == lookahead[j]) {
                            current_length++;
                        }
                        else break;
                    }
                    else break;
                }
                if (current_length > best_length) {
                    best_length = current_length;
                    best_offset = current_offset;
                }
            }
        }
        if (best_length > 15){
            best_length = 15; //max length is 15
        }
        int length = 0;
        if (best_length < MIN_MATCH_LENGTH) { //single
            //here we set the lenght to 1 for the memcpy into the window buffer, set the
            //flag bit to 1 for byte and pass the single byte to the output buffer
            length = 1;
            flag |= (1 << (7 - count));

        } else { //match
            length = best_length;
        }
        if (position + length > WINDOW_SIZE) {
            memmove(sliding_window, sliding_window + length, WINDOW_SIZE - length);
            position = WINDOW_SIZE - length;
        }
        memcpy(sliding_window + position, lookahead, length);
        position += length;
        lookahead_filled -=length;

        //now we place the date in the output buffer
        if (length == 1) {
            output[output_index] = lookahead[0];
            output_index++;
        } else {
            //the 2 output bytes are 12 bits offset 4 bits length
            //first we shift the offset to right 4 times to get just 8 bits into b1
            //then we place the other 4 bits of offset at the start of b2
            //and place the 4 lenght bits at the end of b2
            unsigned int offset = (position - length) - best_offset;
            unsigned int output_length = length;

            uint8_t b1 = (offset >> 4) & 0xFF;
            uint8_t b2 = ((offset & 0x0F) << 4 | (output_length & 0x0F));
            output[output_index] = b1;
            output_index++;
            output[output_index] = b2;
            output_index++;
        }

        //every 8 iterations we write the flag and then the 8 entries into the output file
        if (count == 7) {
            fputc(flag, out);
            fwrite(output, 1, output_index, out);
            flag = 0;
            count = 0;
            output_index = 0;
        } else count++;

        memmove(lookahead, lookahead + length, LOOKAHEAD_SIZE - length);
        lookahead_filled += fread(lookahead + lookahead_filled, 1, LOOKAHEAD_SIZE - lookahead_filled, in);
    }

    //add any partial data left
    if (count > 0) {
        fputc(flag, out);
        fwrite(output, 1, output_index, out);
    }
}

void decompress_lzss(FILE *in, FILE *out) {
    uint8_t flag;
    uint8_t sliding_window[WINDOW_SIZE] = {0};
    int position = 0;

    while (((flag = fgetc(in)) != EOF)) {
        for (int i = 0; i < 8; i++) {
            if (flag & (1 << (7 - i))) { //single byte
                int c = fgetc(in);
                if (c == EOF) return;
                fputc(c, out);

                sliding_window[position] = (uint8_t)c;
                position++;

                if (position >= WINDOW_SIZE) {
                    memmove(sliding_window, sliding_window + 1, WINDOW_SIZE - 1);
                    position = WINDOW_SIZE - 1;
                }
            } else { //match
                int b1 = fgetc(in);
                int b2 = fgetc(in);

                if (b1 == EOF || b2 == EOF) return;
                
                int offset = (b1 << 4) | ((b2 & 0xF0) >> 4);
                int length = b2 & 0x0F;

                if (offset > position) {
                    fprintf(stderr, "Error: Invalid offset %d, position %d\n", offset, position);
                    return;
                }

                int start_position = position - offset;

                for (int j = 0; j < length; j++) {
                    uint8_t c = sliding_window[(start_position + j)];
                    fputc(c, out);
                    sliding_window[position] = c;
                    position++;
                    if (position >= WINDOW_SIZE) {
                        memmove(sliding_window, sliding_window + 1, WINDOW_SIZE - 1);
                        position = WINDOW_SIZE - 1;
                        start_position--;
                    }
                }
            }
        }
    }
}