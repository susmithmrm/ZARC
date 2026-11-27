#include "transport.h"
#include <zephyr/drivers/uart.h>
#include <errno.h>
#include <string.h>

static void receive(const struct device *dev,struct uart_event *event,void *user){
 struct zarc_native_uart *u=user;
 switch(event->type){
 case UART_TX_DONE:
  atomic_clear(&u->tx_busy);break;
 case UART_TX_ABORTED:
  atomic_set(&u->fault,1);atomic_clear(&u->tx_busy);break;
 case UART_RX_RDY:{
  const size_t offset=event->data.rx.offset,n=event->data.rx.len;
  const uint8_t *p=event->data.rx.buf;
  if((p!=u->rx_buf[0]&&p!=u->rx_buf[1])||offset>sizeof u->rx_buf[0]||n>sizeof u->rx_buf[0]-offset){
   atomic_set(&u->fault,1);break;
  }
  struct zarc_native_bytes chunk={.len=n};
  memcpy(chunk.data,p+offset,n);
  if(n&&k_msgq_put(u->rx,&chunk,K_NO_WAIT)!=0)atomic_set(&u->fault,1);
  break;}
 case UART_RX_BUF_REQUEST:
  for(size_t i=0;i<2;++i){
   if(atomic_cas(&u->rx_owned[i],0,1)){
    if(uart_rx_buf_rsp(dev,u->rx_buf[i],sizeof u->rx_buf[i])<0){
     atomic_clear(&u->rx_owned[i]);atomic_set(&u->fault,1);
    }
    return;
   }
  }
  atomic_set(&u->fault,1);break;
 case UART_RX_BUF_RELEASED:
  for(size_t i=0;i<2;++i)if(event->data.rx_buf.buf==u->rx_buf[i])atomic_clear(&u->rx_owned[i]);
  break;
 case UART_RX_DISABLED:
  atomic_clear(&u->rx_active);atomic_set(&u->fault,1);break;
 case UART_RX_STOPPED:
  atomic_set(&u->fault,1);break;
 default:break;
 }
}
int zarc_native_uart_open(struct zarc_native_uart *u,const struct device *dev,struct k_msgq *rx){
 if(!u||!dev||!rx||!device_is_ready(dev)||rx->msg_size!=sizeof(struct zarc_native_bytes))return -EINVAL;
 /* Fresh statically allocated instance only; never overwrite active state. */
 if(u->dev)return -EALREADY;
 u->dev=dev;u->rx=rx;
 int r=uart_callback_set(dev,receive,u);
 if(r<0){u->dev=NULL;return r;}
 atomic_set(&u->rx_owned[0],1);atomic_set(&u->rx_active,1);
 r=uart_rx_enable(dev,u->rx_buf[0],sizeof u->rx_buf[0],10000);
 if(r<0){atomic_clear(&u->rx_owned[0]);atomic_clear(&u->rx_active);atomic_set(&u->fault,1);}
 return r;
}
int zarc_native_uart_send(struct zarc_native_uart *u,const zarc_frame *f){
 if(!u||!u->dev||atomic_get(&u->fault)||!atomic_get(&u->rx_active))return -ENOTCONN;
 if(!atomic_cas(&u->tx_busy,0,1))return -EBUSY;
 size_t n;
 if(!zarc_frame_encode(f,u->tx_buf,sizeof u->tx_buf,&n)){atomic_clear(&u->tx_busy);return -EINVAL;}
 int ret=uart_tx(u->dev,u->tx_buf,n,100000);
 if(ret<0)atomic_clear(&u->tx_busy);
 return ret;
}
int zarc_native_uart_stop(struct zarc_native_uart *u){
 if(!u||!u->dev)return -EINVAL;
 atomic_set(&u->fault,1);
 if(atomic_get(&u->tx_busy))(void)uart_tx_abort(u->dev);
 if(atomic_get(&u->rx_active)){int ret=uart_rx_disable(u->dev);if(ret<0)return ret;}
 /* State remains allocated until driver-specific callback quiescence. */
 return atomic_get(&u->rx_active)||atomic_get(&u->tx_busy)||atomic_get(&u->rx_owned[0])||atomic_get(&u->rx_owned[1])?-EINPROGRESS:0;
}
