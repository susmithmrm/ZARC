#include "zarc/diag_store.h"
#include <stdio.h>
#include <string.h>
#define CHECK(x) do{if(!(x)){fprintf(stderr,"line %d: %s\n",__LINE__,#x);return 1;}}while(0)
int main(void){
 zarc_diagnostics d={0},recovered={0};uint64_t rev=0;
 zarc_fault f={.version=1,.code=1,.service=2,.epoch=3,.timestamp=4,.generation=5,.correlation=6,
 .config_digest=7,.build_digest=8,.quality=ZARC_DIAG_FAULT,.severity=ZARC_DIAG_CRITICAL};
 uint8_t bytes[ZARC_DIAG_STORE_SIZE],again[ZARC_DIAG_STORE_SIZE];
 CHECK(zarc_diag_record(&d,&f)==ZARC_DIAG_STORED);
 ++f.timestamp;CHECK(zarc_diag_record(&d,&f)==ZARC_DIAG_COALESCED);
 for(uint32_t i=2;i<=ZARC_FAULT_LIMIT;++i){f.code=i;CHECK(zarc_diag_record(&d,&f)==ZARC_DIAG_STORED);}
 f.code=99;CHECK(zarc_diag_record(&d,&f)==ZARC_DIAG_FULL&&d.critical_overflow);
 CHECK(zarc_diag_store_encode(&d,12,bytes,sizeof bytes));
 CHECK(zarc_diag_store_decode(bytes,sizeof bytes,&recovered,&rev)&&rev==12);
 CHECK(recovered.count==ZARC_FAULT_LIMIT&&recovered.critical_overflow&&recovered.rejected==1);
 CHECK(recovered.records[0].occurrences==2&&recovered.records[0].timestamp==5);
 CHECK(zarc_diag_store_encode(&recovered,rev,again,sizeof again)&&memcmp(bytes,again,sizeof bytes)==0);
 CHECK(!zarc_diag_store_decode(bytes,sizeof bytes-1,&recovered,&rev));
 bytes[50]^=1;CHECK(!zarc_diag_store_decode(bytes,sizeof bytes,&recovered,&rev));
 CHECK(rev==12&&recovered.records[0].occurrences==2);
 puts("Diagnostic journal serialized retention and corruption contracts passed");return 0;
}
