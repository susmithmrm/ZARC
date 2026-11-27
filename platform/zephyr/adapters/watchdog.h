#ifndef ZARC_NATIVE_WATCHDOG_H
#define ZARC_NATIVE_WATCHDOG_H
#include <zephyr/device.h>
#include "zarc/health.h"
struct zarc_native_watchdog {
 const struct device *device;
 int channel;
 bool armed;
};
/* Sole supervisor owner, approved timeout/reset policy required.
 * No watchdog feed on stale or failed health evaluation. */
int zarc_native_watchdog_arm(struct zarc_native_watchdog *, const struct device *, uint32_t max_ms);
int zarc_native_watchdog_supervise(struct zarc_native_watchdog *, zarc_health *, uint64_t now);
#endif
