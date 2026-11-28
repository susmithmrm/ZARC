#include "zarc/calibration.h"
#include <stdio.h>
#include <string.h>

#define CHECK(x) do { if (!(x)) { fprintf(stderr, "line %d: %s\n", __LINE__, #x); return 1; } } while (0)

static zarc_calibration_record golden(void) {
 return (zarc_calibration_record){
  .version=ZARC_CALIBRATION_VERSION,.sensor_id=UINT32_C(0x10203040),
  .unit_id=2u,.calibration_id=7u,.scale10=-1,
  .raw_min=0,.raw_max=1000,.minimum=-2,.maximum=10000,
  .gain_num=10,.gain_den=1u,.offset=-2,
  .provenance_id=UINT64_C(0x0102030405060708)
 };
}

static bool same_record(const zarc_calibration_record *a,const zarc_calibration_record *b) {
 return a->version==b->version&&a->sensor_id==b->sensor_id&&a->unit_id==b->unit_id&&
        a->calibration_id==b->calibration_id&&a->scale10==b->scale10&&
        a->raw_min==b->raw_min&&a->raw_max==b->raw_max&&a->minimum==b->minimum&&
        a->maximum==b->maximum&&a->gain_num==b->gain_num&&a->gain_den==b->gain_den&&
        a->offset==b->offset&&a->provenance_id==b->provenance_id;
}

int main(void) {
 const uint8_t expected[ZARC_CALIBRATION_WIRE_SIZE]={
  0x01,0x00,0x00,0x00,0x40,0x30,0x20,0x10,0x02,0x00,0x00,0x00,
  0x07,0x00,0x00,0x00,0xff,0x00,0x00,0x00,0x00,0xe8,0x03,0x00,
  0x00,0xfe,0xff,0xff,0xff,0x10,0x27,0x00,0x00,0x0a,0x00,0x00,
  0x00,0x01,0x00,0x00,0x00,0xfe,0xff,0xff,0xff,0x08,0x07,0x06,
  0x05,0x04,0x03,0x02,0x01
 };
 zarc_calibration_record record=golden(),decoded={0};
 uint8_t wire[ZARC_CALIBRATION_WIRE_SIZE];
 CHECK(zarc_calibration_validate(&record));
 CHECK(zarc_calibration_encode(&record,wire,sizeof wire));
 CHECK(memcmp(wire,expected,sizeof expected)==0);
 CHECK(!zarc_calibration_encode(&record,wire,sizeof wire-1u));
 CHECK(zarc_calibration_decode(wire,sizeof wire,&decoded));
 CHECK(memcmp(&record,&decoded,sizeof record)==0);
 CHECK(!zarc_calibration_decode(wire,sizeof wire-1u,&decoded));
 CHECK(!zarc_calibration_decode(wire,sizeof wire+1u,&decoded));
 decoded=record;
 uint8_t malformed[ZARC_CALIBRATION_WIRE_SIZE];memcpy(malformed,wire,sizeof malformed);
 malformed[0]=2u;
 CHECK(!zarc_calibration_decode(malformed,sizeof malformed,&decoded));
 CHECK(same_record(&decoded,&record));

 int32_t value=123;
 CHECK(zarc_calibration_apply(&record,record.sensor_id,0,&value)&&value==-2);
 CHECK(zarc_calibration_apply(&record,record.sensor_id,321,&value)&&value==3208);
 CHECK(zarc_calibration_apply(&record,record.sensor_id,1000,&value)&&value==9998);
 value=123;
 CHECK(!zarc_calibration_apply(&record,record.sensor_id+1u,10,&value)&&value==123);
 CHECK(!zarc_calibration_apply(&record,record.sensor_id,-1,&value)&&value==123);
 CHECK(!zarc_calibration_apply(&record,record.sensor_id,1001,&value)&&value==123);
 CHECK(!zarc_calibration_apply(&record,record.sensor_id,10,NULL));

 zarc_calibration_record invalid=record;
 invalid.version=2u;CHECK(!zarc_calibration_validate(&invalid));
 invalid=record;invalid.provenance_id=0u;CHECK(!zarc_calibration_validate(&invalid));
 invalid=record;invalid.scale10=10;CHECK(!zarc_calibration_validate(&invalid));
 invalid=record;invalid.raw_min=1;invalid.raw_max=0;CHECK(!zarc_calibration_validate(&invalid));
 invalid=record;invalid.gain_den=0u;CHECK(!zarc_calibration_validate(&invalid));
 invalid=record;invalid.minimum=0;CHECK(!zarc_calibration_validate(&invalid));
 invalid=record;invalid.gain_num=INT32_MAX;invalid.gain_den=1u;
 invalid.raw_min=0;invalid.raw_max=INT32_MAX;invalid.minimum=0;invalid.maximum=INT32_MAX;
 CHECK(!zarc_calibration_validate(&invalid));

 zarc_config_store store={0},before;
 zarc_calibration_record loaded={0};
 uint64_t revision=0u;size_t slot=0u;
 CHECK(zarc_calibration_load(&store,record.sensor_id,&loaded,&revision)==ZARC_CONFIG_UNAVAILABLE);
 before=store;
 CHECK(zarc_calibration_stage(&store,&record,1u,false,&slot)==ZARC_CONFIG_UNAVAILABLE);
 CHECK(memcmp(&before,&store,sizeof store)==0);
 CHECK(zarc_calibration_stage(&store,&record,1u,true,&slot)==ZARC_CONFIG_OK);
 CHECK(zarc_calibration_load(&store,record.sensor_id,&loaded,&revision)==ZARC_CONFIG_UNAVAILABLE);
 CHECK(zarc_calibration_commit(&store,slot,1u,false)==ZARC_CONFIG_UNAVAILABLE);
 CHECK(zarc_calibration_commit(&store,slot,1u,true)==ZARC_CONFIG_OK);
 CHECK(zarc_calibration_load(&store,record.sensor_id,&loaded,&revision)==ZARC_CONFIG_OK);
 CHECK(revision==1u&&same_record(&loaded,&record));

 zarc_calibration_record next=record;next.calibration_id=8u;next.provenance_id=9u;next.offset=0;
 next.minimum=0;next.maximum=10000;
 CHECK(zarc_calibration_validate(&next));
 CHECK(zarc_calibration_stage(&store,&next,2u,true,&slot)==ZARC_CONFIG_OK);
 /* Simulated interruption after stage and before commit preserves old record. */
 CHECK(zarc_calibration_load(&store,record.sensor_id,&loaded,&revision)==ZARC_CONFIG_OK);
 CHECK(revision==1u&&loaded.calibration_id==record.calibration_id);
 CHECK(zarc_calibration_commit(&store,slot,2u,true)==ZARC_CONFIG_OK);
 CHECK(zarc_calibration_load(&store,record.sensor_id,&loaded,&revision)==ZARC_CONFIG_OK);
 CHECK(revision==2u&&same_record(&loaded,&next));
 CHECK(zarc_calibration_load(&store,record.sensor_id+1u,&loaded,&revision)==ZARC_CONFIG_UNAVAILABLE);
 puts("Versioned calibration, golden encoding and staged recovery contracts passed");
 return 0;
}
