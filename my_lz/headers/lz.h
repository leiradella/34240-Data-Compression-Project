#ifndef LZSS_H
#define LZSS_H

#define WINDOW_SIZE 4096
#define LOOKAHEAD_SIZE 18
#define MIN_MATCH 3

void compress_lzss(FILE *in, FILE *out);
void decompress_lzss(FILE *in, FILE *out);

#endif