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

K_MSGQ_DEFINE(uart_rx,sizeof(struct zarc_native_bytes),8,8);
K_MSGQ_DEFINE(can_rx_queue,sizeof(struct can_frame),4,4);
K_MSGQ_DEFINE(ipc_rx,sizeof(struct zarc_native_bytes),2,8);
static struct zarc_native_uart uart_adapter;
static struct zarc_native_can can_adapter;
static struct zarc_native_ipc ipc_adapter;
static struct zarc_native_watchdog watchdog_adapter;

static const zarc_frame sample={
 .operation=ZARC_OP_SNAPSHOT,.length=4,.boot=7,.session=3,.request_id=11,.payload={1,0,0,0}
};

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
 zassert_equal(result,0);
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
 ipc_state.cfg->cb.unbound(ipc_state.cfg->priv);
 zassert_equal(zarc_native_ipc_send(&ipc_adapter,&sample),-ENOTCONN);
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
ZTEST_SUITE(zarc_providers,NULL,NULL,NULL,NULL,NULL);
