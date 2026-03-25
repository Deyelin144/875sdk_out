#ifndef _STORAGE_H
#define _STORAGE_H

#include <stdint.h>

struct storage;

struct storage *audio_storage_init(const char *path);

void storage_deinit(struct storage *storage);

void storage_enable(struct storage *storage, uint8_t enable);

int storage_write(struct storage *storage,
                           void *data, size_t size);
                           
void storage_enable(struct storage *storage, uint8_t enable);
#endif
