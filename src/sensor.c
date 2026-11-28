#include "zarc/sensor.h"
#include <limits.h>
bool zarc_sensor_init(zarc_sensor *s, const zarc_sensor_config *c,
                      uint64_t boot, uint64_t now)
{
    if (!s || !c || !c->id || !c->unit_id || !c->calibration_id ||
        c->scale10 < -9 || c->scale10 > 9 || c->minimum > c->maximum ||
        !c->max_age || !boot) return false;
    zarc_sensor next = {0};
    next.config = *c;
    next.latest.id = c->id;
    next.latest.unit_id = c->unit_id;
    next.latest.calibration_id = c->calibration_id;
    next.latest.scale10 = c->scale10;
    next.latest.boot = boot;
    next.latest.quality = ZARC_SAMPLE_UNKNOWN;
    next.last_tick = now;
    next.initialized = true;
    *s = next;
    return true;
}
bool zarc_sensor_publish(zarc_sensor *s, uint64_t boot, uint64_t sequence,
                         uint64_t sample_tick, uint64_t now, int32_t value,
                         zarc_sample_quality quality)
{
    if (!s || !s->initialized || boot != s->latest.boot ||
        now < s->last_tick || sample_tick > now ||
        sample_tick < s->latest.sample_tick || sequence <= s->latest.sequence ||
        (quality != ZARC_SAMPLE_VALID && quality != ZARC_SAMPLE_FAULT))
        return false;
    if (quality == ZARC_SAMPLE_VALID &&
        (value < s->config.minimum || value > s->config.maximum ||
         now - sample_tick >= s->config.max_age)) return false;
    s->latest.sequence = sequence;
    s->latest.sample_tick = sample_tick;
    s->latest.value = quality == ZARC_SAMPLE_VALID ? value : 0;
    s->latest.quality = quality;
    s->last_tick = now;
    return true;
}
bool zarc_sensor_snapshot(zarc_sensor *s, uint64_t now, zarc_sensor_sample *out)
{
    if (!s || !s->initialized || !out || now < s->last_tick) return false;
    s->last_tick = now;
    *out = s->latest;
    if (out->quality == ZARC_SAMPLE_VALID &&
        now - out->sample_tick >= s->config.max_age)
        out->quality = ZARC_SAMPLE_STALE;
    if (out->quality != ZARC_SAMPLE_VALID) out->value = 0;
    return true;
}

static uint64_t magnitude_i64(int64_t value)
{
    /* Avoid negating INT64_MIN, which is not representable as int64_t. */
    return value < 0 ? (uint64_t)(-(value + 1)) + UINT64_C(1) : (uint64_t)value;
}

bool zarc_sensor_quantize_ratio(int64_t numerator, int64_t denominator,
                                int32_t *out)
{
    if (!out || !denominator ||
        (numerator == INT64_MIN && denominator == -1)) return false;

    int64_t quotient = numerator / denominator;
    int64_t remainder = numerator % denominator;
    uint64_t remainder_magnitude = magnitude_i64(remainder);
    uint64_t denominator_magnitude = magnitude_i64(denominator);
    uint64_t half_threshold = denominator_magnitude / UINT64_C(2) +
                              denominator_magnitude % UINT64_C(2);

    /* C integer division truncates toward zero. Adjust only when the exact
     * remainder is at least half the divisor; ties therefore round away from
     * zero for both signs. The threshold expression cannot overflow. */
    if (remainder_magnitude >= half_threshold) {
        bool positive = (numerator < 0) == (denominator < 0);
        quotient += positive ? INT64_C(1) : -INT64_C(1);
    }
    if (quotient < INT32_MIN || quotient > INT32_MAX) return false;

    *out = (int32_t)quotient;
    return true;
}
