#include "transport.h"
#include <errno.h>
#include <string.h>
static void bound(void *user){struct zarc_native_ipc *i=user;atomic_set(&i->bound,1);}
static void unbound(void *user){struct zarc_native_ipc *i=user;atomic_clear(&i->bound);atomic_set(&i->fault,1);}
static void error(const char *message,void *user){ARG_UNUSED(message);unbound(user);}
static void received(const void *data,size_t len,void *user){
 struct zarc_native_ipc *i=user;
 if(!data||len<ZARC_FRAME_HEADER||len>ZARC_NATIVE_FRAME_MAX){atomic_set(&i->fault,1);return;}
 struct zarc_native_bytes b={.len=len};memcpy(b.data,data,len);
 if(k_msgq_put(i->rx,&b,K_NO_WAIT)!=0)atomic_set(&i->fault,1);
}
int zarc_native_ipc_open(struct zarc_native_ipc *i,const struct device *dev,const char *name,struct k_msgq *rx){
 if(!i||!dev||!name||!rx||!device_is_ready(dev)||rx->msg_size!=sizeof(struct zarc_native_bytes))return -EINVAL;
 if(i->registered)return -EALREADY;
 i->dev=dev;i->rx=rx;i->config=(struct ipc_ept_cfg){.name=name,.cb={.bound=bound,.unbound=unbound,.received=received,.error=error},.priv=i};
 atomic_clear(&i->bound);atomic_clear(&i->fault);
 int ret=ipc_service_open_instance(dev);
 if(ret<0&&ret!=-EALREADY)return ret;
 ret=ipc_service_register_endpoint(dev,&i->endpoint,&i->config);
 if(ret<0)return ret;
 i->registered=true;return 0;
}
int zarc_native_ipc_send(struct zarc_native_ipc *i,const zarc_frame *f){
 if(!i||!i->registered||!atomic_get(&i->bound)||atomic_get(&i->fault))return -ENOTCONN;
 uint8_t buffer[ZARC_NATIVE_FRAME_MAX];size_t n;
 if(!zarc_frame_encode(f,buffer,sizeof buffer,&n))return -EINVAL;
 /* Copying API; backend latency must be qualified. Dedicated C3 worker only. */
 int ret=ipc_service_send(&i->endpoint,buffer,n);
 return ret<0?ret:(size_t)ret==n?0:-EIO;
}
int zarc_native_ipc_close(struct zarc_native_ipc *i){
 if(!i||!i->registered)return -EINVAL;
 atomic_set(&i->fault,1);atomic_clear(&i->bound);
 int ret=ipc_service_deregister_endpoint(&i->endpoint);
 if(ret<0)return ret;
 i->registered=false;return 0;
}
