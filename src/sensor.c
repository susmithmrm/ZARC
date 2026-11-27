#include "zarc/sensor.h"
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
