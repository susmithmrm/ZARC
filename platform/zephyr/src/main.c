#include <zephyr/kernel.h>
#include <zephyr/sys/atomic.h>
#include <zephyr/ztest.h>
#include "zarc/controller.h"

/* Reference native kernel binding, simulated indicator only.
 * Controller state belongs exclusively to the ztest owner thread.
 * Device worker never dispatches or accesses controller state. */
struct observation {
 uint32_t id;
 uint64_t boot, sequence, sample_tick;
 zarc_value value;
};
K_MSGQ_DEFINE(effect_queue, sizeof(zarc_controller_effect), 2, 8);
K_MSGQ_DEFINE(observation_queue, sizeof(struct observation), 4, 8);
K_THREAD_STACK_DEFINE(device_stack, 2048);
static struct k_thread device_thread;
static atomic_t device_error;
static zarc_controller controller;

static uint64_t ticks(void)
{
 return (uint64_t)k_uptime_get();
}
static void virtual_device(void *a, void *b, void *c)
{
 ARG_UNUSED(a);
 ARG_UNUSED(b);
 ARG_UNUSED(c);
 zarc_controller_effect effect;
 if(k_msgq_get(&effect_queue,&effect,K_MSEC(500))!=0 || !effect.emitted){
  atomic_set(&device_error,1);
  return;
 }
 k_sleep(K_MSEC(30));
 struct observation sample={
  .id=effect.id,.boot=101,.sequence=2,.sample_tick=ticks(),.value=effect.desired
 };
 if(k_msgq_put(&observation_queue,&sample,K_NO_WAIT)!=0)atomic_set(&device_error,1);
}
ZTEST(zarc_native_lab, test_disconnected_feedback)
{
 k_msgq_purge(&effect_queue);
 k_msgq_purge(&observation_queue);
 atomic_clear(&device_error);
 uint64_t now=ticks();
 const zarc_endpoint_config config={1,500};
 zassert_true(zarc_controller_init(&controller,&config,1,101,1000,now));
 zassert_true(zarc_controller_ready(&controller,1,true));
 zassert_true(zarc_controller_feedback(&controller,1,101,1,now,now,ZARC_OFF));
 zassert_equal(zarc_controller_open(&controller,1,1,1,0,now),ZARC_CONN_COMPLETED);
 k_tid_t worker=k_thread_create(&device_thread,device_stack,
  K_THREAD_STACK_SIZEOF(device_stack),virtual_device,NULL,NULL,NULL,
  K_PRIO_PREEMPT(5),0,K_NO_WAIT);
 zarc_intent intent={.session=1,.operation=ZARC_OP_DOMAIN,.request_id=1,
  .domain=1,.desired=ZARC_ON,.validity=500};
 zarc_controller_effect effect;
 now=ticks();
 /* Only this thread publishes effects; the consumer cannot take capacity
  * away. Check the free slot before committing admission. */
 bool available=k_msgq_num_free_get(&effect_queue)>0;
 zassert_equal(zarc_controller_request(&controller,&intent,true,true,0,
  now,available,&effect),ZARC_CONN_ACCEPTED);
 zassert_true(effect.emitted);
 zassert_equal(k_msgq_put(&effect_queue,&effect,K_NO_WAIT),0);
 zarc_controller_disconnect(&controller);
 struct observation sample;
 int result=k_msgq_get(&observation_queue,&sample,K_MSEC(500));
 int joined=k_thread_join(worker,K_MSEC(500));
 if(joined!=0)k_thread_abort(worker);
 zassert_equal(joined,0);
 zassert_equal(atomic_get(&device_error),0);
 zassert_equal(result,0);
 now=ticks();
 zassert_true(zarc_controller_feedback(&controller,sample.id,sample.boot,
  sample.sequence,sample.sample_tick,now,sample.value));
 zassert_equal(zarc_controller_open(&controller,2,1,1,0,now),ZARC_CONN_COMPLETED);
 zarc_observation observation;
 zassert_true(zarc_controller_snapshot(&controller,1,&observation));
 zassert_equal(observation.value,ZARC_ON);
 zassert_equal(observation.quality,ZARC_QUALITY_VALID);
}
ZTEST_SUITE(zarc_native_lab,NULL,NULL,NULL,NULL,NULL);
