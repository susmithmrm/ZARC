#ifndef ZARC_CONFIG_STORE_H
#define ZARC_CONFIG_STORE_H
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#define ZARC_CONFIG_MAX 128u
#define ZARC_CONFIG_HEADER 20u
#define ZARC_CONFIG_SLOT (ZARC_CONFIG_HEADER + ZARC_CONFIG_MAX + 1u)
typedef struct { uint8_t slots[2][ZARC_CONFIG_SLOT]; } zarc_config_store;
typedef struct { uint32_t schema; uint64_t revision; size_t length; uint8_t bytes[ZARC_CONFIG_MAX]; } zarc_config;
typedef enum { ZARC_CONFIG_OK, ZARC_CONFIG_UNAVAILABLE, ZARC_CONFIG_INVALID, ZARC_CONFIG_CONFLICT } zarc_config_status;
/* Desktop byte-array model only. CRC is corruption detection, not authentication.
 * Approval and migration occur outside this model. Stage never changes the
 * selected committed record; commit marker is written last. */
zarc_config_status zarc_config_load(const zarc_config_store *, uint32_t schema, zarc_config *);
zarc_config_status zarc_config_stage(zarc_config_store *, uint32_t schema, uint64_t revision, const uint8_t *, size_t, size_t *slot);
zarc_config_status zarc_config_commit(zarc_config_store *, size_t slot, uint32_t schema, uint64_t revision);
/* Migration is an explicit reviewed mapping, never a silent fallback or
 * default. Caller owns authorization, quiescence and durable commit. */
typedef bool (*zarc_config_migration_fn)(void *,const zarc_config *,uint8_t *,size_t,size_t *);
typedef struct {
 uint32_t source_schema,target_schema;
 uint64_t source_revision,target_revision;
 size_t slot,length;
 uint8_t bytes[ZARC_CONFIG_MAX];
 bool prepared;
} zarc_config_migration;
zarc_config_status zarc_config_prepare_migration(zarc_config_store *,uint32_t source,uint32_t target,
 uint64_t revision,bool authorized,zarc_config_migration_fn,void *,zarc_config_migration *);
zarc_config_status zarc_config_commit_migration(zarc_config_store *,const zarc_config_migration *);
#endif
