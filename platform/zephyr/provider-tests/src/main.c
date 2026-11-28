#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/ztest.h>
#include <zephyr/drivers/uart.h>
#include <zephyr/drivers/can.h>
#include <zephyr/drivers/flash.h>
#include <zephyr/drivers/watchdog.h>
#include <zephyr/storage/flash_map.h>
#include <zephyr/ipc/ipc_service_backend.h>
#include <errno.h>
#include "transport.h"
#include "storage.h"
#include "watchdog.h"
#include "observation_bus.h"
#include "device_pm.h"
#include "sensor_sample.h"
#include <zephyr/pm/device.h>
#include <zephyr/pm/device_runtime.h>
#include <zephyr/net/socket.h>

K_MSGQ_DEFINE(uart_rx,sizeof(struct zarc_native_bytes),8,8);
K_MSGQ_DEFINE(can_rx_queue,sizeof(struct can_frame),4,4);
K_MSGQ_DEFINE(ipc_rx,sizeof(struct zarc_native_bytes),2,8);
static struct zarc_native_uart uart_adapter;
static struct zarc_native_can can_adapter;
static struct zarc_native_ipc ipc_adapter;
static struct zarc_native_watchdog watchdog_adapter;
static struct zarc_native_tcp tcp_adapter;

static const zarc_frame sample={
 .operation=ZARC_OP_SNAPSHOT,.length=4,.boot=7,.session=3,.request_id=11,.payload={1,0,0,0}
};

static int loopback_listener(struct sockaddr_in *address)
{
 int fd=zsock_socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
 if(fd<0)return -errno;
 struct sockaddr_in local={.sin_family=AF_INET,.sin_port=0};
 if(zsock_inet_pton(AF_INET,"127.0.0.1",&local.sin_addr)!=1||
    zsock_bind(fd,(const struct sockaddr *)&local,sizeof(local))<0||
    zsock_listen(fd,1)<0){int error=errno;(void)zsock_close(fd);return -error;}
 socklen_t length=sizeof(*address);
 if(zsock_getsockname(fd,(struct sockaddr *)address,&length)<0){int error=errno;(void)zsock_close(fd);return -error;}
 return fd;
}

static int socket_send_all(int fd,const uint8_t *data,size_t length)
{
 size_t sent=0;int64_t deadline=k_uptime_get()+1000;
 while(sent<length){
  int64_t remaining=deadline-k_uptime_get();if(remaining<=0)return -ETIMEDOUT;
  struct zsock_pollfd wait={.fd=fd,.events=ZSOCK_POLLOUT};
  int ready=zsock_poll(&wait,1,(int)remaining);
  if(ready<0){if(errno==EINTR)continue;return -errno;}
  if(ready==0)return -ETIMEDOUT;
  ssize_t count=zsock_send(fd,data+sent,length-sent,ZSOCK_MSG_DONTWAIT);
  if(count>0){sent+=(size_t)count;continue;}
  if(count<0&&(errno==EINTR||errno==EAGAIN||errno==EWOULDBLOCK))continue;
  return count==0?-EPIPE:-errno;
 }
 return 0;
}

static int socket_receive_exact(int fd,uint8_t *data,size_t length)
{
 size_t received=0;int64_t deadline=k_uptime_get()+1000;
 while(received<length){
  int64_t remaining=deadline-k_uptime_get();if(remaining<=0)return -ETIMEDOUT;
  struct zsock_pollfd wait={.fd=fd,.events=ZSOCK_POLLIN};
  int ready=zsock_poll(&wait,1,(int)remaining);
  if(ready<0){if(errno==EINTR)continue;return -errno;}
  if(ready==0)return -ETIMEDOUT;
  ssize_t count=zsock_recv(fd,data+received,length-received,ZSOCK_MSG_DONTWAIT);
  if(count>0){received+=(size_t)count;continue;}
  if(count<0&&(errno==EINTR||errno==EAGAIN||errno==EWOULDBLOCK))continue;
  return count==0?-ECONNRESET:-errno;
 }
 return 0;
}

