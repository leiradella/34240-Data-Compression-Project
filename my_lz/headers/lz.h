#ifndef LZSS_H
#define LZSS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

void find_best_match(char* sliding_window, char* lookahead, int position, int lookahead_filled, int *best_length, int *best_offset);
void compress_lzss(FILE *in, FILE *out);
void decompress_lzss(FILE *in, FILE *out);

#endif