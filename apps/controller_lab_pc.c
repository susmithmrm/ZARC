#include "zarc/runtime_wire.h"
#include "loopback.h"
#include "lab_wire.h"
#include <stdio.h>
#include <stdlib.h>
typedef struct {
 bool pending;uint64_t due,sequence;zarc_controller_effect effect;zarc_value observed;
} virtual_device;
static bool scope(void *ctx,uint64_t principal,uint8_t op,uint32_t id){
 (void)ctx;(void)op;(void)id;return principal==1; /* isolated lab only */
}
static bool reply(zarc_runtime *r,virtual_device *d,zarc_socket peer,const zarc_frame *q,uint64_t now){
 zarc_frame result;zarc_controller_effect effect;
 const zarc_wire_peer p={.principal=1,.age_known=true,.allow=scope};
 /* Preserve the old zero-length status query in this lab adapter only. */
 zarc_frame normalized=*q;
 if(q->operation==ZARC_OP_REQUEST_STATUS&&q->length==0){
  normalized.length=8;lab_put(normalized.payload,8,q->request_id);
 }
 if(!zarc_runtime_dispatch(r,&p,&normalized,now,!d->pending,&result,&effect))return false;
 if(effect.emitted){d->pending=true;d->due=now+200u;d->effect=effect;}
 return zarc_net_send(peer,&result);
}
int main(int argc,char **argv){
 if(argc!=2)return 2;
 char *end;unsigned long long boot=strtoull(argv[1],&end,10);
 if(!boot||*end)return 2;
 if(!zarc_net_init())return 1;
 uint16_t port=0;zarc_socket server=zarc_net_listen(&port);
 if(server==ZARC_BAD_SOCKET){zarc_net_cleanup();return 1;}
 uint64_t start=zarc_net_milliseconds();
 zarc_runtime r={0};
 const zarc_runtime_config config={.endpoint={1,10000},.period=50,.deadline=50,
  .health_limit=2000,.max_validity=5000,.remote_enabled=true};
 virtual_device device={.sequence=1,.observed=ZARC_OFF};
 if(!zarc_runtime_init(&r,&config,(uint64_t)boot,0)||
    !zarc_runtime_step(&r,0,true,device.sequence,0,device.observed)){
  zarc_net_close(server);zarc_net_cleanup();return 1;
 }
 printf("%u\n",(unsigned)port);fflush(stdout);
 zarc_socket peer=ZARC_BAD_SOCKET;zarc_stream stream={0};
 for(;;){
  uint64_t now=zarc_net_milliseconds()-start;
  if(device.pending&&now>=device.due){
   if(now<device.effect.deadline)device.observed=device.effect.desired;
   if(!zarc_runtime_release(&r,&device.effect,now))break;
   device.pending=false;
  }
  if(device.sequence==UINT64_MAX||!zarc_runtime_step(&r,now,true,++device.sequence,now,device.observed))break;
  int ready=zarc_net_readable(peer==ZARC_BAD_SOCKET?server:peer,10);
  if(ready<0)break;
  if(!ready)continue;
  if(peer==ZARC_BAD_SOCKET){peer=zarc_net_accept(server);stream=(zarc_stream){0};continue;}
  uint8_t bytes[512];int n=zarc_net_read(peer,bytes,sizeof bytes);bool ok=n>0;
  size_t pos=0;
  while(ok&&pos<(size_t)n){
   zarc_frame q;size_t used;
   zarc_stream_status s=zarc_stream_feed(&stream,bytes+pos,(size_t)n-pos,&used,&q);pos+=used;
   if(s==ZARC_STREAM_INVALID)ok=false;
   else if(s==ZARC_STREAM_FRAME)ok=reply(&r,&device,peer,&q,now);
   else if(!used)ok=false;
  }
  if(!ok){zarc_net_close(peer);peer=ZARC_BAD_SOCKET;zarc_runtime_disconnect(&r);}
 }
 zarc_net_close(peer);zarc_net_close(server);zarc_net_cleanup();return 1;
}
