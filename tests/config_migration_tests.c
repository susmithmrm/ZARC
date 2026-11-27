#include "zarc/config_store.h"
#include <stdio.h>
#include <string.h>
#define CHECK(x) do{if(!(x)){fprintf(stderr,"line %d: %s\n",__LINE__,#x);return 1;}}while(0)
static bool map(void *ctx,const zarc_config *old,uint8_t *out,size_t cap,size_t *n){
 if(!ctx||old->schema!=1||old->length!=2||cap<3)return false;
 out[0]=2;out[1]=old->bytes[0];out[2]=old->bytes[1];*n=3;return true;
}
int main(void){
 zarc_config_store store={0},before;zarc_config config;zarc_config_migration ticket;
 size_t slot;const uint8_t old[]={7,8},bad[]={1,2,3};
 CHECK(zarc_config_stage(&store,1,1,old,sizeof old,&slot)==ZARC_CONFIG_OK);
 CHECK(zarc_config_commit(&store,slot,1,1)==ZARC_CONFIG_OK);
 before=store;
 CHECK(zarc_config_prepare_migration(&store,1,2,2,false,map,&store,&ticket)==ZARC_CONFIG_UNAVAILABLE);
 CHECK(memcmp(&before,&store,sizeof store)==0);
 CHECK(zarc_config_prepare_migration(&store,1,2,2,true,map,NULL,&ticket)==ZARC_CONFIG_INVALID);
 CHECK(memcmp(&before,&store,sizeof store)==0);
 CHECK(zarc_config_prepare_migration(&store,1,2,2,true,map,&store,&ticket)==ZARC_CONFIG_OK);
 CHECK(zarc_config_load(&store,1,&config)==ZARC_CONFIG_OK&&config.revision==1);
 CHECK(zarc_config_stage(&store,2,2,bad,sizeof bad,&slot)==ZARC_CONFIG_OK);
 CHECK(zarc_config_commit_migration(&store,&ticket)==ZARC_CONFIG_CONFLICT);
 CHECK(zarc_config_prepare_migration(&store,1,2,2,true,map,&store,&ticket)==ZARC_CONFIG_OK);
 CHECK(zarc_config_commit_migration(&store,&ticket)==ZARC_CONFIG_OK);
 CHECK(zarc_config_commit_migration(&store,&ticket)==ZARC_CONFIG_OK);
 CHECK(zarc_config_load(&store,1,&config)==ZARC_CONFIG_UNAVAILABLE);
 CHECK(zarc_config_load(&store,2,&config)==ZARC_CONFIG_OK&&config.length==3&&config.bytes[1]==7);
 puts("Explicit configuration migration and stale-ticket contracts passed");return 0;
}
