#ifndef ZARC_NATIVE_STORAGE_H
#define ZARC_NATIVE_STORAGE_H
#include <zephyr/fs/nvs.h>
#include "zarc/config_store.h"
/* Mounted NVS partition belongs exclusively to the maintenance worker.
 * Store the explicit byte records, never native C struct padding. */
int zarc_native_config_read(struct nvs_fs *,uint16_t key,zarc_config_store *);
int zarc_native_config_write(struct nvs_fs *,uint16_t key,const zarc_config_store *);
#endif
