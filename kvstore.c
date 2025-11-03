//
// Created by Samarth Mahendra on 11/3/25.
//

#include <string.h>
#include "kvstore.h"


void kv_init(KVStore *store) {
    store->size = 0;
    for (int i = 0; i < MAX_TABLE; i++)
        store->table[i].in_use = 0;
}

int kv_put(KVStore *store, const char *key, const char *value) {
    // update if exists
    for (int i = 0; i < MAX_TABLE; i++) {
        if (store->table[i].in_use && strcmp(store->table[i].key, key) == 0) {
            strncpy(store->table[i].value, value, MAX_VAL_LEN);
            return 0;
        }
    }

    // insert new
    for (int i = 0; i < MAX_TABLE; i++) {
        if (!store->table[i].in_use) {
            strncpy(store->table[i].key, key, MAX_KEY_LEN);
            strncpy(store->table[i].value, value, MAX_VAL_LEN);
            store->table[i].in_use = 1;
            store->size++;
            return 0;
        }
    }
    return -1; // table full
}

char *kv_get(KVStore *store, const char *key) {
    for (int i = 0; i < MAX_TABLE; i++) {
        if (store->table[i].in_use && strcmp(store->table[i].key, key) == 0)
            return store->table[i].value;
    }
    return NULL;
}

int kv_del(KVStore *store, const char *key) {
    for (int i = 0; i < MAX_TABLE; i++) {
        if (store->table[i].in_use && strcmp(store->table[i].key, key) == 0) {
            store->table[i].in_use = 0;
            store->size--;
            return 0;
        }
    }
    return -1;
}

