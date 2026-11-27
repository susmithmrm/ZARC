#include "maintenance_fixture.h"
#include "storage.h"
#include "zarc_manifest.h"
#include <zephyr/drivers/flash.h>
#include <zephyr/storage/flash_map.h>
#include <errno.h>
#include <string.h>
/* Per-profile keys in the simulator's dedicated NVS area. This is not a
 * secure monotonic counter: cloning or losing simulator flash can reuse IDs. */
#define BOOT_KEY ((uint16_t)(200u+2u*ZARC_PROFILE_KIND))
#define DIAG_KEY ((uint16_t)(BOOT_KEY+1u))
static struct nvs_fs fs;
static const struct flash_area *area;
static struct zarc_native_diag_store journal;
static uint64_t epoch;
static uint64_t decode(const uint8_t *p){
 uint64_t value=0;for(size_t i=0;i<8;++i)value|=(uint64_t)p[i]<<(8u*i);return value;
}
static int read_epoch(struct nvs_fs *store,uint64_t *out){
 uint8_t bytes[8];ssize_t n=nvs_read(store,BOOT_KEY,bytes,sizeof bytes);
 if(n<0)return (int)n;
 if(n!=(ssize_t)sizeof bytes||!decode(bytes))return -EBADMSG;
 *out=decode(bytes);return 0;
}
static bool open_partition(void){
 if(flash_area_open(FIXED_PARTITION_ID(storage_partition),&area)!=0)return false;
 const struct device *dev=flash_area_get_device(area);struct flash_pages_info page;
 if(!device_is_ready(dev)||flash_get_page_info_by_offs(dev,area->fa_off,&page)!=0||
    page.size>UINT16_MAX||!page.size||area->fa_size/page.size<2)return false;
 fs=(struct nvs_fs){.offset=area->fa_off,.sector_size=(uint16_t)page.size,.sector_count=2,.flash_device=dev};
 return nvs_mount(&fs)==0;
}
bool zarc_profile_maintenance_open(uint64_t *boot,zarc_diagnostics *restored){
 if(!boot||!restored||area||!open_partition())return false;
 uint64_t previous=0;int result=read_epoch(&fs,&previous);
 if(result!=0&&result!=-ENOENT)return false;
 if(previous==UINT64_MAX)return false;
 epoch=previous+1u;
 journal=(struct zarc_native_diag_store){.fs=&fs,.key=DIAG_KEY};
 zarc_diagnostics loaded={0};
 result=zarc_native_diag_load(&journal,&loaded);
 if(result==-ENOENT)journal.writable=true;
 else if(result!=0||!previous)return false;
 for(size_t i=0;i<loaded.count;++i)if(loaded.records[i].epoch>previous)return false;
 uint8_t bytes[8];
 for(size_t i=0;i<8;++i)bytes[i]=(uint8_t)(epoch>>(8u*i));
 ssize_t written=nvs_write(&fs,BOOT_KEY,bytes,sizeof bytes);
 if(written!=(ssize_t)sizeof bytes||read_epoch(&fs,boot)!=0||*boot!=epoch)return false;
 *restored=loaded;return true;
}
bool zarc_profile_maintenance_close(const zarc_diagnostics *snapshot){
 if(!area||!snapshot||!epoch)return false;
 bool ok=zarc_native_diag_persist(&journal,snapshot)==0;
 /* Remount and verify the exact serialized journal at shutdown, independent
  * of the in-memory store. No erase or reinitialization on corruption. */
 if(ok){
  struct nvs_fs remount={.offset=fs.offset,.sector_size=fs.sector_size,.sector_count=fs.sector_count,.flash_device=fs.flash_device};
  struct zarc_native_diag_store check={.fs=&remount,.key=DIAG_KEY};
  zarc_diagnostics loaded;uint64_t boot;
  uint8_t expected[ZARC_DIAG_STORE_SIZE],actual[ZARC_DIAG_STORE_SIZE];
  ok=nvs_mount(&remount)==0&&read_epoch(&remount,&boot)==0&&boot==epoch&&
     zarc_native_diag_load(&check,&loaded)==0&&check.revision==journal.revision&&
     zarc_diag_store_encode(snapshot,journal.revision,expected,sizeof expected)&&
     zarc_diag_store_encode(&loaded,check.revision,actual,sizeof actual)&&
     memcmp(expected,actual,sizeof expected)==0;
 }
 flash_area_close(area);area=NULL;epoch=0;return ok;
}
