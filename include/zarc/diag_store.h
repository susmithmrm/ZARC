#ifndef ZARC_DIAG_STORE_H
#define ZARC_DIAG_STORE_H
#include "zarc/diagnostics.h"
#define ZARC_DIAG_STORE_HEADER 28u
#define ZARC_DIAG_STORE_RECORD 72u
#define ZARC_DIAG_STORE_SIZE (ZARC_DIAG_STORE_HEADER+ZARC_FAULT_LIMIT*ZARC_DIAG_STORE_RECORD)
/* Fixed-size little-endian journal snapshot. CRC detects corruption, not
 * authorization or rollback. Caller owns atomic storage and revision policy. */
bool zarc_diag_store_encode(const zarc_diagnostics *,uint64_t revision,uint8_t *,size_t);
bool zarc_diag_store_decode(const uint8_t *,size_t,zarc_diagnostics *,uint64_t *revision);
#endif
