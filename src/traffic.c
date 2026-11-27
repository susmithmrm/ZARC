#include "zarc/traffic.h"
bool zarc_traffic_push(zarc_traffic *q,const zarc_frame *f,bool core){
 if(!q||!f||f->length>ZARC_PAYLOAD_MAX)return false;
 size_t cap=core?ZARC_CORE_QUEUE:ZARC_BULK_QUEUE;
 size_t *count=core?&q->core_count:&q->bulk_count;
 size_t *peak=core?&q->core_peak:&q->bulk_peak;
 size_t head=core?q->core_head:q->bulk_head;
 uint32_t *rejected=core?&q->core_rejected:&q->bulk_rejected;
 zarc_frame *frames=core?q->core:q->bulk;
 if(*count>=cap){if(*rejected<UINT32_MAX)++*rejected;return false;}
 frames[(head+*count)%cap]=*f;
 ++*count;if(*count>*peak)*peak=*count;return true;
}
bool zarc_traffic_pop(zarc_traffic *q,zarc_frame *out,bool *core){
 if(!q||!out||!core||(!q->core_count&&!q->bulk_count))return false;
 *core=q->core_count!=0;
 size_t cap=*core?ZARC_CORE_QUEUE:ZARC_BULK_QUEUE;
 size_t *count=*core?&q->core_count:&q->bulk_count;
 size_t *head=*core?&q->core_head:&q->bulk_head;
 zarc_frame *frames=*core?q->core:q->bulk;
 *out=frames[*head];*head=(*head+1u)%cap;--*count;return true;
}
