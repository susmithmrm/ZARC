#ifndef ZARC_CALIBRATION_H
#define ZARC_CALIBRATION_H
#include "zarc/config_store.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/* Candidate versioned sensor calibration record; provenance is an allocated
 * engineering record ID, not proof of a trusted signer or approval. */
#define ZARC_CALIBRATION_VERSION 1u
#define ZARC_CALIBRATION_SCHEMA 1u
#define ZARC_CALIBRATION_WIRE_SIZE 53u

typedef struct {
 uint32_t version;
 uint32_t sensor_id, unit_id, calibration_id;
 int8_t scale10;
 int32_t raw_min, raw_max;
 int32_t minimum, maximum;
 int32_t gain_num;
 uint32_t gain_den;
 int32_t offset;
 uint64_t provenance_id;
} zarc_calibration_record;

bool zarc_calibration_validate(const zarc_calibration_record *);
bool zarc_calibration_encode(const zarc_calibration_record *,uint8_t *,size_t);
bool zarc_calibration_decode(const uint8_t *,size_t,zarc_calibration_record *);
/* engineering_value = round(raw * gain_num / gain_den) + offset, in the
 * record's unit and scale10. The ratio uses ties-away-from-zero rounding.
 * Converts only the declared sensor and raw range; output is unchanged on failure. */
bool zarc_calibration_apply(const zarc_calibration_record *,uint32_t sensor_id,
                            int32_t raw_value,int32_t *engineering_value);
/* These wrappers use the existing CRC and two-slot commit model. The caller
 * supplies the authorization decision; hardware persistence remains target-owned. */
zarc_config_status zarc_calibration_load(const zarc_config_store *,uint32_t sensor_id,
                                         zarc_calibration_record *,uint64_t *revision);
zarc_config_status zarc_calibration_stage(zarc_config_store *,
                                          const zarc_calibration_record *,uint64_t revision,
                                          bool authorized,size_t *slot);
zarc_config_status zarc_calibration_commit(zarc_config_store *,size_t slot,
                                           uint64_t revision,bool authorized);
#endif