ZTEST(zarc_providers,test_tcp_frame_loopback_and_recovery)
{
 struct sockaddr_in local={0};int listener=loopback_listener(&local);
 zassert_true(listener>=0,"loopback listener: %d",listener);
 zarc_native_tcp_init(&tcp_adapter);
 zassert_equal(zarc_native_tcp_connect(&tcp_adapter,&local,ZARC_NATIVE_TCP_TIMEOUT_MAX_MS),0);
 zassert_true(tcp_adapter.fd>=0);
 zassert_equal(zarc_native_tcp_connect(&tcp_adapter,&local,10),-EALREADY);
 int peer=zsock_accept(listener,NULL,NULL);
 zassert_true(peer>=0,"accept: %d",errno);

 zassert_equal(zarc_native_tcp_send(&tcp_adapter,&sample,ZARC_NATIVE_TCP_TIMEOUT_MAX_MS),0);
 uint8_t frame_bytes[ZARC_NATIVE_FRAME_MAX];size_t frame_length=ZARC_FRAME_HEADER+sample.length;
 zassert_equal(socket_receive_exact(peer,frame_bytes,frame_length),0);
 zarc_frame decoded={0};
 zassert_true(zarc_frame_decode(frame_bytes,frame_length,&decoded));
 zassert_equal(decoded.request_id,sample.request_id);
 zassert_mem_equal(decoded.payload,sample.payload,sample.length);

 zarc_frame invalid=sample;invalid.operation=0;
 zassert_equal(zarc_native_tcp_send(&tcp_adapter,&invalid,10),-EINVAL);
 zassert_equal(zarc_native_tcp_send(&tcp_adapter,&sample,0),-EINVAL);
 zarc_frame maximum=sample;maximum.length=ZARC_PAYLOAD_MAX;
 for(size_t i=0;i<maximum.length;++i)maximum.payload[i]=(uint8_t)i;
 zassert_equal(zarc_native_tcp_send(&tcp_adapter,&maximum,100),0);
 frame_length=ZARC_FRAME_HEADER+ZARC_PAYLOAD_MAX;
 zassert_equal(socket_receive_exact(peer,frame_bytes,frame_length),0);
 zassert_true(zarc_frame_decode(frame_bytes,frame_length,&decoded));
 zassert_equal(decoded.length,ZARC_PAYLOAD_MAX);
 zassert_mem_equal(decoded.payload,maximum.payload,ZARC_PAYLOAD_MAX);

 zarc_frame response=sample;response.operation=ZARC_OP_EVENT;response.request_id=29;
 response.payload[0]=7;
 zassert_true(zarc_frame_encode(&response,frame_bytes,sizeof(frame_bytes),&frame_length));
 zassert_equal(socket_send_all(peer,frame_bytes,7),0);
 struct zarc_native_bytes bytes={0};
 zassert_equal(zarc_native_tcp_receive(&tcp_adapter,&bytes,100),0);
 zassert_equal(bytes.len,7);
 zarc_stream stream={0};size_t consumed=0;
 zassert_equal(zarc_stream_feed(&stream,bytes.data,bytes.len,&consumed,&decoded),ZARC_STREAM_MORE);
 zassert_equal(consumed,bytes.len);
 zassert_equal(stream.used,7);
 zassert_equal(zarc_native_tcp_receive(&tcp_adapter,&bytes,20),-ETIMEDOUT);
 zassert_equal(stream.used,7,"partial frame state remains with the caller across idle timeout");
 zassert_true(tcp_adapter.fd>=0,"partial-frame timeout must not close the socket");
 zassert_equal(zarc_native_tcp_receive(&tcp_adapter,&bytes,20),-ETIMEDOUT);
 zassert_true(tcp_adapter.fd>=0,"repeated idle timeout must preserve the socket");
 zassert_equal(socket_send_all(peer,frame_bytes+7,frame_length-7),0);
 bool complete=false;int64_t deadline=k_uptime_get()+500;
 while(!complete&&k_uptime_get()<deadline){
  zassert_equal(zarc_native_tcp_receive(&tcp_adapter,&bytes,100),0);
  consumed=0;zarc_stream_status status=zarc_stream_feed(&stream,bytes.data,bytes.len,&consumed,&decoded);
  zassert_equal(consumed,bytes.len);zassert_not_equal(status,ZARC_STREAM_INVALID);
  complete=status==ZARC_STREAM_FRAME;
 }
 zassert_true(complete,"fragmented TCP frame not reconstructed");
 zassert_equal(decoded.request_id,response.request_id);
 zassert_equal(decoded.payload[0],response.payload[0]);

 zassert_equal(zarc_native_tcp_receive(&tcp_adapter,&bytes,20),-ETIMEDOUT);
 zassert_true(tcp_adapter.fd>=0,"ordinary receive timeout must keep the session socket");
 zassert_equal(zsock_close(peer),0);
 zassert_equal(zarc_native_tcp_receive(&tcp_adapter,&bytes,100),-ECONNRESET);
 zassert_equal(tcp_adapter.fd,-1);
 zassert_equal(zarc_native_tcp_send(&tcp_adapter,&sample,10),-ENOTCONN);
 zassert_equal(zarc_native_tcp_close(&tcp_adapter),0);
 zassert_equal(zarc_native_tcp_close(&tcp_adapter),0);
 zassert_equal(zsock_close(listener),0);
}

