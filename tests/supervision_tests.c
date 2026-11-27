#include "zarc/health.h"
#include "zarc/diagnostics.h"
#include "zarc/config_store.h"
#include <stdio.h>
#define CHECK(x) do{if(!(x)){fprintf(stderr,"line %d: %s\n",__LINE__,#x);return 1;}}while(0)
int main(void){
 zarc_health h;zarc_health_policy p[]={{1,5,10,true},{2,2,2,false}};
 zarc_health_snapshot status;
 CHECK(zarc_health_init(&h,p,2,9,0));
 CHECK(zarc_health_check(&h,0,&status)&&!status.required_healthy);
 CHECK(!zarc_health_report(&h,1,8,1,1,true,true));
 CHECK(zarc_health_report(&h,1,9,1,1,true,true));
 CHECK(zarc_health_check(&h,1,&status)&&status.required_healthy&&status.alive_failed==2);
 CHECK(!zarc_health_report(&h,1,9,1,2,true,true));
 CHECK(zarc_health_report(&h,1,9,2,5,false,true));
 CHECK(zarc_health_check(&h,10,&status)&&!status.required_healthy&&status.alive_failed==3&&status.deadline_failed==2);
 CHECK(zarc_health_report(&h,1,9,3,11,false,false));
 CHECK(zarc_health_check(&h,11,&status)&&status.deadline_failed==3&&status.logical_failed==3);
 CHECK(!zarc_health_check(&h,10,&status));
 zarc_diagnostics d={0};zarc_fault f={.version=1,.code=1,.service=1,.epoch=9,.severity=ZARC_DIAG_ERROR,.quality=ZARC_DIAG_FAULT};
 CHECK(zarc_diag_record(&d,&f)==ZARC_DIAG_STORED);
 f.timestamp=1;CHECK(zarc_diag_record(&d,&f)==ZARC_DIAG_COALESCED);
 zarc_fault read;CHECK(zarc_diag_read(&d,0,&read)&&read.occurrences==2);
 for(uint32_t i=2;i<=ZARC_FAULT_LIMIT;++i){f.code=i;CHECK(zarc_diag_record(&d,&f)==ZARC_DIAG_STORED);}
 f.code=99;f.severity=ZARC_DIAG_CRITICAL;
 CHECK(zarc_diag_record(&d,&f)==ZARC_DIAG_FULL&&d.critical_overflow&&d.rejected==1);
 zarc_config_store store={0};zarc_config config;size_t first,second;uint8_t a[]={1,2},b[]={3,4};
 CHECK(zarc_config_load(&store,1,&config)==ZARC_CONFIG_UNAVAILABLE);
 CHECK(zarc_config_stage(&store,1,1,a,sizeof a,&first)==ZARC_CONFIG_OK);
 CHECK(zarc_config_load(&store,1,&config)==ZARC_CONFIG_UNAVAILABLE);
 CHECK(zarc_config_commit(&store,first,1,1)==ZARC_CONFIG_OK);
 CHECK(zarc_config_stage(&store,1,2,b,sizeof b,&second)==ZARC_CONFIG_OK&&first!=second);
 CHECK(zarc_config_load(&store,1,&config)==ZARC_CONFIG_OK&&config.bytes[0]==1);
 CHECK(zarc_config_commit(&store,second,1,2)==ZARC_CONFIG_OK);
 CHECK(zarc_config_load(&store,1,&config)==ZARC_CONFIG_OK&&config.bytes[0]==3);
 CHECK(zarc_config_stage(&store,1,1,a,sizeof a,&second)==ZARC_CONFIG_CONFLICT);
 CHECK(zarc_config_stage(&store,2,3,b,sizeof b,&second)==ZARC_CONFIG_OK);
 CHECK(zarc_config_commit(&store,second,2,3)==ZARC_CONFIG_OK);
 CHECK(zarc_config_load(&store,1,&config)==ZARC_CONFIG_UNAVAILABLE);
 CHECK(zarc_config_load(&store,2,&config)==ZARC_CONFIG_OK&&config.revision==3);
 store.slots[second][ZARC_CONFIG_HEADER]^=1u;
 CHECK(zarc_config_load(&store,1,&config)==ZARC_CONFIG_OK&&config.revision==2);
 puts("Supervision and storage contracts passed");return 0;
}
