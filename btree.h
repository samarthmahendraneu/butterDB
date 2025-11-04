//
// Created by Samarth Mahendra on 11/4/25.
//
#ifndef BTREE_H
#define BTREE_H

#include <stdio.h>

#define ORDER 4                 // max children per node
#define MAX_KEY_LEN 64
#define MAX_VAL_LEN 256
#define PAGE_SIZE 4096

typedef struct {
    int is_leaf;
    int num_keys;
    char keys[ORDER - 1][MAX_KEY_LEN];
    char values[ORDER - 1][MAX_VAL_LEN];  // used only in leaf nodes
    long children[ORDER];                 // file offsets of children
} BTreeNode;

typedef struct {
    FILE *fp;
    long root_offset;
} BTree;

BTree *btree_open(const char *filename);
void btree_close(BTree *tree);
int btree_search(BTree *tree, long offset, const char *key, char *out_value);
int btree_insert(BTree *tree, const char *key, const char *value);

#endif