ZTEST(zarc_providers,test_tcp_invalid_inputs_and_refused_peer)
{
 zarc_native_tcp tcp;zarc_native_tcp_init(&tcp);
 struct zarc_native_bytes bytes={0};
 zassert_equal(zarc_native_tcp_send(&tcp,&sample,1),-ENOTCONN);
 zassert_equal(zarc_native_tcp_receive(&tcp,&bytes,1),-ENOTCONN);
 struct sockaddr_in remote={.sin_family=AF_INET,.sin_port=0};
 zassert_equal(zsock_inet_pton(AF_INET,"127.0.0.1",&remote.sin_addr),1);
 zassert_equal(zarc_native_tcp_connect(NULL,&remote,1),-EINVAL);
 zassert_equal(zarc_native_tcp_connect(&tcp,NULL,1),-EINVAL);
 zassert_equal(zarc_native_tcp_connect(&tcp,&remote,1),-EINVAL);
 remote.sin_port=htons(1);
 zassert_equal(zarc_native_tcp_connect(&tcp,&remote,0),-EINVAL);
 zassert_equal(zarc_native_tcp_connect(&tcp,&remote,ZARC_NATIVE_TCP_TIMEOUT_MAX_MS+1u),-EINVAL);
 remote.sin_family=AF_INET6;
 zassert_equal(zarc_native_tcp_connect(&tcp,&remote,1),-EINVAL);
 remote.sin_family=AF_INET;
 struct sockaddr_in unused={0};int listener=loopback_listener(&unused);
 zassert_true(listener>=0);
 uint16_t unused_port=unused.sin_port;zassert_equal(zsock_close(listener),0);
 unused.sin_port=unused_port;
 int ret=zarc_native_tcp_connect(&tcp,&unused,ZARC_NATIVE_TCP_TIMEOUT_MAX_MS);
 zassert_true(ret<0,"a closed loopback port must not connect");
 zassert_equal(tcp.fd,-1,"failed connect must release the descriptor");
 zassert_equal(zarc_native_tcp_close(&tcp),0);
}

