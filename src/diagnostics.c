#include "zarc/diagnostics.h"
static bool same(const zarc_fault *a,const zarc_fault *b){
 return a->code==b->code&&a->service==b->service&&a->epoch==b->epoch&&
 a->severity==b->severity&&a->quality==b->quality&&a->correlation==b->correlation&&
 a->config_digest==b->config_digest&&a->build_digest==b->build_digest;
}
zarc_diag_result zarc_diag_record(zarc_diagnostics *d,const zarc_fault *f){
 if(!d||!f||d->count>ZARC_FAULT_LIMIT||f->version!=ZARC_DIAG_VERSION||!f->code||!f->service||!f->epoch||
 (unsigned)f->quality>ZARC_DIAG_FAULT||(unsigned)f->severity>ZARC_DIAG_CRITICAL)return ZARC_DIAG_INVALID;
 for(size_t i=0;i<d->count;++i){
  zarc_fault *r=&d->records[i];if(!same(r,f))continue;
  if(f->timestamp<r->timestamp||f->generation<r->generation)return ZARC_DIAG_INVALID;
  if(r->occurrences<UINT32_MAX)++r->occurrences;
  r->timestamp=f->timestamp;r->generation=f->generation;return ZARC_DIAG_COALESCED;
 }
 if(d->count==ZARC_FAULT_LIMIT){
  if(d->rejected<UINT32_MAX)++d->rejected;
  if(f->severity==ZARC_DIAG_CRITICAL)d->critical_overflow=true;
  return ZARC_DIAG_FULL;
 }
 d->records[d->count]=*f;d->records[d->count].occurrences=1;++d->count;return ZARC_DIAG_STORED;
}
bool zarc_diag_read(const zarc_diagnostics *d,size_t i,zarc_fault *out){
 if(!d||!out||d->count>ZARC_FAULT_LIMIT||i>=d->count)return false;
 *out=d->records[i];return true;
}
