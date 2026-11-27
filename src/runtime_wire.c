#include "zarc/runtime_wire.h"
static void put(uint8_t *p,size_t n,uint64_t v){for(size_t i=0;i<n;++i)p[i]=(uint8_t)(v>>(8u*i));}
static uint64_t get(const uint8_t *p,size_t n){uint64_t v=0;for(size_t i=0;i<n;++i)v|=(uint64_t)p[i]<<(8u*i);return v;}
static bool allowed(const zarc_wire_peer *p,uint8_t op,uint32_t id){
 return p->allow(p->context,p->principal,op,id);
}
static void observation(uint8_t *p,const zarc_observation *o,uint64_t sequence){
 put(p,8,sequence);put(p+8,4,o->id);p[12]=(uint8_t)o->value;p[13]=(uint8_t)o->quality;p[14]=o->ready?1u:0u;
 put(p+15,8,o->generation);put(p+23,8,o->sample_tick);put(p+31,8,o->source_sequence);
}
bool zarc_runtime_dispatch(zarc_runtime *r,const zarc_wire_peer *p,const zarc_frame *q,uint64_t now,
 bool reserved,zarc_frame *out,zarc_controller_effect *effect){
 if(!r||!p||!q||!out||!effect||!p->principal||!p->allow||!q->request_id||q->length>ZARC_PAYLOAD_MAX)return false;
 *effect=(zarc_controller_effect){0};
 *out=(zarc_frame){.operation=ZARC_OP_RESULT,.length=1,.boot=r->controller.link.boot,
  .session=r->controller.link.session,.request_id=q->request_id};
 zarc_conn_status status=ZARC_CONN_INVALID;
 if(!zarc_runtime_remote_ready(r))status=ZARC_CONN_UNAVAILABLE;
 else if(q->operation==ZARC_OP_HELLO&&q->length==0&&q->boot==0&&q->session==0){
  if(!allowed(p,ZARC_OP_HELLO,0))status=ZARC_CONN_DENIED;
  else if(r->controller.link.session==UINT64_MAX)status=ZARC_CONN_UNAVAILABLE;
  else status=zarc_runtime_open(r,r->controller.link.session+1u,p->principal,1,0,now);
  out->session=r->controller.link.session;
 }else if(q->operation==ZARC_OP_HELLO&&q->length==0&&q->boot==r->controller.link.boot&&q->session!=0){
  if(!allowed(p,ZARC_OP_HELLO,0))status=ZARC_CONN_DENIED;
  else status=zarc_controller_renew(&r->controller,q->session,p->principal,now);
  out->session=r->controller.link.session;
 }else if(!r->controller.link.connected||q->boot!=r->controller.link.boot||
           q->session!=r->controller.link.session||p->principal!=r->controller.link.principal){
  status=ZARC_CONN_UNAVAILABLE;
 }else{
  /* Refresh expiry/readiness before exposing a snapshot. No periodic sample
   * is fabricated here; the independent provider owns batch sampling. */
  if(!zarc_runtime_refresh(r,now))return false;
  if(q->operation==ZARC_OP_CAPABILITIES&&q->length==0){
   if(!allowed(p,ZARC_OP_CAPABILITIES,0))status=ZARC_CONN_DENIED;
   else{
    size_t count=0;
    for(size_t i=0;i<r->controller.count;++i){
     const zarc_endpoint *e=&r->controller.endpoints[i];
     if(!allowed(p,ZARC_OP_SNAPSHOT,e->config.id))continue;
     put(out->payload+2u+5u*count,4,e->config.id);out->payload[6u+5u*count]=e->ready?1u:0u;++count;
    }
    out->payload[1]=(uint8_t)count;out->length=(uint16_t)(2u+5u*count);status=ZARC_CONN_COMPLETED;
   }
  }else if((q->operation==ZARC_OP_SNAPSHOT||q->operation==ZARC_OP_SUBSCRIBE)&&q->length==4){
   uint32_t id=(uint32_t)get(q->payload,4);zarc_observation o;uint64_t seq;
   if(!allowed(p,q->operation,id))status=ZARC_CONN_DENIED;
   else if(zarc_observation_barrier(&r->observations,id,&o,&seq)){
    status=ZARC_CONN_COMPLETED;
    if(q->operation==ZARC_OP_SNAPSHOT){out->length=11;out->payload[1]=(uint8_t)o.value;out->payload[2]=(uint8_t)o.quality;put(out->payload+3,8,o.generation);}
    else{out->length=40;observation(out->payload+1,&o,seq);}
   }
  }else if(q->operation==ZARC_OP_HEALTH&&q->length==4){
   uint32_t id=(uint32_t)get(q->payload,4);zarc_runtime_health_view v;
   if(!allowed(p,ZARC_OP_HEALTH,id))status=ZARC_CONN_DENIED;
   else if(zarc_runtime_health_read(r,id,&v)){
    status=ZARC_CONN_COMPLETED;out->length=28;
    out->payload[1]=1;put(out->payload+2,4,v.id);put(out->payload+6,4,v.failures);
    out->payload[10]=v.ready?1u:0u;
    out->payload[11]=(uint8_t)((v.pending?1u:0u)|(v.effect_owned?2u:0u));
    put(out->payload+12,8,v.sample_tick);put(out->payload+20,8,v.source_sequence);
   }
  }else if(q->operation==ZARC_OP_UPDATE_STATUS&&q->length==0){
   zarc_update update;uint64_t revision,tick;
   if(!allowed(p,ZARC_OP_UPDATE_STATUS,0))status=ZARC_CONN_DENIED;
   else if(!zarc_runtime_read_update(r,now,&update,&revision,&tick))status=ZARC_CONN_UNAVAILABLE;
   else{
    status=ZARC_CONN_COMPLETED;out->length=51;
    out->payload[1]=1;out->payload[2]=(uint8_t)update.phase;
    put(out->payload+3,8,revision);put(out->payload+11,8,update.active.image_id);
    put(out->payload+19,8,update.active.version);put(out->payload+27,8,update.candidate.image_id);
    put(out->payload+35,8,update.candidate.version);put(out->payload+43,8,tick);
   }
  }else if(q->operation==ZARC_OP_EVENT&&q->length==8){
   zarc_observation_event e;
   if(!allowed(p,ZARC_OP_EVENT,0))status=ZARC_CONN_DENIED;
   else{
    zarc_event_status next=zarc_observation_next(&r->observations,q->boot,get(q->payload,8),&e);
    if(next==ZARC_EVENT_AVAILABLE){
     /* Never silently skip an unauthorized event or advance its cursor. */
     if(!allowed(p,ZARC_OP_EVENT,e.observation.id))status=ZARC_CONN_DENIED;
     else{status=ZARC_CONN_COMPLETED;out->length=40;observation(out->payload+1,&e.observation,e.sequence);}
    }else if(next==ZARC_EVENT_EMPTY)status=ZARC_CONN_BUSY;
    else if(next==ZARC_EVENT_GAP)status=ZARC_CONN_UNKNOWN_OUTCOME;
   }
  }else if(q->operation==ZARC_OP_DOMAIN&&q->length==9){
   zarc_intent intent={.session=q->session,.request_id=q->request_id,.domain=(uint32_t)get(q->payload,4),
    .desired=q->payload[4],.operation=ZARC_OP_DOMAIN,.validity=get(q->payload+5,4)};
   status=zarc_runtime_remote(r,&intent,allowed(p,ZARC_OP_DOMAIN,intent.domain),p->age_known,p->age,now,reserved,effect);
  }else if(q->operation==ZARC_OP_REQUEST_STATUS&&q->length==8){
   uint64_t id=get(q->payload,8);status=ZARC_CONN_UNKNOWN_OUTCOME;
   for(size_t i=0;i<r->controller.link.count;++i){
    const zarc_request_entry *e=&r->controller.link.entries[i];
    if(e->intent.session==q->session&&e->intent.operation==ZARC_OP_DOMAIN&&e->intent.request_id==id){
     status=allowed(p,ZARC_OP_REQUEST_STATUS,e->intent.domain)?
      zarc_link_result(&r->controller.link,q->session,ZARC_OP_DOMAIN,id,now):ZARC_CONN_DENIED;break;
    }
   }
  }
 }
 out->payload[0]=(uint8_t)status;return true;
}
