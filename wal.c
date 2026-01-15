#include "wal.h"
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <stdio.h>

#define WAL_FILE "butterdb.wal"

WAL *wal_open(const char *filename) {
    const char *path = filename ? filename : WAL_FILE;
    int fd = open(path, O_RDWR | O_CREAT | O_APPEND, S_IWUSR | S_IRUSR);
    if (fd == -1) {
        perror("WAL open failed");
        return NULL;
    }
    WAL *wal = malloc(sizeof(WAL));
    wal->fd = fd;
    return wal;
}

void wal_close(WAL *wal) {
    close(wal->fd);
    free(wal);
}

static lsn_t current_lsn = 0; 

lsn_t wal_log_insert(WAL *wal, const char *key, const char *value) {
    if (current_lsn == 0) {
        struct stat st;
        fstat(wal->fd, &st);
        current_lsn = st.st_size; 
    }

    uint32_t type = LOG_INSERT;
    uint32_t klen = strlen(key) + 1;
    uint32_t vlen = strlen(value) + 1;
    
    // Simple verification LSN = offset
    lsn_t this_lsn = lseek(wal->fd, 0, SEEK_END);
    
    write(wal->fd, &this_lsn, sizeof(lsn_t));
    write(wal->fd, &type, sizeof(uint32_t));
    write(wal->fd, &klen, sizeof(uint32_t));
    write(wal->fd, &vlen, sizeof(uint32_t));
    write(wal->fd, key, klen);
    write(wal->fd, value, vlen);
    
    return this_lsn;
}

void wal_flush(WAL *wal, lsn_t lsn) {
    fsync(wal->fd);
}

void wal_iterator_reset(WAL *wal, int *iter_state) {
    *iter_state = 0;
}

int wal_iterator_next(WAL *wal, int *iter_state, LogRecord *out_rec) {
    off_t offset = *iter_state;
    off_t file_len = lseek(wal->fd, 0, SEEK_END);
    
    if (offset >= file_len) return 0;
    
    // Read Header
    if (pread(wal->fd, &out_rec->lsn, sizeof(lsn_t), offset) != sizeof(lsn_t)) return 0;
    offset += sizeof(lsn_t);
    
    uint32_t type, klen, vlen;
    pread(wal->fd, &type, sizeof(uint32_t), offset); offset += sizeof(uint32_t);
    pread(wal->fd, &klen, sizeof(uint32_t), offset); offset += sizeof(uint32_t);
    pread(wal->fd, &vlen, sizeof(uint32_t), offset); offset += sizeof(uint32_t);
    
    out_rec->type = type;
    
    // Read Key
    uint32_t read_k = klen > 64 ? 64 : klen;
    pread(wal->fd, out_rec->key, read_k, offset);
    // Ensure null term if we truncated? (Not strictly needed if we treat as opaque, but for strings yes)
    out_rec->key[63] = '\0';
    offset += klen; // Advance by real length
    
    // Read Value
    uint32_t read_v = vlen > 256 ? 256 : vlen;
    pread(wal->fd, out_rec->value, read_v, offset);
    out_rec->value[255] = '\0';
    offset += vlen; // Advance by real length
    
    *iter_state = offset;
    return 1;
}
