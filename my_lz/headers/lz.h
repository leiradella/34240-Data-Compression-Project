#ifndef LZSS_H
#define LZSS_H

void compress_lzss(FILE *in, FILE *out);
void decompress_lzss(FILE *in, FILE *out);

#endif