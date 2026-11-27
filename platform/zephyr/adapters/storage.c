#include "storage.h"
#include <errno.h>
#include <string.h>
int zarc_native_config_read(struct nvs_fs *fs,uint16_t key,zarc_config_store *out){
 if(!fs||!out)return -EINVAL;
 zarc_config_store temp;
 ssize_t n=nvs_read(fs,key,temp.slots,sizeof temp.slots);
 if(n<0)return (int)n;
 if((size_t)n!=sizeof temp.slots)return -EBADMSG;
 *out=temp;return 0;
}
int zarc_native_config_write(struct nvs_fs *fs,uint16_t key,const zarc_config_store *s){
 if(!fs||!s)return -EINVAL;
 ssize_t n=nvs_write(fs,key,s->slots,sizeof s->slots);
 if(n<0)return (int)n;
 if(n!=0&&(size_t)n!=sizeof s->slots)return -EIO;
 zarc_config_store check;int ret=zarc_native_config_read(fs,key,&check);
 if(ret<0)return ret;
 return memcmp(check.slots,s->slots,sizeof s->slots)==0?0:-EIO;
}
