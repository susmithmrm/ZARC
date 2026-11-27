#ifndef ZARC_NATIVE_SENSOR_SAMPLE_H
#define ZARC_NATIVE_SENSOR_SAMPLE_H
#include <zephyr/device.h>
#include <zephyr/drivers/sensor.h>
#include "zarc/sensor.h"
/* Conversion from Zephyr's whole + micro fractional unit to a declared
 * decimal scale. Truncates toward zero; rejects overflow, never clamps. */
bool zarc_native_sensor_convert(const struct sensor_value *, int8_t, int32_t *);
/* Call from the sole device owner in thread context. The caller allocates
 * a monotonic timestamp and sequence; driver execution needs target timing
 * evidence. On an acquisition fault publish invalid quality, not a reading. */
int zarc_native_sensor_poll(zarc_sensor *, const struct device *,
                            enum sensor_channel, uint64_t boot,
                            uint64_t sequence, uint64_t tick);
#endif
