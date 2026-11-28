#include "zarc/advisory.h"
#include <string.h>

static bool initialized(const zarc_advisory_store *store){
 return store&&store->storage&&store->capacity>0&&store->length<=store->capacity;
}

bool zarc_advisory_init(zarc_advisory_store *store,uint8_t *storage,size_t capacity){
 if(!store||!storage||capacity==0)return false;
 *store=(zarc_advisory_store){.storage=storage,.capacity=capacity};
 return true;
}

zarc_advisory_result zarc_advisory_set_available(zarc_advisory_store *store,bool available){
 if(!initialized(store))return ZARC_ADVISORY_INVALID;
 store->available=available;
 if(!available)store->length=0;
 return ZARC_ADVISORY_OK;
}

zarc_advisory_result zarc_advisory_publish(zarc_advisory_store *store,
                                            const uint8_t *data,size_t length){
 if(!initialized(store)||!data||length==0)return ZARC_ADVISORY_INVALID;
 if(!store->available)return ZARC_ADVISORY_UNAVAILABLE;
 if(length>store->capacity)return ZARC_ADVISORY_CAPACITY;
 memmove(store->storage,data,length);
 store->length=length;
 return ZARC_ADVISORY_OK;
}

zarc_advisory_result zarc_advisory_read(const zarc_advisory_store *store,
                                         uint8_t *out,size_t capacity,size_t *length){
 if(!initialized(store)||!out||!length)return ZARC_ADVISORY_INVALID;
 if(!store->available){*length=0;return ZARC_ADVISORY_UNAVAILABLE;}
 if(store->length>capacity){*length=store->length;return ZARC_ADVISORY_CAPACITY;}
 if(store->length)memcpy(out,store->storage,store->length);
 *length=store->length;
 return ZARC_ADVISORY_OK;
}
