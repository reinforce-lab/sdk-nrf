/**
 * @file storage.h
 * 
 */
#ifndef _STORAGE_H_
#define _STORAGE_H_

#include <kernel.h>
#include <zephyr/types.h>

void init_storage(void);
int storage_write(uint16_t id, uint8_t *p_src, int len);
int storage_read(uint16_t id, uint8_t *p_dst, int len);

#endif /* _STORAGE_H_ */