#include "zarc/diag_store.h"
#include <string.h>
static void put(uint8_t *p,size_t n,uint64_t x){for(size_t i=0;i<n;++i)p[i]=(uint8_t)(x>>(8u*i));}
static uint64_t get(const uint8_t *p,size_t n){uint64_t x=0;for(size_t i=0;i<n;++i)x|=(uint64_t)p[i]<<(8u*i);return x;}
static uint32_t crc(const uint8_t *p,size_t n){
 uint32_t c=UINT32_MAX;
 for(size_t i=0;i<n;++i){
  uint8_t byte=(i>=24u&&i<28u)?0:p[i];
  c^=byte;for(unsigned j=0;j<8;++j)c=(c>>1)^((c&1u)?UINT32_C(0xedb88320):0u);
 }
 return ~c;
}
static bool valid(const zarc_fault *f){
 return f->version==ZARC_DIAG_VERSION&&f->code&&f->service&&f->epoch&&f->occurrences&&
  (unsigned)f->quality<=ZARC_DIAG_FAULT&&(unsigned)f->severity<=ZARC_DIAG_CRITICAL;
}
static void encode_record(uint8_t *p,const zarc_fault *f){
 put(p,4,f->version);put(p+4,4,f->code);put(p+8,4,f->service);put(p+12,4,f->occurrences);
 put(p+16,8,f->epoch);put(p+24,8,f->generation);put(p+32,8,f->timestamp);put(p+40,8,f->correlation);
 put(p+48,8,f->config_digest);put(p+56,8,f->build_digest);
 put(p+64,4,(uint32_t)f->quality);put(p+68,4,(uint32_t)f->severity);
}
static bool decode_record(const uint8_t *p,zarc_fault *f){
 uint64_t quality=get(p+64,4),severity=get(p+68,4);
 if(quality>ZARC_DIAG_FAULT||severity>ZARC_DIAG_CRITICAL)return false;
 *f=(zarc_fault){.version=(uint32_t)get(p,4),.code=(uint32_t)get(p+4,4),.service=(uint32_t)get(p+8,4),
 .occurrences=(uint32_t)get(p+12,4),.epoch=get(p+16,8),.generation=get(p+24,8),.timestamp=get(p+32,8),
 .correlation=get(p+40,8),.config_digest=get(p+48,8),.build_digest=get(p+56,8),
 .quality=(zarc_diag_quality)quality,.severity=(zarc_diag_severity)severity};
 return valid(f);
}
bool zarc_diag_store_encode(const zarc_diagnostics *d,uint64_t revision,uint8_t *p,size_t n){
 if(!d||!p||n!=ZARC_DIAG_STORE_SIZE||!revision||d->count>ZARC_FAULT_LIMIT)return false;
 for(size_t i=0;i<d->count;++i)if(!valid(&d->records[i]))return false;
 memset(p,0,n);p[0]='Z';p[1]='D';p[2]='J';p[3]=1;
 put(p+4,8,revision);put(p+12,4,d->count);put(p+16,4,d->rejected);p[20]=d->critical_overflow?1u:0u;
 for(size_t i=0;i<d->count;++i)encode_record(p+ZARC_DIAG_STORE_HEADER+i*ZARC_DIAG_STORE_RECORD,&d->records[i]);
 put(p+24,4,crc(p,n));return true;
}
bool zarc_diag_store_decode(const uint8_t *p,size_t n,zarc_diagnostics *d,uint64_t *revision){
 if(!p||!d||!revision||n!=ZARC_DIAG_STORE_SIZE||p[0]!='Z'||p[1]!='D'||p[2]!='J'||p[3]!=1||
    p[20]>1||p[21]||p[22]||p[23]||get(p+24,4)!=crc(p,n))return false;
 uint64_t count=get(p+12,4),rev=get(p+4,8);
 if(count>ZARC_FAULT_LIMIT||!rev)return false;
 zarc_diagnostics next={.count=(size_t)count,.rejected=(uint32_t)get(p+16,4),.critical_overflow=p[20]!=0};
 for(size_t i=0;i<next.count;++i){
  if(!decode_record(p+ZARC_DIAG_STORE_HEADER+i*ZARC_DIAG_STORE_RECORD,&next.records[i]))return false;
  /* Reconstructing duplicate coalescing identities is invalid; compare with
   * the same insertion logic rather than trusting a serialized count. */
  zarc_diagnostics check={0};
  for(size_t j=0;j<i;++j)check.records[j]=next.records[j];
  check.count=i;
  if(zarc_diag_record(&check,&next.records[i])!=ZARC_DIAG_STORED)return false;
 }
 for(size_t i=ZARC_DIAG_STORE_HEADER+next.count*ZARC_DIAG_STORE_RECORD;i<n;++i)if(p[i])return false;
 *d=next;*revision=rev;return true;
}
