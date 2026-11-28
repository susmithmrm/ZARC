#include "zarc/calibration.h"
#include "zarc/sensor.h"
#include <limits.h>
#include <string.h>

static void write_u32(uint8_t *p,uint32_t value) {
 for(size_t i=0;i<4u;++i)p[i]=(uint8_t)(value>>(8u*i));
}
static uint32_t read_u32(const uint8_t *p) {
 uint32_t value=0u;
 for(size_t i=0;i<4u;++i)value|=(uint32_t)p[i]<<(8u*i);
 return value;
}
static void write_u64(uint8_t *p,uint64_t value) {
 for(size_t i=0;i<8u;++i)p[i]=(uint8_t)(value>>(8u*i));
}
static uint64_t read_u64(const uint8_t *p) {
 uint64_t value=0u;
 for(size_t i=0;i<8u;++i)value|=(uint64_t)p[i]<<(8u*i);
 return value;
}
static int32_t read_i32(const uint8_t *p) {
 uint32_t bits=read_u32(p);
 if(bits<=(uint32_t)INT32_MAX)return (int32_t)bits;
 return -1-(int32_t)(UINT32_MAX-bits);
}

static bool convert(const zarc_calibration_record *record,int32_t raw,int32_t *out) {
 int64_t product=(int64_t)raw*(int64_t)record->gain_num;
 int32_t scaled;
 if(!zarc_sensor_quantize_ratio(product,(int64_t)record->gain_den,&scaled))return false;
 int64_t value=(int64_t)scaled+(int64_t)record->offset;
 if(value<INT32_MIN||value>INT32_MAX)return false;
 *out=(int32_t)value;
 return true;
}

bool zarc_calibration_validate(const zarc_calibration_record *record) {
 if(!record||record->version!=ZARC_CALIBRATION_VERSION||!record->sensor_id||
    !record->unit_id||!record->calibration_id||record->scale10< -9||
    record->scale10>9||record->raw_min>record->raw_max||
    record->minimum>record->maximum||!record->gain_num||!record->gain_den||
    !record->provenance_id)return false;
 int32_t low,high;
 return convert(record,record->raw_min,&low)&&convert(record,record->raw_max,&high)&&
        low>=record->minimum&&low<=record->maximum&&
        high>=record->minimum&&high<=record->maximum;
}

bool zarc_calibration_encode(const zarc_calibration_record *record,uint8_t *out,size_t capacity) {
 if(!out||capacity<ZARC_CALIBRATION_WIRE_SIZE||!zarc_calibration_validate(record))return false;
 uint8_t next[ZARC_CALIBRATION_WIRE_SIZE]={0};
 write_u32(next,record->version);
 write_u32(next+4u,record->sensor_id);
 write_u32(next+8u,record->unit_id);
 write_u32(next+12u,record->calibration_id);
 next[16]=(uint8_t)record->scale10;
 write_u32(next+17u,(uint32_t)record->raw_min);
 write_u32(next+21u,(uint32_t)record->raw_max);
 write_u32(next+25u,(uint32_t)record->minimum);
 write_u32(next+29u,(uint32_t)record->maximum);
 write_u32(next+33u,(uint32_t)record->gain_num);
 write_u32(next+37u,record->gain_den);
 write_u32(next+41u,(uint32_t)record->offset);
 write_u64(next+45u,record->provenance_id);
 memcpy(out,next,sizeof next);
 return true;
}

bool zarc_calibration_decode(const uint8_t *bytes,size_t length,zarc_calibration_record *out) {
 if(!bytes||!out||length!=ZARC_CALIBRATION_WIRE_SIZE)return false;
 uint8_t scale=bytes[16];
 int8_t scale10=scale<=(uint8_t)INT8_MAX?(int8_t)scale:
                (int8_t)(-1-(int8_t)(UINT8_MAX-scale));
 zarc_calibration_record next={
  .version=read_u32(bytes),.sensor_id=read_u32(bytes+4u),
  .unit_id=read_u32(bytes+8u),.calibration_id=read_u32(bytes+12u),
  .scale10=scale10,.raw_min=read_i32(bytes+17u),.raw_max=read_i32(bytes+21u),
  .minimum=read_i32(bytes+25u),.maximum=read_i32(bytes+29u),
  .gain_num=read_i32(bytes+33u),.gain_den=read_u32(bytes+37u),
  .offset=read_i32(bytes+41u),.provenance_id=read_u64(bytes+45u)
 };
 if(!zarc_calibration_validate(&next))return false;
 *out=next;
 return true;
}

bool zarc_calibration_apply(const zarc_calibration_record *record,uint32_t sensor_id,
                            int32_t raw_value,int32_t *engineering_value) {
 if(!engineering_value||!zarc_calibration_validate(record)||
    sensor_id!=record->sensor_id||raw_value<record->raw_min||raw_value>record->raw_max)
  return false;
 int32_t next;
 if(!convert(record,raw_value,&next)||next<record->minimum||next>record->maximum)return false;
 *engineering_value=next;
 return true;
}

zarc_config_status zarc_calibration_load(const zarc_config_store *store,uint32_t sensor_id,
                                         zarc_calibration_record *out,uint64_t *revision) {
 if(!store||!sensor_id||!out)return ZARC_CONFIG_INVALID;
 zarc_config config;
 zarc_config_status status=zarc_config_load(store,ZARC_CALIBRATION_SCHEMA,&config);
 if(status!=ZARC_CONFIG_OK)return status;
 zarc_calibration_record next;
 if(!zarc_calibration_decode(config.bytes,config.length,&next))return ZARC_CONFIG_INVALID;
 if(next.sensor_id!=sensor_id)return ZARC_CONFIG_UNAVAILABLE;
 *out=next;
 if(revision)*revision=config.revision;
 return ZARC_CONFIG_OK;
}

zarc_config_status zarc_calibration_stage(zarc_config_store *store,
                                          const zarc_calibration_record *record,uint64_t revision,
                                          bool authorized,size_t *slot) {
 if(!store||!record||!revision||!slot)return ZARC_CONFIG_INVALID;
 if(!authorized)return ZARC_CONFIG_UNAVAILABLE;
 if(!zarc_calibration_validate(record))return ZARC_CONFIG_INVALID;
 uint8_t bytes[ZARC_CALIBRATION_WIRE_SIZE];
 if(!zarc_calibration_encode(record,bytes,sizeof bytes))return ZARC_CONFIG_INVALID;
 return zarc_config_stage(store,ZARC_CALIBRATION_SCHEMA,revision,bytes,sizeof bytes,slot);
}

zarc_config_status zarc_calibration_commit(zarc_config_store *store,size_t slot,
                                           uint64_t revision,bool authorized) {
 if(!store||!revision||slot>=2u)return ZARC_CONFIG_INVALID;
 if(!authorized)return ZARC_CONFIG_UNAVAILABLE;
 return zarc_config_commit(store,slot,ZARC_CALIBRATION_SCHEMA,revision);
}
