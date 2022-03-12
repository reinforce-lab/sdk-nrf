/**
 * @file storage.c
 * 
 */
#include "storage.h"

#include <zephyr.h>
#include <kernel.h>
#include <net/buf.h>
#include <sys/byteorder.h>

#include <drivers/flash.h>
#include <storage/flash_map.h>
#include <fs/nvs.h>

#include <logging/log.h>
LOG_MODULE_DECLARE(hci_uart);

K_FIFO_DEFINE(storage_fifo);
NET_BUF_POOL_DEFINE(storage_pool, 12, 400, 0, NULL);

#define NVS_SECTOR_SIZE  (DT_PROP(DT_CHOSEN(zephyr_flash), erase_block_size))
#define NVS_SECTOR_COUNT 96
#define NVS_STORAGE_OFFSET FLASH_AREA_OFFSET(storage)

static struct nvs_fs fs = {
	.sector_size  = NVS_SECTOR_SIZE,
	.sector_count = NVS_SECTOR_COUNT,
	.offset       = NVS_STORAGE_OFFSET,
};

/**
 * Public method
 */
void init_storage(void)
{
	int ret;

    LOG_INF("init_storage:");
    LOG_INF("  sector size 0x%08x, count: 0x%08x.", NVS_SECTOR_SIZE, NVS_SECTOR_COUNT);    
    LOG_INF("  storage_offset: 0x%08x.", NVS_STORAGE_OFFSET);

	ret = nvs_init(&fs, DT_CHOSEN_ZEPHYR_FLASH_CONTROLLER_LABEL);
	if (ret < 0) {
		LOG_INF("Cannot initialize NVS.");
	}
}

int storage_write(uint16_t id, uint8_t *p_src, int len)
{
    LOG_INF("storage_write(), id: %d, len: %d.", id, len);

    int ret;

    ret = nvs_write(&fs, id, (void *)p_src, len);
    LOG_INF("nvs_write(), len: %d. ret: %d.", len, ret);

    return ret;
}

int storage_read(uint16_t id, uint8_t *p_dst, int len)
{
    LOG_INF("storage_read(), id: %d, len: %d.", id, len);

	int ret = nvs_read(&fs, id, (void *)p_dst, len);
    if(ret < 0) {
        LOG_ERR("nvs_read(), error: %d.", ret);
    }
    return ret;
}
