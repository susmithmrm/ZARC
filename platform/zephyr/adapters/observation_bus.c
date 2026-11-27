#include "observation_bus.h"
#include <zephyr/kernel.h>
#include <zephyr/sys/atomic.h>
#include <zephyr/zbus/zbus.h>
#include <errno.h>
K_MSGQ_DEFINE(zarc_observations,sizeof(zarc_observation_event),4,8);
static atomic_t overflow;
static void copy_observation(const struct zbus_channel *chan){
 const zarc_observation_event *event=zbus_chan_const_msg(chan);
 if(k_msgq_put(&zarc_observations,event,K_NO_WAIT)!=0)atomic_set(&overflow,1);
}
ZBUS_LISTENER_DEFINE(zarc_observer,copy_observation);
ZBUS_CHAN_DEFINE(zarc_observation_channel,zarc_observation_event,NULL,NULL,
 ZBUS_OBSERVERS(zarc_observer),ZBUS_MSG_INIT(0));
int zarc_native_observation_publish(const zarc_observation_event *event){
 if(!event||!event->sequence||!event->observation.boot||!event->observation.id)return -EINVAL;
 if(atomic_get(&overflow))return -EOVERFLOW;
 int ret=zbus_chan_pub(&zarc_observation_channel,event,K_NO_WAIT);
 if(ret!=0){atomic_set(&overflow,1);return ret;}
 return atomic_get(&overflow)?-EOVERFLOW:0;
}
int zarc_native_observation_read(zarc_observation_event *event){
 if(!event)return -EINVAL;
 if(atomic_get(&overflow))return -EOVERFLOW;
 int ret=k_msgq_get(&zarc_observations,event,K_NO_WAIT);
 return atomic_get(&overflow)?-EOVERFLOW:ret;
}
void zarc_native_observation_reset(void){
 k_msgq_purge(&zarc_observations);atomic_clear(&overflow);
}
