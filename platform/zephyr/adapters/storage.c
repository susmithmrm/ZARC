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

int zarc_native_update_journal_persist(void *context,const zarc_update *u){
 struct zarc_native_update_journal *j=context;
 if(!j||!j->fs||!j->writable||j->revision==UINT64_MAX)return -EINVAL;
 zarc_config_store next=j->store;
 uint64_t revision=j->revision+1u;size_t slot;
 if(zarc_update_journal_stage(&next,revision,u,&slot)!=ZARC_CONFIG_OK)return -EINVAL;
 int ret=zarc_native_config_write(j->fs,j->key,&next);
 if(ret<0){j->writable=false;return ret;}
 if(zarc_update_journal_commit(&next,slot,revision)!=ZARC_CONFIG_OK){j->writable=false;return -EIO;}
 ret=zarc_native_config_write(j->fs,j->key,&next);
 if(ret<0){j->writable=false;return ret;}
 j->store=next;j->revision=revision;return 0;
}

int zarc_native_diag_load(struct zarc_native_diag_store *j,zarc_diagnostics *d){
 if(!j||!j->fs||!d)return -EINVAL;
 j->writable=false;
 uint8_t bytes[ZARC_DIAG_STORE_SIZE];
 ssize_t n=nvs_read(j->fs,j->key,bytes,sizeof bytes);
 if(n<0)return (int)n;
 if((size_t)n!=sizeof bytes||!zarc_diag_store_decode(bytes,sizeof bytes,d,&j->revision))return -EBADMSG;
 j->writable=true;return 0;
}
int zarc_native_diag_persist(struct zarc_native_diag_store *j,const zarc_diagnostics *d){
 if(!j||!j->fs||!j->writable||j->revision==UINT64_MAX)return -EINVAL;
 uint8_t bytes[ZARC_DIAG_STORE_SIZE],check[ZARC_DIAG_STORE_SIZE];
 uint64_t rev=j->revision+1u;
 if(!zarc_diag_store_encode(d,rev,bytes,sizeof bytes))return -EINVAL;
 ssize_t n=nvs_write(j->fs,j->key,bytes,sizeof bytes);
 if(n<0||(n!=0&&(size_t)n!=sizeof bytes)){j->writable=false;return n<0?(int)n:-EIO;}
 n=nvs_read(j->fs,j->key,check,sizeof check);
 if(n<0||(size_t)n!=sizeof check||memcmp(bytes,check,sizeof bytes)!=0){j->writable=false;return n<0?(int)n:-EIO;}
 j->revision=rev;return 0;
}
