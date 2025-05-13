#include "../headers/huffman.h"

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

void generate_codes(HuffmanNode *node, HuffmanCode *codes, uint32_t code, uint8_t length) {
    //check if we are at a leaf node
    //if so, we set the code and length of the symbol
    if (node->left == NULL && node->right == NULL) {
        codes[node->symbol].code = code;
        codes[node->symbol].length = length;
        return;
    }

    //if not, we continue down the tree
    //making sure to update the codes and lengths
    generate_codes(node->left, codes, (code << 1), length + 1);
    generate_codes(node->right, codes, (code << 1) | 1, length + 1);
}

void compress_huffman(FILE *in, FILE *out) {
    //frequency table
    int frequency[256] = {0};
    //byte value
    uint8_t c;

    //read the input file byte by byte and count the frequency of each byte
    while (fread(&c, 1, 1, in) == 1) {
        frequency[c]++;
    }

    //write the frequency table to the output file
    fwrite(frequency, sizeof(int), 256, out);

    //huffman nodes for each frequency
    HuffmanNode *nodes[256];
    //number of nodes
    int num_nodes = 0;

    //now we go through all the frequencies and create huffman nodes
    for (int i = 0; i < 256; i++) {
        if (frequency[i] > 0) {
            HuffmanNode *node = (HuffmanNode *)malloc(sizeof(HuffmanNode));
            node->symbol = i;
            node->freq = frequency[i];
            node->left = NULL;
            node->right = NULL;
            nodes[num_nodes++] = node;
        }
    }

    while (num_nodes > 1) {
        int min1 = 0;
        int min2 = 1;
        //find the two nodes with the smallest frequency
        for (int i = 2; i < num_nodes; i++) {
            if (nodes[i]->freq < nodes[min1]->freq) {
                min2 = min1;
                min1 = i;
            } else if (nodes[i]->freq < nodes[min2]->freq) {
                min2 = i;
            }
        }

        //now we create a new node with the two smallest nodes as children
        HuffmanNode *parent = (HuffmanNode *)malloc(sizeof(HuffmanNode));
        parent->symbol = 0;
        parent->freq = nodes[min1]->freq + nodes[min2]->freq;
        parent->left = nodes[min2];
        parent->right = nodes[min1];

        nodes[min1] = parent;
        nodes[min2] = nodes[num_nodes - 1];
        num_nodes--;
    }

    //now we have a single node which is the root of the huffman tree
    HuffmanNode *root = nodes[0];

    //generate the huffman codes
    HuffmanCode codes[256] = {0};
    generate_codes(root, codes, 0, 0);

    //now we reread the input file and write the huffman codes to the output
    rewind(in);


    uint8_t buffer[8];
    uint8_t buffer_filled = fread(buffer, 1, 8, in);

    uint8_t output_byte = 0;
    uint8_t bit_count = 0;

    while (buffer_filled > 0) {
        for (int i = 0; i < buffer_filled; i++) {
            HuffmanCode current_code = codes[buffer[i]];

            for (int j = 0; j < current_code.length; j++) {
                //add the bit to the buffer
                output_byte = (output_byte << 1) | ((current_code.code >> (current_code.length - j - 1)) & 1);
                bit_count++;
                
                //if we have 8 bits, we write them to the output byte
                if (bit_count == 8) {
                    fputc(output_byte, out);
                    output_byte = 0;
                    bit_count = 0;
                }
            }
        }        
        buffer_filled = fread(buffer, 1, 8, in);
    }

    //write leftover bits with 0s to the right
    if (bit_count > 0) {
        output_byte <<= (8 - bit_count);
        fputc(output_byte, out);
    }
}

HuffmanNode *build_huffman_tree(int *freq) {
    HuffmanNode *nodes[256];
    int num_nodes = 0;

    //create a node for every non 0 frequency
    for (int i = 0; i < 256; i++) {
        if (freq[i] > 0) {
            nodes[num_nodes] = (HuffmanNode *)malloc(sizeof(HuffmanNode));
            nodes[num_nodes]->symbol = i;
            nodes[num_nodes]->freq = freq[i];
            nodes[num_nodes]->left = NULL;
            nodes[num_nodes]->right = NULL;
            num_nodes++;
        }
    }

    //combine all the nodes to form the huffman tree
    while (num_nodes > 1) {

        //find the two nodes with the smallest frequency
        int min1 = 0;
        int min2 = 1;
        for (int i = 2; i < num_nodes; i++) {
            if (nodes[i]->freq < nodes[min1]->freq) {
                min2 = min1;
                min1 = i;
            } else if (nodes[i]->freq < nodes[min2]->freq) {
                min2 = i;
            }
        }

        //create a new node with the two smallest nodes as children
        HuffmanNode *parent = (HuffmanNode *)malloc(sizeof(HuffmanNode));
        parent->symbol = 0;
        parent->freq = nodes[min1]->freq + nodes[min2]->freq;
        parent->left = nodes[min2];
        parent->right = nodes[min1];

        //replace the two smallest nodes with the new node
        nodes[min1] = parent;
        nodes[min2] = nodes[num_nodes - 1];
        num_nodes--;
    }

    return nodes[0];
}

void decompress_huffman(FILE *in, FILE *out) {
    //read the frequency table from the input file
    int freq[256];
    if (fread(freq, sizeof(int), 256, in) != 256) {
        fprintf(stderr, "Failed to read frequency table.\n");
        return;
    }

    HuffmanNode *root = build_huffman_tree(freq);

    //now we read the huffman codes from the input file
    uint8_t buffer[8];
    uint8_t buffer_filled = fread(buffer, 1, 8, in);

    uint8_t byte_count = 0;
    int bit_count = 7;
    HuffmanNode *current_node = root;

    while (buffer_filled > 0) {
        uint8_t byte = buffer[byte_count];
        int bit = (byte >> bit_count) & 1;

        if (bit == 0) {
            current_node = current_node->left;
        } else {
            current_node = current_node->right;
        }

        if (current_node->left == NULL && current_node->right == NULL) {
            //we have reached a leaf node, so we write the symbol to the output file
            fputc(current_node->symbol, out);
            current_node = root;
        }

        bit_count--;
        if (bit_count < 0) {
            bit_count = 7;
            byte_count++;
        }

        if (byte_count >= buffer_filled) {
            byte_count = 0;
            buffer_filled = fread(buffer, 1, 8, in);
        }
        
    }
}