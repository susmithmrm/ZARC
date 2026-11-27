#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/ipc/ipc_service_backend.h>
#include <errno.h>
#include "transport.h"
/* Deliberately simulated backend, actual native IPC service and ZARC adapter.
 * No physical inter-core memory, trust, or remote peer is asserted. */
struct fixture {const struct ipc_ept_cfg *cfg;};
static struct fixture f;
static int reg(const struct device *dev,void **token,const struct ipc_ept_cfg *cfg){
 struct fixture *state=dev->data;if(state->cfg)return -EBUSY;
 state->cfg=cfg;*token=state;cfg->cb.bound(cfg->priv);return 0;
}
static int dereg(const struct device *dev,void *token){
 struct fixture *state=dev->data;if(state!=token||!state->cfg)return -EINVAL;
 state->cfg=NULL;return 0;
}
static int send_copy(const struct device *dev,void *token,const void *data,size_t len){
 struct fixture *state=dev->data;if(state!=token||!state->cfg)return -ENOTCONN;
 state->cfg->cb.received(data,len,state->cfg->priv);return (int)len;
}
static const struct ipc_service_backend api={.register_endpoint=reg,.deregister_endpoint=dereg,.send=send_copy};
DEVICE_DEFINE(zarc_profile_ipc,"zarc_profile_ipc",NULL,NULL,&f,NULL,POST_KERNEL,
 CONFIG_KERNEL_INIT_PRIORITY_DEVICE,&api);
K_MSGQ_DEFINE(ipc_queue,sizeof(struct zarc_native_bytes),2,8);
static struct zarc_native_ipc adapter;
int zarc_profile_ipc_open(void){
 return zarc_native_ipc_open(&adapter,DEVICE_GET(zarc_profile_ipc),"profile-fixture",&ipc_queue);
}
int zarc_profile_ipc_exchange(const zarc_frame *q,zarc_frame *received){
 struct zarc_native_bytes b;
 int rc=zarc_native_ipc_send(&adapter,q);
 if(rc)return rc;
 rc=k_msgq_get(&ipc_queue,&b,K_NO_WAIT);
 if(rc)return rc;
 return zarc_frame_decode(b.data,b.len,received)?0:-EBADMSG;
}
int zarc_profile_ipc_close(void){return zarc_native_ipc_close(&adapter);}
