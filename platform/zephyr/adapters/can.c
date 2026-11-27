#include "transport.h"
#include <errno.h>
static void can_rx(const struct device *dev,struct can_frame *frame,void *user){
 ARG_UNUSED(dev);
 struct zarc_native_can *c=user;
 if(k_msgq_put(c->rx,frame,K_NO_WAIT)!=0)atomic_set(&c->fault,1);
}
static void can_tx(const struct device *dev,int error,void *user){
 ARG_UNUSED(dev);
 struct zarc_native_can *c=user;
 if(error)atomic_set(&c->fault,1);
}
int zarc_native_can_open(struct zarc_native_can *c,const struct device *dev,struct k_msgq *rx,const struct can_filter *filter){
 if(!c||!dev||!rx||!filter||!device_is_ready(dev)||rx->msg_size!=sizeof(struct can_frame))return -EINVAL;
 if(c->active)return -EALREADY;
 c->dev=dev;c->rx=rx;atomic_clear(&c->fault);
 int ret=can_add_rx_filter(dev,can_rx,c,filter);
 if(ret<0)return ret;
 c->filter_id=ret;ret=can_start(dev);
 if(ret<0){can_remove_rx_filter(dev,c->filter_id);return ret;}
 c->active=true;return 0;
}
int zarc_native_can_send(struct zarc_native_can *c,const struct can_frame *frame){
 if(!c||!frame||!c->active||atomic_get(&c->fault))return -ENOTCONN;
 /* Frame mapping and approved identifiers are supplied by product adapter. */
 return can_send(c->dev,frame,K_NO_WAIT,can_tx,c);
}
int zarc_native_can_close(struct zarc_native_can *c){
 if(!c||!c->active)return -EINVAL;
 atomic_set(&c->fault,1);
 int ret=can_stop(c->dev);
 if(ret<0)return ret;
 can_remove_rx_filter(c->dev,c->filter_id);c->active=false;return 0;
}
