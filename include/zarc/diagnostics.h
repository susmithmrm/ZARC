#ifndef ZARC_DIAGNOSTICS_H
#define ZARC_DIAGNOSTICS_H
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#define ZARC_FAULT_LIMIT 16u
#define ZARC_DIAG_VERSION 1u
typedef enum { ZARC_DIAG_UNKNOWN, ZARC_DIAG_GOOD, ZARC_DIAG_STALE, ZARC_DIAG_FAULT } zarc_diag_quality;
typedef enum { ZARC_DIAG_INFO, ZARC_DIAG_WARNING, ZARC_DIAG_ERROR, ZARC_DIAG_CRITICAL } zarc_diag_severity;
typedef struct {
 uint32_t version, code, service, occurrences;
 uint64_t epoch, generation, timestamp, correlation, config_digest, build_digest;
 zarc_diag_quality quality; zarc_diag_severity severity;
} zarc_fault;
typedef struct { zarc_fault records[ZARC_FAULT_LIMIT]; size_t count; uint32_t rejected; bool critical_overflow; } zarc_diagnostics;
typedef enum { ZARC_DIAG_STORED, ZARC_DIAG_COALESCED, ZARC_DIAG_FULL, ZARC_DIAG_INVALID } zarc_diag_result;
/* No silent eviction. Saturation is explicit; critical overflow remains latched
 * until the owning maintenance policy replaces or resets the journal. */
zarc_diag_result zarc_diag_record(zarc_diagnostics *, const zarc_fault *);
bool zarc_diag_read(const zarc_diagnostics *, size_t index, zarc_fault *);
#endif