ZTEST(zarc_providers,test_uart_loopback){
 const struct device *dev=DEVICE_DT_GET(DT_NODELABEL(zarc_uart));
 zassert_true(device_is_ready(dev));
 zassert_equal(zarc_native_uart_open(&uart_adapter,dev,&uart_rx),0);
 zassert_equal(zarc_native_uart_send(&uart_adapter,&sample),0);
 zarc_stream stream={0};zarc_frame decoded;bool complete=false;
 int64_t deadline=k_uptime_get()+1000;
 while(!complete&&k_uptime_get()<deadline){
  struct zarc_native_bytes bytes;
  if(k_msgq_get(&uart_rx,&bytes,K_MSEC(50))!=0)continue;
  size_t used=0;
  zarc_stream_status ret=zarc_stream_feed(&stream,bytes.data,bytes.len,&used,&decoded);
  zassert_equal(used,bytes.len);
  zassert_not_equal(ret,ZARC_STREAM_INVALID);
  complete=ret==ZARC_STREAM_FRAME;
 }
 zassert_true(complete,"UART emulator did not return frame");
 zassert_equal(decoded.request_id,sample.request_id);
 zassert_mem_equal(decoded.payload,sample.payload,sample.length);
 zassert_equal(atomic_get(&uart_adapter.fault),0);
 int result=zarc_native_uart_stop(&uart_adapter);
 deadline=k_uptime_get()+500;
 while(result==-EINPROGRESS&&k_uptime_get()<deadline){k_sleep(K_MSEC(5));result=zarc_native_uart_stop(&uart_adapter);}
 zassert_equal(result,0,"shutdown result %d; rx=%ld tx=%ld owners=%ld,%ld",result,
  (long)atomic_get(&uart_adapter.rx_active),(long)atomic_get(&uart_adapter.tx_busy),
  (long)atomic_get(&uart_adapter.rx_owned[0]),(long)atomic_get(&uart_adapter.rx_owned[1]));
 zassert_equal(atomic_get(&uart_adapter.rx_owned[0]),0);
 zassert_equal(atomic_get(&uart_adapter.rx_owned[1]),0);
 zassert_equal(zarc_native_uart_send(&uart_adapter,&sample),-ENOTCONN);
}

ZTEST(zarc_providers,test_can_loopback){
 const struct device *dev=DEVICE_DT_GET(DT_CHOSEN(zephyr_canbus));
 zassert_true(device_is_ready(dev));
 zassert_equal(can_set_mode(dev,CAN_MODE_LOOPBACK),0);
 const struct can_filter filter={.id=0x123,.mask=CAN_STD_ID_MASK};
 zassert_equal(zarc_native_can_open(&can_adapter,dev,&can_rx_queue,&filter),0);
 const struct can_frame tx={.id=0x123,.dlc=2,.data={0x12,0x34}};
 struct can_frame rx;
 zassert_equal(zarc_native_can_send(&can_adapter,&tx),0);
 zassert_equal(k_msgq_get(&can_rx_queue,&rx,K_MSEC(500)),0);
 zassert_equal(rx.id,tx.id);zassert_equal(rx.dlc,tx.dlc);
 zassert_mem_equal(rx.data,tx.data,2);
 zassert_equal(atomic_get(&can_adapter.fault),0);
 zassert_equal(zarc_native_can_close(&can_adapter),0);
 zassert_equal(zarc_native_can_send(&can_adapter,&tx),-ENOTCONN);
}

