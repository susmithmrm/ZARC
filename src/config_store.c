#include "zarc/config_store.h"
#include <string.h>
#define MARK UINT8_C(0xa5)
#define MARK_OFFSET (ZARC_CONFIG_SLOT-1u)
static uint64_t get(const uint8_t *p,size_t n){
 uint64_t v=0;for(size_t i=0;i<n;++i)v|=(uint64_t)p[i]<<(8u*i);return v;
}
static void put(uint8_t *p,size_t n,uint64_t v){for(size_t i=0;i<n;++i)p[i]=(uint8_t)(v>>(8u*i));}
static uint32_t crc(const uint8_t *p,size_t n,uint32_t c){
 for(size_t i=0;i<n;++i){c^=p[i];for(unsigned j=0;j<8;++j)c=(c>>1)^((c&1u)?UINT32_C(0xedb88320):0u);}return c;
}
static uint32_t checksum(const uint8_t *p,size_t n){return ~crc(p+ZARC_CONFIG_HEADER,n,crc(p,16,UINT32_MAX));}
static bool decode(const uint8_t *p,zarc_config *out){
 uint64_t schema=get(p,4),rev=get(p+4,8),len=get(p+12,4);
 if(!schema||!rev||len>ZARC_CONFIG_MAX||get(p+16,4)!=checksum(p,(size_t)len))return false;
 *out=(zarc_config){0};out->schema=(uint32_t)schema;out->revision=rev;out->length=(size_t)len;
 memcpy(out->bytes,p+ZARC_CONFIG_HEADER,out->length);return true;
}
/* Select highest valid committed revision regardless of schema. Never silently
 * fall back to an older schema when a newer committed record is incompatible. */
static zarc_config_status latest(const zarc_config_store *s,zarc_config *out,size_t *index){
 bool found=false;
 for(size_t i=0;i<2;++i){
  zarc_config c;if(s->slots[i][MARK_OFFSET]!=MARK||!decode(s->slots[i],&c))continue;
  if(found&&c.revision==out->revision)return ZARC_CONFIG_CONFLICT;
  if(!found||c.revision>out->revision){*out=c;*index=i;found=true;}
 }
 return found?ZARC_CONFIG_OK:ZARC_CONFIG_UNAVAILABLE;
}
zarc_config_status zarc_config_load(const zarc_config_store *s,uint32_t schema,zarc_config *out){
 if(!s||!schema||!out)return ZARC_CONFIG_INVALID;
 zarc_config c;size_t i;zarc_config_status r=latest(s,&c,&i);
 if(r!=ZARC_CONFIG_OK)return r;
 if(c.schema!=schema)return ZARC_CONFIG_UNAVAILABLE;
 *out=c;return ZARC_CONFIG_OK;
}
zarc_config_status zarc_config_stage(zarc_config_store *s,uint32_t schema,uint64_t revision,const uint8_t *bytes,size_t n,size_t *slot){
 if(!s||!schema||!revision||n>ZARC_CONFIG_MAX||(!bytes&&n)||!slot)return ZARC_CONFIG_INVALID;
 zarc_config c;size_t active=0;zarc_config_status r=latest(s,&c,&active);
 if(r==ZARC_CONFIG_CONFLICT)return r;
 if(r==ZARC_CONFIG_OK&&revision<=c.revision)return ZARC_CONFIG_CONFLICT;
 size_t i=r==ZARC_CONFIG_OK?1u-active:0u;
 uint8_t record[ZARC_CONFIG_SLOT]={0};put(record,4,schema);put(record+4,8,revision);put(record+12,4,n);
 if(n)memcpy(record+ZARC_CONFIG_HEADER,bytes,n);
 put(record+16,4,checksum(record,n));
 s->slots[i][MARK_OFFSET]=0;memcpy(s->slots[i],record,MARK_OFFSET);*slot=i;return ZARC_CONFIG_OK;
}
zarc_config_status zarc_config_commit(zarc_config_store *s,size_t i,uint32_t schema,uint64_t revision){
 if(!s||i>=2||!schema||!revision)return ZARC_CONFIG_INVALID;
 zarc_config c;
 if(!decode(s->slots[i],&c)||c.schema!=schema||c.revision!=revision)return ZARC_CONFIG_INVALID;
 zarc_config current;size_t active=0;zarc_config_status r=latest(s,&current,&active);
 if(r==ZARC_CONFIG_CONFLICT)return r;
 if(r==ZARC_CONFIG_OK){
  if(active==i&&current.revision==revision)return ZARC_CONFIG_OK;
  if(current.revision>=revision)return ZARC_CONFIG_CONFLICT;
 }
 s->slots[i][MARK_OFFSET]=MARK;return ZARC_CONFIG_OK;
}

zarc_config_status zarc_config_prepare_migration(zarc_config_store *s,uint32_t source,uint32_t target,
 uint64_t revision,bool authorized,zarc_config_migration_fn map,void *context,zarc_config_migration *ticket){
 if(!s||!source||!target||source==target||!revision||!map||!ticket)return ZARC_CONFIG_INVALID;
 *ticket=(zarc_config_migration){0};
 if(!authorized)return ZARC_CONFIG_UNAVAILABLE;
 zarc_config current;zarc_config_status status=zarc_config_load(s,source,&current);
 if(status!=ZARC_CONFIG_OK)return status;
 if(revision<=current.revision)return ZARC_CONFIG_CONFLICT;
 zarc_config_migration next={.source_schema=source,.target_schema=target,.source_revision=current.revision,
  .target_revision=revision};
 if(!map(context,&current,next.bytes,sizeof next.bytes,&next.length)||next.length>sizeof next.bytes)return ZARC_CONFIG_INVALID;
 status=zarc_config_stage(s,target,revision,next.bytes,next.length,&next.slot);
 if(status!=ZARC_CONFIG_OK)return status;
 next.prepared=true;*ticket=next;return ZARC_CONFIG_OK;
}
zarc_config_status zarc_config_commit_migration(zarc_config_store *s,const zarc_config_migration *t){
 if(!s||!t||!t->prepared||t->slot>=2||!t->source_schema||!t->target_schema||
    t->source_schema==t->target_schema||!t->source_revision||t->target_revision<=t->source_revision||
    t->length>ZARC_CONFIG_MAX)return ZARC_CONFIG_INVALID;
 zarc_config candidate,current;size_t active;
 if(!decode(s->slots[t->slot],&candidate)||candidate.schema!=t->target_schema||candidate.revision!=t->target_revision||
    candidate.length!=t->length||memcmp(candidate.bytes,t->bytes,t->length)!=0)return ZARC_CONFIG_CONFLICT;
 zarc_config_status status=latest(s,&current,&active);
 if(status!=ZARC_CONFIG_OK)return status;
 if(active==t->slot&&current.schema==t->target_schema&&current.revision==t->target_revision)return ZARC_CONFIG_OK;
 if(current.schema!=t->source_schema||current.revision!=t->source_revision||active==t->slot)return ZARC_CONFIG_CONFLICT;
 return zarc_config_commit(s,t->slot,t->target_schema,t->target_revision);
}
