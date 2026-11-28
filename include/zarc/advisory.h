#ifndef ZARC_ADVISORY_H
#define ZARC_ADVISORY_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef enum {
 ZARC_ADVISORY_OK,
 ZARC_ADVISORY_INVALID,
 ZARC_ADVISORY_UNAVAILABLE,
 ZARC_ADVISORY_CAPACITY
} zarc_advisory_result;

/* Caller zero-initializes before init and serializes every operation. Storage remains unavailable until the
 * owner enables it. Disabling clears the published length. This is bounded
 * caller-owned storage for opaque bytes only: no controller, domain, transport,
 * or effect interface, no dynamic allocation, and no analytics implementation. */
typedef struct {
 uint8_t *storage;
 size_t capacity;
 size_t length;
 bool available;
} zarc_advisory_store;

bool zarc_advisory_init(zarc_advisory_store *,uint8_t *storage,size_t capacity);
zarc_advisory_result zarc_advisory_set_available(zarc_advisory_store *,bool available);
zarc_advisory_result zarc_advisory_publish(zarc_advisory_store *,const uint8_t *data,size_t length);
zarc_advisory_result zarc_advisory_read(const zarc_advisory_store *,uint8_t *out,size_t capacity,size_t *length);

#endif