ZTEST(zarc_providers,test_nvs_configuration_remount){
 const struct flash_area *area;
 zassert_equal(flash_area_open(FIXED_PARTITION_ID(storage_partition),&area),0);
 struct flash_pages_info page;
 const struct device *dev=flash_area_get_device(area);
 zassert_true(device_is_ready(dev));
 zassert_equal(flash_get_page_info_by_offs(dev,area->fa_off,&page),0);
 zassert_true(page.size<=UINT16_MAX&&area->fa_size/page.size>=2);
 struct nvs_fs fs={.offset=area->fa_off,.sector_size=(uint16_t)page.size,.sector_count=2,.flash_device=dev};
 zassert_equal(nvs_mount(&fs),0);
 /* Only the dedicated simulator partition is cleared. Never use this test on
  * an actual product partition or a board with retained customer data. */
 zassert_equal(nvs_clear(&fs),0);
 zassert_equal(nvs_mount(&fs),0);
 zarc_config_store store={0},loaded;
 zarc_config config;
 size_t index;
 const uint8_t first[]={1,2},second[]={3,4};
 zassert_equal(zarc_config_stage(&store,1,1,first,sizeof first,&index),ZARC_CONFIG_OK);
 zassert_equal(zarc_config_commit(&store,index,1,1),ZARC_CONFIG_OK);
 zassert_equal(zarc_native_config_write(&fs,123,&store),0);
 zassert_equal(zarc_config_stage(&store,1,2,second,sizeof second,&index),ZARC_CONFIG_OK);
 zassert_equal(zarc_native_config_write(&fs,123,&store),0);
 struct nvs_fs remount={.offset=fs.offset,.sector_size=fs.sector_size,.sector_count=fs.sector_count,.flash_device=dev};
 zassert_equal(nvs_mount(&remount),0);
 zassert_equal(zarc_native_config_read(&remount,123,&loaded),0);
 zassert_equal(zarc_config_load(&loaded,1,&config),ZARC_CONFIG_OK);
 zassert_equal(config.revision,1);
 zassert_equal(zarc_config_commit(&loaded,index,1,2),ZARC_CONFIG_OK);
 zassert_equal(zarc_native_config_write(&remount,123,&loaded),0);
 zassert_equal(zarc_native_config_read(&remount,123,&store),0);
 zassert_equal(zarc_config_load(&store,1,&config),ZARC_CONFIG_OK);
 zassert_equal(config.revision,2);
 struct zarc_native_update_journal journal={.fs=&remount,.key=124,.writable=true};
 zarc_update update,recovered;uint64_t revision;
 const zarc_update_policy policy={1,2,3,1};
 const zarc_image image={11,1,1,2,3};
 zassert_true(zarc_update_init(&update,&policy,&image,true));
 zassert_equal(zarc_native_update_journal_persist(&journal,&update),0);
 zassert_equal(zarc_native_config_read(&remount,124,&loaded),0);
 zassert_equal(zarc_update_journal_load(&loaded,&policy,&recovered,&revision),ZARC_CONFIG_OK);
 zassert_equal(revision,1);
 zassert_equal(recovered.active.image_id,11);
 struct zarc_native_diag_store diag_store={.fs=&remount,.key=125,.writable=true};
 zarc_diagnostics faults={0},restored={0};
 const zarc_fault fault={.version=1,.code=1,.service=1,.epoch=4,.timestamp=8,.quality=ZARC_DIAG_FAULT,.severity=ZARC_DIAG_ERROR};
 zassert_equal(zarc_diag_record(&faults,&fault),ZARC_DIAG_STORED);
 zassert_equal(zarc_native_diag_persist(&diag_store,&faults),0);
 struct nvs_fs diag_remount={.offset=fs.offset,.sector_size=fs.sector_size,.sector_count=fs.sector_count,.flash_device=dev};
 zassert_equal(nvs_mount(&diag_remount),0);
 struct zarc_native_diag_store restored_store={.fs=&diag_remount,.key=125};
 zassert_equal(zarc_native_diag_load(&restored_store,&restored),0);
 zassert_equal(restored_store.revision,1);
 zassert_equal(restored.count,1);
 zassert_equal(restored.records[0].epoch,4);
 flash_area_close(area);
}

/* Explicit simulated IPC backend. Calls through the actual Zephyr IPC service
 * front end; no claim of inter-core cache or shared-memory behavior. */
struct ipc_fixture { const struct ipc_ept_cfg *cfg;bool registered; };
static struct ipc_fixture ipc_state;
static int ipc_register(const struct device *dev,void **token,const struct ipc_ept_cfg *cfg){
 struct ipc_fixture *f=dev->data;
 if(f->registered)return -EBUSY;
 f->cfg=cfg;f->registered=true;*token=f;cfg->cb.bound(cfg->priv);return 0;
}
static int ipc_deregister(const struct device *dev,void *token){
 struct ipc_fixture *f=dev->data;
 if(f!=token||!f->registered)return -ENOENT;
 f->registered=false;f->cfg=NULL;return 0;
}
static int ipc_send_fixture(const struct device *dev,void *token,const void *data,size_t len){
 struct ipc_fixture *f=dev->data;
 if(f!=token||!f->registered)return -ENOENT;
 f->cfg->cb.received(data,len,f->cfg->priv);return (int)len;
}
static const struct ipc_service_backend ipc_api={
 .register_endpoint=ipc_register,.deregister_endpoint=ipc_deregister,.send=ipc_send_fixture
};
DEVICE_DEFINE(zarc_test_ipc,"zarc_test_ipc",NULL,NULL,&ipc_state,NULL,POST_KERNEL,
              CONFIG_KERNEL_INIT_PRIORITY_DEVICE,&ipc_api);

