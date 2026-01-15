#include "pager.h"
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <string.h>

Pager *pager_open(const char *filename) {
    int fd = open(filename, O_RDWR | O_CREAT, S_IWUSR | S_IRUSR);
    if (fd == -1) {
        perror("Unable to open file");
        return NULL;
    }

    off_t file_len = lseek(fd, 0, SEEK_END);
    
    Pager *pager = malloc(sizeof(Pager));
    pager->fd = fd;
    pager->num_pages = (uint32_t)(file_len / PAGE_SIZE);

    return pager;
}

void pager_read(Pager *pager, uint32_t page_num, void *out_data) {
    off_t offset = page_num * PAGE_SIZE;
    ssize_t res = pread(pager->fd, out_data, PAGE_SIZE, offset);
    if (res == -1) {
        perror("Error reading page");
        exit(EXIT_FAILURE);
    }
    if (res < PAGE_SIZE) {
        // Zero out the remaining part (or all if res=0)
        memset((char*)out_data + res, 0, PAGE_SIZE - res);
    }
}

void pager_write(Pager *pager, uint32_t page_num, const void *in_data) {
    off_t offset = page_num * PAGE_SIZE;
    ssize_t res = pwrite(pager->fd, in_data, PAGE_SIZE, offset);
    if (res == -1) {
        perror("Error writing page");
        exit(EXIT_FAILURE);
    }
}

void pager_close(Pager *pager) {
    close(pager->fd);
    free(pager);
}

uint32_t pager_get_num_pages(Pager *pager) {
    return pager->num_pages;
}

uint32_t pager_allocate_page(Pager *pager) {
    return pager->num_pages++;
}
