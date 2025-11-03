//
// Created by Samarth Mahendra on 11/3/25.
//

#ifndef KVSTORE_H
#define KVSTORE_H

#define MAX_KEY_LEN 64
#define MAX_VAL_LEN 256
#define MAX_TABLE   1024


typedef struct {
    char key[MAX_KEY_LEN];
    char value[MAX_VAL_LEN];
    int in_use; // 0 = free, 1 = used
} KVPair;

typedef struct {
    kv_entry_t entries[MAX_TABLE];
    int size;
} KVStore;

void kvstore_init(KVStore *store);

int kvstore_set(KVStore *store, const char *key, const char *value);

int kvstore_get(KVStore *store, const char *key, char *value_out);

int kvstore_delete(KVStore *store, const char *key);

#endif // KVSTORE_H