ZTEST(zarc_providers,test_ipc_service_and_loss){
 const struct device *dev=DEVICE_GET(zarc_test_ipc);
 zassert_equal(zarc_native_ipc_open(&ipc_adapter,dev,"zarc-test",&ipc_rx),0);
 zassert_equal(zarc_native_ipc_send(&ipc_adapter,&sample),0);
 struct zarc_native_bytes bytes;zarc_frame decoded;
 zassert_equal(k_msgq_get(&ipc_rx,&bytes,K_NO_WAIT),0);
 zassert_true(zarc_frame_decode(bytes.data,bytes.len,&decoded));
 zassert_equal(decoded.request_id,sample.request_id);

 uint8_t encoded[ZARC_NATIVE_FRAME_MAX];size_t encoded_len=0;
 zassert_true(zarc_frame_encode(&sample,encoded,sizeof encoded,&encoded_len));
 /* Exact bounded depth preserves both valid items; the next item latches a
  * transport fault and is not silently accepted as usable channel state. */
 ipc_state.cfg->cb.received(encoded,encoded_len,ipc_state.cfg->priv);
 ipc_state.cfg->cb.received(encoded,encoded_len,ipc_state.cfg->priv);
 zassert_equal(k_msgq_num_used_get(&ipc_rx),2);
 ipc_state.cfg->cb.received(encoded,encoded_len,ipc_state.cfg->priv);
 zassert_true(atomic_get(&ipc_adapter.fault));
 zassert_equal(zarc_native_ipc_send(&ipc_adapter,&sample),-ENOTCONN);
 zassert_equal(k_msgq_get(&ipc_rx,&bytes,K_NO_WAIT),0);
 zassert_equal(k_msgq_get(&ipc_rx,&bytes,K_NO_WAIT),0);
 zassert_equal(k_msgq_get(&ipc_rx,&bytes,K_NO_WAIT),-ENOMSG);
 zassert_equal(zarc_native_ipc_close(&ipc_adapter),0);

 /* Malformed callback inputs fail closed before copying or queueing. */
 zassert_equal(zarc_native_ipc_open(&ipc_adapter,dev,"zarc-test-short",&ipc_rx),0);
 ipc_state.cfg->cb.received(encoded,ZARC_FRAME_HEADER-1u,ipc_state.cfg->priv);
 zassert_true(atomic_get(&ipc_adapter.fault));
 zassert_equal(k_msgq_num_used_get(&ipc_rx),0);
 zassert_equal(zarc_native_ipc_close(&ipc_adapter),0);
 zassert_equal(zarc_native_ipc_open(&ipc_adapter,dev,"zarc-test-long",&ipc_rx),0);
 uint8_t oversized[ZARC_NATIVE_FRAME_MAX+1u]={0};
 ipc_state.cfg->cb.received(oversized,sizeof oversized,ipc_state.cfg->priv);
 zassert_true(atomic_get(&ipc_adapter.fault));
 zassert_equal(k_msgq_num_used_get(&ipc_rx),0);
 zassert_equal(zarc_native_ipc_close(&ipc_adapter),0);
 zassert_equal(zarc_native_ipc_open(&ipc_adapter,dev,"zarc-test-null",&ipc_rx),0);
 ipc_state.cfg->cb.received(NULL,ZARC_FRAME_HEADER,ipc_state.cfg->priv);
 zassert_true(atomic_get(&ipc_adapter.fault));
 zassert_equal(zarc_native_ipc_close(&ipc_adapter),0);

 /* A rebound endpoint stays faulted until a full close/open lifecycle. */
 zassert_equal(zarc_native_ipc_open(&ipc_adapter,dev,"zarc-test-rebind",&ipc_rx),0);
 ipc_state.cfg->cb.unbound(ipc_state.cfg->priv);
 zassert_equal(zarc_native_ipc_send(&ipc_adapter,&sample),-ENOTCONN);
 ipc_state.cfg->cb.bound(ipc_state.cfg->priv);
 zassert_equal(zarc_native_ipc_send(&ipc_adapter,&sample),-ENOTCONN);
 zassert_equal(zarc_native_ipc_close(&ipc_adapter),0);
 zassert_equal(zarc_native_ipc_open(&ipc_adapter,dev,"zarc-test-reopened",&ipc_rx),0);
 zassert_equal(zarc_native_ipc_send(&ipc_adapter,&sample),0);
 zassert_equal(k_msgq_get(&ipc_rx,&bytes,K_NO_WAIT),0);
 zassert_equal(zarc_native_ipc_close(&ipc_adapter),0);
}

