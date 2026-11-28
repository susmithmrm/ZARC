#include "zarc/advisory.h"
#include "zarc/runtime.h"
#include <stdio.h>
#include <string.h>

static unsigned failures;
#define CHECK(x) do { if(!(x)){fprintf(stderr,"FAIL %s:%d: %s\n",__FILE__,__LINE__,#x);++failures;} } while(0)

static void test_init_and_validation(void){
 zarc_advisory_store store={0};uint8_t storage[4];
 CHECK(!zarc_advisory_init(NULL,storage,sizeof storage));
 CHECK(!zarc_advisory_init(&store,NULL,sizeof storage));
 CHECK(!zarc_advisory_init(&store,storage,0));
 CHECK(zarc_advisory_set_available(&store,true)==ZARC_ADVISORY_INVALID);
 CHECK(zarc_advisory_publish(&store,storage,1)==ZARC_ADVISORY_INVALID);
}

static void test_capacity_boundaries_and_atomic_rejection(void){
 uint8_t storage[4]={0xA1,0xA2,0xA3,0xA4};
 uint8_t input[5]={1,2,3,4,5},out[5]={0xCC,0xCC,0xCC,0xCC,0xCC};
 size_t length=99;zarc_advisory_store store={0};
 CHECK(zarc_advisory_init(&store,storage,sizeof storage));
 CHECK(zarc_advisory_set_available(&store,true)==ZARC_ADVISORY_OK);
 CHECK(zarc_advisory_publish(&store,input,0)==ZARC_ADVISORY_INVALID);
 CHECK(zarc_advisory_publish(&store,NULL,1)==ZARC_ADVISORY_INVALID);
 CHECK(zarc_advisory_publish(&store,input,sizeof input)==ZARC_ADVISORY_CAPACITY);
 CHECK(store.length==0);
 CHECK(storage[0]==0xA1&&storage[1]==0xA2&&storage[2]==0xA3&&storage[3]==0xA4);
 CHECK(zarc_advisory_publish(&store,input,sizeof storage)==ZARC_ADVISORY_OK);
 CHECK(store.length==sizeof storage);
 CHECK(zarc_advisory_read(&store,out,sizeof storage-1,&length)==ZARC_ADVISORY_CAPACITY);
 CHECK(length==sizeof storage);
 CHECK(out[0]==0xCC&&out[3]==0xCC);
 CHECK(zarc_advisory_read(&store,out,sizeof storage,&length)==ZARC_ADVISORY_OK);
 CHECK(length==sizeof storage&&memcmp(out,input,sizeof storage)==0);
 CHECK(zarc_advisory_read(&store,NULL,sizeof out,&length)==ZARC_ADVISORY_INVALID);
 CHECK(zarc_advisory_read(&store,out,sizeof out,NULL)==ZARC_ADVISORY_INVALID);
}

static void test_overlap_and_optional_service_failure(void){
 uint8_t storage[4]={10,20,30,40},out[4]={0xCC,0xCC,0xCC,0xCC};
 size_t length=99;zarc_advisory_store store={0};
 CHECK(zarc_advisory_init(&store,storage,sizeof storage));
 CHECK(zarc_advisory_set_available(&store,true)==ZARC_ADVISORY_OK);
 CHECK(zarc_advisory_publish(&store,storage+1,3)==ZARC_ADVISORY_OK);
 CHECK(storage[0]==20&&storage[1]==30&&storage[2]==40);
 CHECK(zarc_advisory_read(&store,out,sizeof out,&length)==ZARC_ADVISORY_OK);
 CHECK(length==3&&out[0]==20&&out[1]==30&&out[2]==40);

 CHECK(zarc_advisory_set_available(&store,false)==ZARC_ADVISORY_OK);
 CHECK(store.length==0);
 CHECK(zarc_advisory_publish(&store,out,3)==ZARC_ADVISORY_UNAVAILABLE);
 length=99;
 CHECK(zarc_advisory_read(&store,out,sizeof out,&length)==ZARC_ADVISORY_UNAVAILABLE);
 CHECK(length==0&&out[0]==20&&out[1]==30&&out[2]==40);

 CHECK(zarc_advisory_set_available(&store,true)==ZARC_ADVISORY_OK);
 CHECK(zarc_advisory_read(&store,out,sizeof out,&length)==ZARC_ADVISORY_OK);
 CHECK(length==0);
 CHECK(zarc_advisory_publish(&store,out,3)==ZARC_ADVISORY_OK);
 CHECK(zarc_advisory_read(&store,out,sizeof out,&length)==ZARC_ADVISORY_OK);
 CHECK(length==3&&out[0]==20&&out[1]==30&&out[2]==40);
}

static void test_optional_service_failure_does_not_affect_runtime(void){
 zarc_runtime runtime={0};
 const zarc_runtime_config config={.endpoint={1,30},.period=10,.deadline=8,
  .health_limit=25,.max_validity=50,.config_digest=1,.build_digest=2};
 zarc_advisory_store store={0};uint8_t storage[4],payload[2]={0xA5,0x5A};
 zarc_observation observation;
 CHECK(zarc_advisory_init(&store,storage,sizeof storage));
 CHECK(zarc_runtime_init(&runtime,&config,77,0));
 CHECK(zarc_advisory_set_available(&store,true)==ZARC_ADVISORY_OK);
 CHECK(zarc_advisory_publish(&store,payload,sizeof payload)==ZARC_ADVISORY_OK);
 CHECK(zarc_advisory_set_available(&store,false)==ZARC_ADVISORY_OK);
 CHECK(zarc_runtime_step(&runtime,0,true,1,0,ZARC_ON)&&runtime.healthy);
 CHECK(zarc_controller_snapshot(&runtime.controller,1,&observation));
 CHECK(observation.value==ZARC_ON);
 CHECK(zarc_runtime_local(&runtime,ZARC_OFF,true,1,20,true,
                          &(zarc_controller_effect){0})==ZARC_CONN_ACCEPTED);
}

int main(void){
 test_init_and_validation();
 test_capacity_boundaries_and_atomic_rejection();
 test_overlap_and_optional_service_failure();
 test_optional_service_failure_does_not_affect_runtime();
 if(failures){fprintf(stderr,"%u advisory test(s) failed\n",failures);return 1;}
 puts("Advisory bounded storage contract passed");
 return 0;
}
