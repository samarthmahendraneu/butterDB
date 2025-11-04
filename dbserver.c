//
// ButterDB — Phase 1: B-tree based persistent KV store
//
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "btree.h"

#define PORT 9090
#define BUF_SIZE 1024
#define DB_FILE "btree.dat"

int main() {
    // ---- Initialize B-tree ----
    BTree *btree = btree_open(DB_FILE);
    printf("ButterDB Phase 1 (B-tree) running on port %d...\n", PORT);

    // ---- Network Setup ----
    int server_fd, client_fd;
    struct sockaddr_in addr;
    char buffer[BUF_SIZE];

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 3) < 0) {
        perror("listen failed");
        exit(EXIT_FAILURE);
    }

    printf("Waiting for client connections...\n");

    while (1) {
        socklen_t addrlen = sizeof(addr);
        client_fd = accept(server_fd, (struct sockaddr *)&addr, &addrlen);
        if (client_fd < 0) {
            perror("accept failed");
            continue;
        }

        printf("Client connected.\n");
        while (1) {
            memset(buffer, 0, BUF_SIZE);
            int read_bytes = read(client_fd, buffer, BUF_SIZE - 1);
            if (read_bytes <= 0) break;

            char cmd[10], key[MAX_KEY_LEN], val[MAX_VAL_LEN];
            int args = sscanf(buffer, "%s %s %s", cmd, key, val);

            if (strcmp(cmd, "PUT") == 0 && args == 3) {
                btree_insert(btree, key, val);
                write(client_fd, "OK\n", 3);
            }
            else if (strcmp(cmd, "GET") == 0 && args >= 2) {
                char result[MAX_VAL_LEN];
                if (btree_search(btree, btree->root_offset, key, result))
                    dprintf(client_fd, "%s\n", result);
                else
                    write(client_fd, "NOT_FOUND\n", 10);
            }
            else if (strcmp(cmd, "DEL") == 0 && args >= 2) {
                // Phase 1 doesn’t implement delete yet
                write(client_fd, "DEL_NOT_SUPPORTED\n", 18);
            }
            else if (strcmp(cmd, "EXIT") == 0) {
                break;
            }
            else {
                write(client_fd, "INVALID\n", 8);
            }
        }
        printf("Client disconnected.\n");
        close(client_fd);
    }

    btree_close(btree);
    close(server_fd);
    return 0;
}
