#ifndef HUFFMAN_H
#define HUFFMAN_H

#include <stdio.h>
#include <stdint.h>


typedef struct HuffmanNode {
    uint8_t symbol;
    int freq;
    struct HuffmanNode *left;
    struct HuffmanNode *right;
} HuffmanNode;

typedef struct HuffmanCode {
    uint32_t code;
    uint8_t length;
} HuffmanCode;

void generate_codes(HuffmanNode *node, HuffmanCode *codes, uint32_t code, uint8_t length);
void compress_huffman(FILE *in, FILE *out);

HuffmanNode *build_huffman_tree(int *freq);
void decompress_huffman(FILE *in, FILE *out);

#endif