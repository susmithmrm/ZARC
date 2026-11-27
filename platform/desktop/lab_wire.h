#ifndef ZARC_LAB_WIRE_H
#define ZARC_LAB_WIRE_H
#include "zarc/connect.h"
/* Reference fixture payloads, explicitly little-endian.
 * Domain request: id:u32, desired:u8, validity:u32.
 * Snapshot request: id:u32. Result: status:u8; snapshot result appends
 * value:u8, quality:u8, generation:u64. No production authentication. */
static inline void lab_put(uint8_t *p,size_t n,uint64_t v){
 for(size_t i=0;i<n;++i)p[i]=(uint8_t)(v>>(8u*i));
}
static inline uint64_t lab_get(const uint8_t *p,size_t n){
 uint64_t v=0;for(size_t i=0;i<n;++i)v|=(uint64_t)p[i]<<(8u*i);return v;
}
#endif
