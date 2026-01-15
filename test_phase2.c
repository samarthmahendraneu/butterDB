#include "btree.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

void test_basic_persistence() {
    printf("[Test] Basic Persistence...\n");
    // Clean up
    unlink("test_basic.dat");
    unlink("test_basic.wal"); // Default name is butterdb.wal, need to support custom or rename
    // btree_open takes filename, wal uses butterdb.wal hardcoded in my impl?
    // Let's check wal.c. wal_open(filename) uses it if provided. 
    // btree_open passes NULL to wal_open.
    // So it uses default.
    unlink("butterdb.wal");

    BTree *tree = btree_open("test_basic.dat");
    btree_insert(tree, "key1", "val1");
    btree_insert(tree, "key2", "val2");
    
    char buf[256];
    assert(btree_search(tree, 0, "key1", buf) == 1);
    assert(strcmp(buf, "val1") == 0);
    
    btree_close(tree);

    // Reopen
    tree = btree_open("test_basic.dat");
    memset(buf, 0, 256);
    assert(btree_search(tree, 0, "key1", buf) == 1);
    assert(strcmp(buf, "val1") == 0);
    assert(btree_search(tree, 0, "key2", buf) == 1);
    assert(strcmp(buf, "val2") == 0);
    
    btree_close(tree);
    printf("[PASS] Basic Persistence\n");
}

void test_crash_recovery() {
    printf("[Test] Crash Recovery (WAL)...\n");
    unlink("test_crash.dat");
    unlink("butterdb.wal"); 

    int pid = fork();
    if (pid == 0) {
        // Child: Writes and Crashes
        BTree *tree = btree_open("test_crash.dat");
        btree_insert(tree, "crash_key", "crash_val");
        
        // Force WAL flush to ensure it hits disk (simulating "committed" txn)
        // Since we don't have explicit commit API, we assume internal flush or OS sync.
        // But btree_insert calls wal_log_insert which uses `write`. 
        // We need `fsync` to be sure for this test.
        // Hack: access internal wal to flush? No API.
        // But `write` to file usually persists if we wait a bit or if OS is nice.
        // PROPER WAY: Add wal_flush call or expose it.
        // For test, we will modify `btree.h` to expose WAL? It is exposed in struct.
        wal_flush(tree->wal, 0); // Flush everything
        
        printf("Child: Write done. Crashing (skipping close)...\n");
        _exit(0); // Exit without calling cleanup/btree_close
    }
    
    // Parent
    wait(NULL); // Wait for child to die
    
    printf("Parent: Recovering...\n");
    BTree *tree = btree_open("test_crash.dat");
    // Should verify modification exists via WAL replay
    char buf[256];
    int found = btree_search(tree, 0, "crash_key", buf);
    
    if (found) {
        printf("Found value: %s\n", buf);
        assert(strcmp(buf, "crash_val") == 0);
        printf("[PASS] Crash Recovery\n");
    } else {
        printf("[FAIL] Value not found after recovery.\n");
        printf("Debug: Did WAL flush?\n");
        exit(1);
    }
    btree_close(tree);
}

int main() {
    test_basic_persistence();
    test_crash_recovery();
    return 0;
}