/* Watchdog spy device for policy verification. It does not reset a host. */
struct watchdog_fixture {unsigned feeds;bool installed,armed;};
static struct watchdog_fixture wdt_state;
static int test_wdt_install(const struct device *dev,const struct wdt_timeout_cfg *cfg){
 struct watchdog_fixture *f=dev->data;
 if(!cfg||cfg->window.max!=100||cfg->flags!=WDT_FLAG_RESET_SOC)return -EINVAL;
 f->installed=true;return 0;
}
static int test_wdt_setup(const struct device *dev,uint8_t options){
 struct watchdog_fixture *f=dev->data;
 if(options||!f->installed)return -EINVAL;
 f->armed=true;return 0;
}
static int test_wdt_feed(const struct device *dev,int channel){
 struct watchdog_fixture *f=dev->data;
 if(!f->armed||channel)return -EINVAL;
 ++f->feeds;return 0;
}
static int test_wdt_disable(const struct device *dev){struct watchdog_fixture *f=dev->data;f->armed=false;return 0;}
static DEVICE_API(wdt, test_wdt_api)={
 .install_timeout=test_wdt_install,.setup=test_wdt_setup,.feed=test_wdt_feed,.disable=test_wdt_disable
};
DEVICE_DEFINE(zarc_test_watchdog,"zarc_test_watchdog",NULL,NULL,&wdt_state,NULL,POST_KERNEL,
              CONFIG_KERNEL_INIT_PRIORITY_DEVICE,&test_wdt_api);

ZTEST(zarc_providers,test_watchdog_gated_feed){
 const struct device *dev=DEVICE_GET(zarc_test_watchdog);
 zarc_health health;
 const zarc_health_policy policy={1,10,10,true};
 zassert_true(zarc_health_init(&health,&policy,1,4,0));
 zassert_equal(zarc_native_watchdog_arm(&watchdog_adapter,dev,100),0);
 zassert_equal(zarc_native_watchdog_supervise(&watchdog_adapter,&health,0),-EAGAIN);
 zassert_equal(wdt_state.feeds,0);
 zassert_true(zarc_health_report(&health,1,4,1,1,true,true));
 zassert_equal(zarc_native_watchdog_supervise(&watchdog_adapter,&health,1),0);
 zassert_equal(wdt_state.feeds,1);
 zassert_equal(zarc_native_watchdog_supervise(&watchdog_adapter,&health,11),-EAGAIN);
 zassert_equal(wdt_state.feeds,1);
}
ZTEST(zarc_providers,test_zbus_observation_overflow){
 zarc_native_observation_reset();
 zarc_observation_event event={.observation={.id=1,.boot=7,.value=ZARC_OFF,.quality=ZARC_QUALITY_VALID}};
 zarc_observation_event out;
 for(uint64_t i=1;i<=4;++i){event.sequence=i;zassert_equal(zarc_native_observation_publish(&event),0);}
 for(uint64_t i=1;i<=4;++i){zassert_equal(zarc_native_observation_read(&out),0);zassert_equal(out.sequence,i);}
 for(uint64_t i=5;i<=8;++i){event.sequence=i;zassert_equal(zarc_native_observation_publish(&event),0);}
 event.sequence=9;zassert_equal(zarc_native_observation_publish(&event),-EOVERFLOW);
 zassert_equal(zarc_native_observation_read(&out),-EOVERFLOW);
 zarc_native_observation_reset();
 event.sequence=10;zassert_equal(zarc_native_observation_publish(&event),0);
 zassert_equal(zarc_native_observation_read(&out),0);zassert_equal(out.sequence,10);
}
struct pm_fixture {unsigned resumes,suspends;bool fail_resume;};
static struct pm_fixture pm_state;
static int test_pm_action(const struct device *dev,enum pm_device_action action){
 struct pm_fixture *f=dev->data;
 if(action==PM_DEVICE_ACTION_RESUME){
  if(f->fail_resume)return -EIO;
  ++f->resumes;return 0;
 }
 if(action==PM_DEVICE_ACTION_SUSPEND){++f->suspends;return 0;}
 return -ENOTSUP;
}
PM_DEVICE_DEFINE(zarc_test_pm,test_pm_action);
DEVICE_DEFINE(zarc_test_pm,"zarc_test_pm",NULL,PM_DEVICE_GET(zarc_test_pm),&pm_state,NULL,POST_KERNEL,
              CONFIG_KERNEL_INIT_PRIORITY_DEVICE,NULL);
