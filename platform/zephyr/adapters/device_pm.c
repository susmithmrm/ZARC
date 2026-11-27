#include "device_pm.h"
#include <zephyr/pm/device_runtime.h>
#include <errno.h>
int zarc_native_pm_open(struct zarc_native_pm *p,const struct device *dev){
 if(!p||!dev||!device_is_ready(dev))return -EINVAL;
 if(p->dev)return -EALREADY;
 int ret=pm_device_runtime_enable(dev);
 if(ret<0)return ret;
 if(!pm_device_runtime_is_enabled(dev))return -ENOTSUP;
 if(pm_device_runtime_usage(dev)!=0)return -EBUSY;
 p->dev=dev;p->held=false;p->fault=false;return 0;
}
int zarc_native_pm_acquire(struct zarc_native_pm *p){
 if(!p||!p->dev||p->fault)return -EINVAL;
 if(p->held)return -EALREADY;
 if(pm_device_runtime_usage(p->dev)!=0){p->fault=true;return -EBUSY;}
 int ret=pm_device_runtime_get(p->dev);
 if(ret<0){p->fault=true;return ret;}
 p->held=true;
 if(pm_device_runtime_usage(p->dev)!=1){p->fault=true;return -EIO;}
 return 0;
}
int zarc_native_pm_release(struct zarc_native_pm *p){
 if(!p||!p->dev||!p->held)return -EINVAL;
 int ret=pm_device_runtime_put(p->dev);
 if(ret<0){p->fault=true;return ret;}
 p->held=false;
 if(pm_device_runtime_usage(p->dev)!=0){p->fault=true;return -EIO;}
 return 0;
}
