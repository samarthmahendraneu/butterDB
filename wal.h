#ifndef WAL_H
#define WAL_H

#include <stdint.h>
#include <stdio.h>

typedef uint64_t lsn_t;

typedef enum {
    LOG_INSERT = 1,
    LOG_CHECKPOINT = 2
} LogType;

#include <pthread.h>

typedef struct {
    int fd;
    pthread_mutex_t lock;
} WAL;

WAL *wal_open(const char *filename);
void wal_close(WAL *wal);

// Appends an INSERT record. Returns the LSN of the generic record.
lsn_t wal_log_insert(WAL *wal, const char *key, const char *value);

// Forces the log to disk up to the given LSN.
void wal_flush(WAL *wal, lsn_t lsn);

// Iterator for recovery
typedef struct {
    LogType type;
    char key[64];   // Hardcoded max len for simplicity matching btree.h
    char value[256];
    lsn_t lsn;
} LogRecord;

int wal_iterator_next(WAL *wal, int *iter_state, LogRecord *out_rec);
void wal_iterator_reset(WAL *wal, int *iter_state);

#endif