ZTEST(zarc_providers,test_runtime_pm_balance){
 const struct device *dev=DEVICE_GET(zarc_test_pm);
 struct zarc_native_pm pm={0};
 zassert_equal(zarc_native_pm_open(&pm,dev),0);
 zassert_equal(pm_device_runtime_usage(dev),0);
 zassert_equal(zarc_native_pm_acquire(&pm),0);
 zassert_equal(zarc_native_pm_acquire(&pm),-EALREADY);
 zassert_equal(pm_device_runtime_usage(dev),1);
 zassert_equal(pm_state.resumes,1);
 zassert_equal(zarc_native_pm_release(&pm),0);
 zassert_equal(pm_device_runtime_usage(dev),0);
 zassert_equal(zarc_native_pm_release(&pm),-EINVAL);
 pm_state.fail_resume=true;
 zassert_equal(zarc_native_pm_acquire(&pm),-EIO);
 zassert_true(pm.fault);zassert_false(pm.held);
 zassert_equal(pm_device_runtime_usage(dev),0);
}
ZTEST_SUITE(zarc_providers,NULL,NULL,NULL,NULL,NULL);

ZTEST(zarc_providers,test_sensor_conversion_and_unavailable){
 int32_t out=0;
 struct sensor_value v={.val1=12,.val2=345678};
 zassert_true(zarc_native_sensor_convert(&v,-3,&out));
 zassert_equal(out,12345);
 v.val1=-12;v.val2=-345678;
 zassert_true(zarc_native_sensor_convert(&v,-3,&out));
 zassert_equal(out,-12345);
 v.val1=2147483647;v.val2=0;
 zassert_false(zarc_native_sensor_convert(&v,-9,&out));
 v.val1=1;v.val2=1000000;
 zassert_false(zarc_native_sensor_convert(&v,-3,&out));
 zarc_sensor s={0};zarc_sensor_sample sample_out;
 const zarc_sensor_config cfg={1,2,3,-3,-40000,125000,10};
 zassert_true(zarc_sensor_init(&s,&cfg,1,0));
 zassert_equal(zarc_native_sensor_poll(&s,NULL,SENSOR_CHAN_AMBIENT_TEMP,1,1,1),-ENODEV);
 zassert_true(zarc_sensor_snapshot(&s,1,&sample_out));
 zassert_equal(sample_out.quality,ZARC_SAMPLE_FAULT);
 zassert_equal(sample_out.sequence,1);
 zassert_equal(zarc_native_sensor_poll(&s,NULL,SENSOR_CHAN_AMBIENT_TEMP,1,1,2),-EINVAL);
}
