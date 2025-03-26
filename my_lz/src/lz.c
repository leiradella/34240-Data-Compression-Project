#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../headers/lz.h"

#define WINDOW_SIZE 4096
#define LOOKAHEAD_SIZE 18
#define MIN_MATCH 3


void compress_lzss(FILE *in, FILE *out) {
    // TODO: Read input into buffer
    // Search for longest match in the sliding window
    // Write flag byte (bitmask) and data (literals or (offset, length))

    //total buffer size is sliding window size + lookahead size
    unsigned char sliding_window[WINDOW_SIZE] = {0};
    unsigned char lookahead[LOOKAHEAD_SIZE] = {0};

    //initialize the position which tells us how much data is in the sliding window
    size_t bytes_read;
    int position = 0;
    unsigned char flag = 0;
    int count = 0;

    while ((bytes_read = fread(lookahead, 1, LOOKAHEAD_SIZE, in)) > 0) {
        // printf("%ld", bytes_read);
        int best_offset = 0;
        int best_length = 0;

        for (int i = 0; i < position; i++) {
            if (sliding_window[i] == lookahead[0]) {
                //match!!
                int current_length = 1;
                int current_offset = i;
                for (int j = 1; j < bytes_read; j++) {
                    if (i+j < position) {
                        if (sliding_window[i+j] == lookahead[j]) {
                            current_length++;
                        }
                        else { break; }
                    }
                    else { break; }
                }
                if (current_length > best_length) {
                    best_length = current_length;
                    best_offset = current_offset;
                }
            }
        }
        if (best_length < MIN_MATCH) { //store data literally
            int length = 1;
            flag |= (1 << (7 - count));
            if (position + length > WINDOW_SIZE) {
                //this shifts the data to open space in the end to insert the new match
                memmove(sliding_window, sliding_window + length, WINDOW_SIZE - length);
            }
            memcpy(sliding_window + position, lookahead, length);
            position += length;
        }
        else { //store match
            //check if the sliding_window gets full
            if (position + best_length > WINDOW_SIZE) {
                //this shifts the data to open space in the end to insert the new match
                memmove(sliding_window, sliding_window + best_length, WINDOW_SIZE - best_length);
            }
            memcpy(sliding_window + position, lookahead, best_length);
            position += best_length;
        }

        if (count == 8) {
            fputc(flag, out);
            //printf("0x%02X", flag);
            flag = 0;
            count = 0;
        } else { count++; }
        printf("%s\n", lookahead);
    }
}

void decompress_lzss(FILE *in, FILE *out) {
    // TODO: Read flag byte, then interpret next 8 items accordingly
    // If literal, write to output
    // If (offset, length), copy from sliding window
}