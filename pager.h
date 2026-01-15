#ifndef PAGER_H
#define PAGER_H

#include <stdint.h>
#include <stdio.h>

#define PAGE_SIZE 4096

typedef struct {
    int fd;
    uint32_t num_pages;
} Pager;

Pager *pager_open(const char *filename);
void pager_read(Pager *pager, uint32_t page_num, void *out_data);
void pager_write(Pager *pager, uint32_t page_num, const void *in_data);
void pager_close(Pager *pager);
uint32_t pager_get_num_pages(Pager *pager);
uint32_t pager_allocate_page(Pager *pager);

#endif
