#ifndef ZARC_UPDATE_JOURNAL_H
#define ZARC_UPDATE_JOURNAL_H
#include "zarc/update.h"
#include "zarc/config_store.h"
#define ZARC_UPDATE_JOURNAL_SCHEMA UINT32_C(0x5a550001)
/* Explicit versioned bytes, never struct padding. CRC detects corruption only;
 * verified boot/image identity and approved immutable policy are independent. */
zarc_config_status zarc_update_journal_stage(zarc_config_store *,uint64_t revision,const zarc_update *,size_t *slot);
zarc_config_status zarc_update_journal_commit(zarc_config_store *,size_t slot,uint64_t revision);
zarc_config_status zarc_update_journal_load(const zarc_config_store *,const zarc_update_policy *,zarc_update *,uint64_t *revision);
#endif
