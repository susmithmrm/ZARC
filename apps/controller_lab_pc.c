#include "zarc/controller.h"
#include "zarc/observations.h"
#include "loopback.h"
#include "lab_wire.h"
#include <stdio.h>
#include <stdlib.h>
typedef struct { bool pending; uint64_t due, sequence; zarc_controller_effect effect; } virtual_device;
static void encode_observation(uint8_t *p,const zarc_observation *o,uint64_t seq){
 lab_put(p,8,seq);lab_put(p+8,4,o->id);p[12]=(uint8_t)o->value;p[13]=(uint8_t)o->quality;p[14]=o->ready?1u:0u;
 lab_put(p+15,8,o->generation);lab_put(p+23,8,o->sample_tick);lab_put(p+31,8,o->source_sequence);
}
static bool reply(zarc_controller *c,zarc_observation_stream *stream,virtual_device *d,zarc_socket peer,const zarc_frame *q,uint64_t now){
 zarc_frame r={.operation=ZARC_OP_RESULT,.length=1,.boot=c->link.boot,.session=c->link.session,.request_id=q->request_id};
 zarc_conn_status status=ZARC_CONN_INVALID;
 if(q->operation==ZARC_OP_HELLO&&q->length==0&&q->boot==0&&q->session==0){
  if(c->link.session==UINT64_MAX)return false;
  /* Fixed, explicitly lab-only principal. Never trust peer-supplied roles. */
  status=zarc_controller_open(c,c->link.session+1u,1,1,0,now);r.session=c->link.session;
 }else if(!c->link.connected||q->boot!=c->link.boot||q->session!=c->link.session){
  status=ZARC_CONN_UNAVAILABLE;
 }else if(q->operation==ZARC_OP_CAPABILITIES&&q->length==0){
  status=ZARC_CONN_COMPLETED;r.payload[1]=(uint8_t)c->count;r.length=(uint16_t)(2u+5u*c->count);
  for(size_t i=0;i<c->count;++i){
   lab_put(r.payload+2u+5u*i,4,c->endpoints[i].config.id);r.payload[6u+5u*i]=c->endpoints[i].ready?1u:0u;
  }
 }else if(q->operation==ZARC_OP_SUBSCRIBE&&q->length==4){
  zarc_observation o;uint64_t seq;
  if(zarc_observation_barrier(stream,(uint32_t)lab_get(q->payload,4),&o,&seq)){
   status=ZARC_CONN_COMPLETED;r.length=40;encode_observation(r.payload+1,&o,seq);
  }
 }else if(q->operation==ZARC_OP_EVENT&&q->length==8){
  zarc_observation_event e;zarc_event_status result=zarc_observation_next(stream,q->boot,lab_get(q->payload,8),&e);
  if(result==ZARC_EVENT_AVAILABLE){status=ZARC_CONN_COMPLETED;r.length=40;encode_observation(r.payload+1,&e.observation,e.sequence);}
  else if(result==ZARC_EVENT_EMPTY)status=ZARC_CONN_BUSY;
  else if(result==ZARC_EVENT_GAP)status=ZARC_CONN_UNKNOWN_OUTCOME;
 }else if(q->operation==ZARC_OP_DOMAIN&&q->length==9){
  zarc_intent intent={.session=q->session,.request_id=q->request_id,.domain=(uint32_t)lab_get(q->payload,4),
    .desired=q->payload[4],.operation=ZARC_OP_DOMAIN,.validity=lab_get(q->payload+5,4)};
  zarc_controller_effect effect;
  status=zarc_controller_request(c,&intent,true,true,0,now,!d->pending,&effect);
  if(effect.emitted){d->pending=true;d->due=now+200u;d->effect=effect;}
 }else if(q->operation==ZARC_OP_SNAPSHOT&&q->length==4){
  zarc_observation o;
  if(zarc_controller_snapshot(c,(uint32_t)lab_get(q->payload,4),&o)){
   status=ZARC_CONN_COMPLETED;r.length=11;r.payload[1]=(uint8_t)o.value;r.payload[2]=(uint8_t)o.quality;lab_put(r.payload+3,8,o.generation);
  }
 }else if(q->operation==ZARC_OP_REQUEST_STATUS&&(q->length==0||q->length==8)){
  status=zarc_link_result(&c->link,q->session,ZARC_OP_DOMAIN,q->length==8?lab_get(q->payload,8):q->request_id,now);
 }
 r.payload[0]=(uint8_t)status;return zarc_net_send(peer,&r);
}
int main(int argc,char **argv){
 if(argc!=2)return 2;
 char *end;unsigned long long boot=strtoull(argv[1],&end,10);
 if(!boot||*end)return 2;
 if(!zarc_net_init())return 1;
 uint16_t port=0;zarc_socket server=zarc_net_listen(&port);
 if(server==ZARC_BAD_SOCKET){zarc_net_cleanup();return 1;}
 uint64_t start=zarc_net_milliseconds();zarc_controller c;zarc_endpoint_config config={1,10000};
 if(!zarc_controller_init(&c,&config,1,(uint64_t)boot,5000,0)||!zarc_controller_ready(&c,1,true)||
    !zarc_controller_feedback(&c,1,(uint64_t)boot,1,0,0,ZARC_OFF)){
  zarc_net_close(server);zarc_net_cleanup();return 1;
 }
 zarc_observation_stream observations;
 if(!zarc_observation_stream_init(&observations,&c)){zarc_net_close(server);zarc_net_cleanup();return 1;}
 printf("%u\n",(unsigned)port);fflush(stdout);
 virtual_device device={.sequence=1};
 zarc_socket peer=ZARC_BAD_SOCKET;zarc_stream stream={0};
 for(;;){
  uint64_t now=zarc_net_milliseconds()-start;
  if(!zarc_controller_tick(&c,now))break;
  if(device.pending&&now>=device.due){
   if(!zarc_controller_feedback(&c,1,c.link.boot,++device.sequence,now,now,device.desired))break;
   device.pending=false;
  }
  if(!zarc_observation_stream_sync(&observations,&c))break;
  int ready=zarc_net_readable(peer==ZARC_BAD_SOCKET?server:peer,20);
  if(ready<0)break;
  if(!ready)continue;
  if(peer==ZARC_BAD_SOCKET){peer=zarc_net_accept(server);stream=(zarc_stream){0};continue;}
  uint8_t bytes[512];int n=zarc_net_read(peer,bytes,sizeof bytes);bool ok=n>0;
  size_t pos=0;
  while(ok&&pos<(size_t)n){
   zarc_frame q;size_t used;
   zarc_stream_status s=zarc_stream_feed(&stream,bytes+pos,(size_t)n-pos,&used,&q);pos+=used;
   if(s==ZARC_STREAM_INVALID)ok=false;
   else if(s==ZARC_STREAM_FRAME)ok=reply(&c,&observations,&device,peer,&q,now);
   else if(!used)ok=false;
  }
  if(!ok){zarc_net_close(peer);peer=ZARC_BAD_SOCKET;zarc_controller_disconnect(&c);}
 }
 zarc_net_close(peer);zarc_net_close(server);zarc_net_cleanup();return 1;
}
