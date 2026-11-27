#include "zarc/update_journal.h"
#include <string.h>
#define PAYLOAD_SIZE 77u
static void put(uint8_t *p,size_t n,uint64_t x){for(size_t i=0;i<n;++i)p[i]=(uint8_t)(x>>(8u*i));}
static uint64_t get(const uint8_t *p,size_t n){uint64_t x=0;for(size_t i=0;i<n;++i)x|=(uint64_t)p[i]<<(8u*i);return x;}
static void put_image(uint8_t *p,const zarc_image *i){
 put(p,8,i->image_id);put(p+8,8,i->version);put(p+16,4,i->target);put(p+20,4,i->contract);put(p+24,4,i->config_schema);
}
static zarc_image get_image(const uint8_t *p){
 return (zarc_image){get(p,8),get(p+8,8),(uint32_t)get(p+16,4),(uint32_t)get(p+20,4),(uint32_t)get(p+24,4)};
}
static bool compatible(const zarc_update_policy *p,const zarc_image *i){
 return i->image_id&&i->version>=p->minimum_version&&i->target==p->target&&i->contract==p->contract&&i->config_schema==p->config_schema;
}
static bool valid(const zarc_update *u){
 if(!u||!u->configured||!u->policy.target||!u->policy.contract||!u->policy.config_schema||
    !compatible(&u->policy,&u->active)||(unsigned)u->phase>ZARC_UPDATE_CONFIRMING)return false;
 const zarc_image *c=&u->candidate;
 bool empty=!c->image_id&&!c->version&&!c->target&&!c->contract&&!c->config_schema;
 if(u->phase==ZARC_UPDATE_ACTIVE)return empty;
 if(empty)return u->phase==ZARC_UPDATE_RECOVERY_REQUIRED||u->phase==ZARC_UPDATE_UNAVAILABLE;
 return compatible(&u->policy,c)&&c->image_id!=u->active.image_id&&c->version>u->active.version;
}
zarc_config_status zarc_update_journal_stage(zarc_config_store *store,uint64_t revision,const zarc_update *u,size_t *slot){
 if(!valid(u))return ZARC_CONFIG_INVALID;
 uint8_t payload[PAYLOAD_SIZE]={0};
 payload[0]=(uint8_t)u->phase;
 put(payload+1,4,u->policy.target);put(payload+5,4,u->policy.contract);
 put(payload+9,4,u->policy.config_schema);put(payload+13,8,u->policy.minimum_version);
 put_image(payload+21,&u->active);put_image(payload+49,&u->candidate);
 return zarc_config_stage(store,ZARC_UPDATE_JOURNAL_SCHEMA,revision,payload,sizeof payload,slot);
}
zarc_config_status zarc_update_journal_commit(zarc_config_store *store,size_t slot,uint64_t revision){
 return zarc_config_commit(store,slot,ZARC_UPDATE_JOURNAL_SCHEMA,revision);
}
zarc_config_status zarc_update_journal_load(const zarc_config_store *store,const zarc_update_policy *p,zarc_update *out,uint64_t *revision){
 if(!p||!out||!revision)return ZARC_CONFIG_INVALID;
 zarc_config data;zarc_config_status status=zarc_config_load(store,ZARC_UPDATE_JOURNAL_SCHEMA,&data);
 if(status!=ZARC_CONFIG_OK)return status;
 if(data.length!=PAYLOAD_SIZE||get(data.bytes+1,4)!=p->target||get(data.bytes+5,4)!=p->contract||
    get(data.bytes+9,4)!=p->config_schema||get(data.bytes+13,8)!=p->minimum_version)return ZARC_CONFIG_UNAVAILABLE;
 zarc_update next={.configured=true,.policy=*p,.phase=(zarc_update_phase)data.bytes[0],
  .active=get_image(data.bytes+21),.candidate=get_image(data.bytes+49)};
 if(!valid(&next))return ZARC_CONFIG_INVALID;
 *out=next;*revision=data.revision;return ZARC_CONFIG_OK;
}
