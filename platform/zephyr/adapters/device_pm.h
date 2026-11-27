#ifndef ZARC_NATIVE_DEVICE_PM_H
#define ZARC_NATIVE_DEVICE_PM_H
#include <zephyr/device.h>
#include <stdbool.h>
struct zarc_native_pm {const struct device *dev;bool held,fault;};
/* Exclusive, serialized maintenance/device owner. These calls can sleep and
 * must not run under shared state locks or in ISR. PM is not physical rail state. */
int zarc_native_pm_open(struct zarc_native_pm *,const struct device *);
int zarc_native_pm_acquire(struct zarc_native_pm *);
int zarc_native_pm_release(struct zarc_native_pm *);
#endif
