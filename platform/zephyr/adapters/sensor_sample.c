#include "sensor_sample.h"
#include <errno.h>
#include <limits.h>
bool zarc_native_sensor_convert(const struct sensor_value *v, int8_t scale, int32_t *out)
{
    if (!v || !out || scale < -9 || scale > 9 ||
        v->val2 <= -1000000 || v->val2 >= 1000000) return false;
    int64_t n = (int64_t)v->val1 * INT64_C(1000000) + v->val2;
    int exponent = -6 - scale;
    while (exponent > 0) {
        if (n > INT64_MAX / 10 || n < INT64_MIN / 10) return false;
        n *= 10; --exponent;
    }
    if (exponent < 0) {
        int64_t divisor = 1;
        while (exponent < 0) { divisor *= 10; ++exponent; }
        n /= divisor;
    }
    if (n < INT32_MIN || n > INT32_MAX) return false;
    *out = (int32_t)n;
    return true;
}
int zarc_native_sensor_poll(zarc_sensor *s, const struct device *dev,
                            enum sensor_channel channel, uint64_t boot,
                            uint64_t sequence, uint64_t tick)
{
    if (!s || !s->initialized || boot != s->latest.boot ||
        sequence <= s->latest.sequence || tick < s->last_tick ||
        tick < s->latest.sample_tick) return -EINVAL;
    int rc = -ENODEV;
    int32_t value = 0;
    struct sensor_value raw = {0};
    if (dev && device_is_ready(dev)) {
        rc = sensor_sample_fetch(dev);
        if (!rc) rc = sensor_channel_get(dev, channel, &raw);
        if (!rc && (!zarc_native_sensor_convert(&raw, s->config.scale10, &value) ||
            value < s->config.minimum || value > s->config.maximum)) rc = -ERANGE;
    }
    if (!zarc_sensor_publish(s, boot, sequence, tick, tick, value,
                             rc ? ZARC_SAMPLE_FAULT : ZARC_SAMPLE_VALID)) return -EINVAL;
    return rc;
}
