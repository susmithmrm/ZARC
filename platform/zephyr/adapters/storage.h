#ifndef ZARC_NATIVE_STORAGE_H
#define ZARC_NATIVE_STORAGE_H
#include <zephyr/fs/nvs.h>
#include "zarc/config_store.h"
#include "zarc/update_journal.h"
#include "zarc/diag_store.h"
/* Mounted NVS partition belongs exclusively to the maintenance worker.
 * Store the explicit byte records, never native C struct padding. */
int zarc_native_config_read(struct nvs_fs *,uint16_t key,zarc_config_store *);
int zarc_native_config_write(struct nvs_fs *,uint16_t key,const zarc_config_store *);
struct zarc_native_update_journal {
 struct nvs_fs *fs;
 uint16_t key;
 zarc_config_store store;
 uint64_t revision;
 bool writable;
};
/* Owner must load/reconcile approved metadata before enabling this writer.
 * A fresh journal may be explicitly initialized on a dedicated key. */
int zarc_native_update_journal_persist(void *,const zarc_update *);
struct zarc_native_diag_store {
 struct nvs_fs *fs;
 uint16_t key;
 uint64_t revision;
 bool writable;
};
/* Call only from the designated serialized maintenance worker. */
int zarc_native_diag_load(struct zarc_native_diag_store *,zarc_diagnostics *);
int zarc_native_diag_persist(struct zarc_native_diag_store *,const zarc_diagnostics *);
#endif
