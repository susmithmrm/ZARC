#include "watchdog.h"
#include <zephyr/drivers/watchdog.h>
#include <errno.h>
int zarc_native_watchdog_arm(struct zarc_native_watchdog *w,const struct device *dev,uint32_t max_ms){
 if(!w||!dev||!max_ms||!device_is_ready(dev))return -EINVAL;
 if(w->armed)return -EALREADY;
 struct wdt_timeout_cfg cfg={.window={.min=0,.max=max_ms},.flags=WDT_FLAG_RESET_SOC};
 int channel=wdt_install_timeout(dev,&cfg);if(channel<0)return channel;
 int ret=wdt_setup(dev,0);
 if(ret<0)return ret; /* device may require reset after failed setup */
 *w=(struct zarc_native_watchdog){.device=dev,.channel=channel,.armed=true};return 0;
}
int zarc_native_watchdog_supervise(struct zarc_native_watchdog *w,zarc_health *h,uint64_t now){
 if(!w||!w->armed)return -EINVAL;
 zarc_health_snapshot snapshot;
 if(!zarc_health_check(h,now,&snapshot)||!snapshot.required_healthy)return -EAGAIN;
 return wdt_feed(w->device,w->channel);
}
