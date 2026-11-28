#ifndef ZARC_SENSOR_H
#define ZARC_SENSOR_H
#include <stdbool.h>
#include <stdint.h>

/* Single-owner numeric sample contract. Values use the declared engineering
 * unit and integer scale (value * 10^scale10). No driver or scheduler facade.
 * This is a benign reference service, not a safety-qualified sensor library. */
typedef enum {
    ZARC_SAMPLE_UNKNOWN, ZARC_SAMPLE_VALID, ZARC_SAMPLE_STALE, ZARC_SAMPLE_FAULT
} zarc_sample_quality;
typedef struct {
    uint32_t id, unit_id, calibration_id;
    int8_t scale10;
    int32_t minimum, maximum;
    uint64_t max_age;
} zarc_sensor_config;
typedef struct {
    uint32_t id, unit_id, calibration_id;
    int8_t scale10;
    int32_t value;
    uint64_t boot, sequence, sample_tick;
    zarc_sample_quality quality;
} zarc_sensor_sample;
typedef struct {
    zarc_sensor_config config;
    zarc_sensor_sample latest;
    uint64_t last_tick;
    bool initialized;
} zarc_sensor;
bool zarc_sensor_init(zarc_sensor *, const zarc_sensor_config *, uint64_t boot, uint64_t now);
/* A rejected sample does not alter state. Invalid physical data is represented
 * by an explicit FAULT sample rather than clamping or inventing a reading. */
bool zarc_sensor_publish(zarc_sensor *, uint64_t boot, uint64_t sequence,
                         uint64_t sample_tick, uint64_t now, int32_t value,
                         zarc_sample_quality quality);
bool zarc_sensor_snapshot(zarc_sensor *, uint64_t now, zarc_sensor_sample *);
/* Convert an exact signed ratio to the sensor's int32 engineering scale.
 * Uses integer arithmetic only, truncation plus nearest rounding with ties
 * away from zero. Returns false for a zero denominator, null output, or a
 * rounded result outside int32_t; output is unchanged on failure. No FPU,
 * floating-point rounding mode, or intermediate multiplication is involved. */
bool zarc_sensor_quantize_ratio(int64_t numerator, int64_t denominator,
                                int32_t *out);
#endif
