#ifndef BTREE_H
#define BTREE_H

#include <stdio.h>
#include <stdint.h>
#include <pthread.h>
#include "pager.h"
#include "wal.h"

#define ORDER 4
#define MAX_KEY_LEN 64
#define MAX_VAL_LEN 256
#define PAGE_SIZE 4096

typedef struct {
    uint32_t is_leaf;
    uint32_t num_keys;
    char keys[ORDER - 1][MAX_KEY_LEN];
    char values[ORDER - 1][MAX_VAL_LEN];
    uint32_t children[ORDER];
} BTreeNode;

typedef struct {
    uint32_t page_id;
    uint8_t data[PAGE_SIZE]; 
    pthread_mutex_t lock;    
    int is_dirty;
    int ref_count;
    lsn_t page_lsn; // NEW: Max LSN that updated this page
} Page;

#define MAX_CACHE 64

typedef struct {
    Pager *pager;
    WAL *wal; // NEW: WAL Handle
    uint32_t root_page_id;
    
    Page pages[MAX_CACHE];
    pthread_mutex_t cache_lock;
} BTree;

BTree *btree_open(const char *filename);
void btree_close(BTree *tree);
int btree_search(BTree *tree, uint32_t page_id, const char *key, char *out_value);
int btree_insert(BTree *tree, const char *key, const char *value);

#endif
