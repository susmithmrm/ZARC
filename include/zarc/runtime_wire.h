#ifndef ZARC_RUNTIME_WIRE_H
#define ZARC_RUNTIME_WIRE_H
#include "zarc/runtime.h"
/* Authenticated, serialized transport owner supplies this metadata. The
 * codec does not establish trust, transit age, identity or scope. The scope
 * callback is mandatory for every data operation (including enumeration).
 * No raw hardware operation is exposed. */
typedef struct {
 uint64_t principal,age;
 bool age_known;
 void *context;
 bool (*allow)(void *,uint64_t principal,uint8_t operation,uint32_t endpoint);
} zarc_wire_peer;
bool zarc_runtime_dispatch(zarc_runtime *,const zarc_wire_peer *,const zarc_frame *,uint64_t now,
 bool reserved,zarc_frame *result,zarc_controller_effect *effect);
#endif
