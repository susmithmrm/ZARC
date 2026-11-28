#include "zarc/connect.h"
#include <string.h>
static bool command(uint8_t op){return op==ZARC_OP_DOMAIN||op==ZARC_OP_MODE;}
static bool operation(uint8_t op){return op>=ZARC_OP_HELLO&&op<=ZARC_OP_EVENT;}
static zarc_request_entry *find(zarc_link *l,uint64_t session,uint8_t op,uint64_t id){
 for(size_t i=0;i<l->count;++i){zarc_request_entry *e=&l->entries[i];
  if(e->intent.session==session&&e->intent.operation==op&&e->intent.request_id==id)return e;}
 return NULL;
}
bool zarc_link_init(zarc_link *l,uint64_t boot,uint64_t max_validity,uint64_t now){
 if(!l||!boot||!max_validity)return false;
 *l=(zarc_link){0};l->boot=boot;l->max_validity=max_validity;l->last_tick=now;return true;
}
bool zarc_link_tick(zarc_link *l,uint64_t now){
 if(!l||!l->boot||now<l->last_tick)return false;
 l->last_tick=now;
 for(size_t i=0;i<l->count;++i)
  if(l->entries[i].status==ZARC_CONN_ACCEPTED&&now>=l->entries[i].deadline)
   l->entries[i].status=ZARC_CONN_UNKNOWN_OUTCOME;
 return true;
}
zarc_conn_status zarc_link_open(zarc_link *l,uint64_t session,uint64_t principal,uint8_t major,uint8_t minor,uint64_t now){
 if(!l||!l->boot||!session||!principal||now<l->last_tick)return ZARC_CONN_INVALID;
 if(major!=ZARC_CONNECT_MAJOR||minor!=ZARC_CONNECT_MINOR)return ZARC_CONN_INCOMPATIBLE;
 if(l->connected||session<=l->session)return ZARC_CONN_CONFLICT;
 if(!zarc_link_tick(l,now))return ZARC_CONN_INVALID;
 for(size_t i=0;i<l->count;++i)if(l->entries[i].status==ZARC_CONN_ACCEPTED)return ZARC_CONN_BUSY;
 memset(l->entries,0,sizeof l->entries);l->count=0;l->session=session;l->principal=principal;l->connected=true;
 return ZARC_CONN_COMPLETED;
}
void zarc_link_disconnect(zarc_link *l){if(l)l->connected=false;}
zarc_conn_status zarc_link_admit(zarc_link *l,const zarc_intent *q,bool allowed,bool age_known,uint64_t age,uint64_t now,bool *fresh){
 if(!fresh)return ZARC_CONN_INVALID;
 *fresh=false;
 if(!l||!q||!l->boot||now<l->last_tick||!command(q->operation)||!q->request_id||!q->domain||!q->validity)return ZARC_CONN_INVALID;
 if(!l->connected||q->session!=l->session)return ZARC_CONN_UNAVAILABLE;
 if(!allowed)return ZARC_CONN_DENIED;
 /* The domain adapter validates product-specific desired enums and authority. */
 (void)zarc_link_tick(l,now);
 zarc_request_entry *e=find(l,q->session,q->operation,q->request_id);
 if(e){
  if(e->principal!=l->principal||e->intent.domain!=q->domain||e->intent.desired!=q->desired||e->intent.validity!=q->validity)return ZARC_CONN_CONFLICT;
  return e->status;
 }
 if(q->validity>l->max_validity)return ZARC_CONN_INVALID;
 if(!age_known||age>=q->validity)return ZARC_CONN_EXPIRED;
 uint64_t remaining=q->validity-age;
 if(now>UINT64_MAX-remaining)return ZARC_CONN_INVALID;
 if(l->count==ZARC_REQUEST_LIMIT)return ZARC_CONN_BUSY;
 l->entries[l->count++]=(zarc_request_entry){.intent=*q,.principal=l->principal,.deadline=now+remaining,.status=ZARC_CONN_ACCEPTED};
 *fresh=true;return ZARC_CONN_ACCEPTED;
}
zarc_conn_status zarc_link_result(zarc_link *l,uint64_t session,uint8_t op,uint64_t id,uint64_t now){
 if(!l||!session||!command(op)||!id||now<l->last_tick)return ZARC_CONN_INVALID;
 if(!zarc_link_tick(l,now))return ZARC_CONN_INVALID;
 if(session<l->session)return ZARC_CONN_UNKNOWN_OUTCOME;
 if(!l->connected||session!=l->session)return ZARC_CONN_UNAVAILABLE;
 zarc_request_entry *e=find(l,session,op,id);return e?e->status:ZARC_CONN_UNKNOWN_OUTCOME;
}
zarc_conn_status zarc_link_finish(zarc_link *l,uint64_t session,uint8_t op,uint64_t id,uint64_t now,bool confirmed){
 if(!l||!command(op)||!id||!zarc_link_tick(l,now))return ZARC_CONN_INVALID;
 zarc_request_entry *e=find(l,session,op,id);
 if(!e)return ZARC_CONN_UNKNOWN_OUTCOME;
 if(e->status==ZARC_CONN_ACCEPTED)e->status=confirmed?ZARC_CONN_COMPLETED:ZARC_CONN_UNKNOWN_OUTCOME;
 return e->status;
}
static void put(uint8_t *p,size_t n,uint64_t v){for(size_t i=0;i<n;++i)p[i]=(uint8_t)(v>>(8u*i));}
static uint64_t get(const uint8_t *p,size_t n){uint64_t v=0;for(size_t i=0;i<n;++i)v|=(uint64_t)p[i]<<(8u*i);return v;}
static bool header(const uint8_t *p,size_t *total){
 if(p[0]!='Z'||p[1]!='R'||p[2]!=ZARC_CONNECT_MAJOR||p[3]!=ZARC_CONNECT_MINOR||!operation(p[4])||p[5]!=0)return false;
 size_t n=(size_t)get(p+6,2);if(n>ZARC_PAYLOAD_MAX)return false;
 *total=ZARC_FRAME_HEADER+n;return true;
}
bool zarc_frame_encode(const zarc_frame *f,uint8_t *p,size_t cap,size_t *length){
 if(!f||!p||!length||!operation(f->operation)||f->length>ZARC_PAYLOAD_MAX||cap<ZARC_FRAME_HEADER+(size_t)f->length)return false;
 p[0]='Z';p[1]='R';p[2]=ZARC_CONNECT_MAJOR;p[3]=ZARC_CONNECT_MINOR;p[4]=f->operation;p[5]=0;
 put(p+6,2,f->length);put(p+8,8,f->boot);put(p+16,8,f->session);put(p+24,8,f->request_id);
 memcpy(p+ZARC_FRAME_HEADER,f->payload,f->length);*length=ZARC_FRAME_HEADER+(size_t)f->length;return true;
}
bool zarc_frame_decode(const uint8_t *p,size_t n,zarc_frame *f){
 size_t total;if(!p||!f||n<ZARC_FRAME_HEADER||!header(p,&total)||n!=total)return false;
 zarc_frame x={0};x.operation=p[4];x.length=(uint16_t)(total-ZARC_FRAME_HEADER);
 x.boot=get(p+8,8);x.session=get(p+16,8);x.request_id=get(p+24,8);
 memcpy(x.payload,p+ZARC_FRAME_HEADER,x.length);*f=x;return true;
}
zarc_stream_status zarc_stream_feed(zarc_stream *s,const uint8_t *p,size_t n,size_t *used,zarc_frame *f){
 if(!used)return ZARC_STREAM_INVALID;
 *used=0;
 if(!s||(!p&&n)||!f||s->failed||s->used>sizeof s->bytes)return ZARC_STREAM_INVALID;
 while(*used<n){
  if(s->used==sizeof s->bytes){s->failed=true;return ZARC_STREAM_INVALID;}
  s->bytes[s->used++]=p[(*used)++];
  if(s->used>=ZARC_FRAME_HEADER){
   size_t total;
   if(!header(s->bytes,&total)||s->used>total){s->failed=true;return ZARC_STREAM_INVALID;}
   if(s->used==total){
    if(!zarc_frame_decode(s->bytes,total,f)){s->failed=true;return ZARC_STREAM_INVALID;}
    s->used=0;return ZARC_STREAM_FRAME;
   }
  }
 }
 return ZARC_STREAM_MORE;
}
bool zarc_cache_snapshot(zarc_snapshot_cache *c,uint64_t boot,uint64_t session,uint64_t generation,uint64_t sequence,uint32_t value){
 if(!c||!boot||!session)return false;
 if(c->valid&&c->boot==boot&&c->session==session&&(generation<c->generation||sequence<c->sequence))return false;
 *c=(zarc_snapshot_cache){.boot=boot,.session=session,.generation=generation,.sequence=sequence,.value=value,.valid=true};return true;
}
bool zarc_cache_event(zarc_snapshot_cache *c,uint64_t boot,uint64_t session,uint64_t generation,uint64_t sequence,uint32_t value){
 if(!c||!c->valid)return false;
 if(c->boot!=boot||c->session!=session||c->sequence==UINT64_MAX||sequence!=c->sequence+1||
    generation<c->generation||(generation==c->generation&&value!=c->value)){
  c->valid=false;return false;
 }
 c->generation=generation;c->sequence=sequence;c->value=value;return true;
}
void zarc_cache_invalidate(zarc_snapshot_cache *c){if(c)c->valid=false;}
