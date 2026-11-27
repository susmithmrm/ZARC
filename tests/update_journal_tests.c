#include "zarc/update_journal.h"
#include <stdio.h>
#define CHECK(x) do{if(!(x)){fprintf(stderr,"line %d: %s\n",__LINE__,#x);return 1;}}while(0)
int main(void){
 zarc_update u,restored;zarc_config_store store={0};
 const zarc_update_policy p={1,2,3,1},wrong={1,3,3,1};
 const zarc_image active={11,1,1,2,3},candidate={12,2,1,2,3};
 size_t slot;uint64_t revision;
 CHECK(zarc_update_init(&u,&p,&active,true));
 CHECK(zarc_update_journal_stage(&store,1,&u,&slot)==ZARC_CONFIG_OK);
 CHECK(zarc_update_journal_commit(&store,slot,1)==ZARC_CONFIG_OK);
 CHECK(zarc_update_stage(&u,&candidate,true,true,true)==ZARC_UPDATE_OK);
 CHECK(zarc_update_trial_request(&u,true)==ZARC_UPDATE_OK);
 CHECK(zarc_update_boot_report(&u,&candidate,true)==ZARC_UPDATE_OK);
 CHECK(zarc_update_prepare_confirmation(&u,true,true,true)==ZARC_UPDATE_OK&&u.phase==ZARC_UPDATE_CONFIRMING);
 CHECK(zarc_update_journal_stage(&store,2,&u,&slot)==ZARC_CONFIG_OK);
 CHECK(zarc_update_journal_load(&store,&p,&restored,&revision)==ZARC_CONFIG_OK&&revision==1);
 CHECK(zarc_update_journal_commit(&store,slot,2)==ZARC_CONFIG_OK);
 CHECK(zarc_update_journal_load(&store,&wrong,&restored,&revision)==ZARC_CONFIG_UNAVAILABLE);
 CHECK(zarc_update_journal_load(&store,&p,&restored,&revision)==ZARC_CONFIG_OK&&revision==2);
 CHECK(zarc_update_reconcile_confirmation(&restored,&candidate,true,false)==ZARC_UPDATE_DENIED);
 CHECK(restored.phase==ZARC_UPDATE_RECOVERY_REQUIRED);
 CHECK(zarc_update_journal_load(&store,&p,&restored,&revision)==ZARC_CONFIG_OK);
 CHECK(zarc_update_reconcile_confirmation(&restored,&candidate,true,true)==ZARC_UPDATE_OK);
 CHECK(restored.active.image_id==12&&restored.phase==ZARC_UPDATE_ACTIVE);
 CHECK(zarc_update_journal_stage(&store,3,&restored,&slot)==ZARC_CONFIG_OK);
 CHECK(zarc_update_journal_commit(&store,slot,3)==ZARC_CONFIG_OK);
 CHECK(zarc_update_journal_load(&store,&p,&restored,&revision)==ZARC_CONFIG_OK&&revision==3);
 CHECK(restored.active.version==2);
 CHECK(zarc_update_reconcile_confirmation(&restored,&active,true,true)==ZARC_UPDATE_CONFLICT);
 CHECK(zarc_update_journal_stage(&store,3,&restored,&slot)==ZARC_CONFIG_CONFLICT);
 puts("Versioned update journal and confirmation recovery contracts passed");return 0;
}
