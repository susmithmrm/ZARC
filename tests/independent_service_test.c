/*
 * Integration fixture shaped like a separately maintained benign service.
 * This file consumes only the public composition contract; generic runtime
 * sources do not include or depend on it.
 */
#include "zarc/composition.h"
#include <stdio.h>

#define CHECK(x) do { if (!(x)) { fprintf(stderr, "line %d: %s\n", __LINE__, #x); return 1; } } while (0)

typedef struct {
 bool fail_init;
 unsigned init_count;
 unsigned start_count;
 unsigned quiesce_count;
 unsigned suspend_count;
 unsigned resume_count;
 unsigned release_count;
 unsigned simulated_feedback;
} service_state;

static bool service_init(void *context) {
 service_state *state=context;
 ++state->init_count;
 return !state->fail_init;
}
static bool service_start(void *context) {
 service_state *state=context;
 ++state->start_count;
 return true;
}
static bool service_quiesce(void *context,uint64_t deadline) {
 service_state *state=context;
 if(deadline==0u)return false;
 ++state->quiesce_count;
 return true;
}
static bool service_suspend(void *context) {
 ++((service_state *)context)->suspend_count;
 return true;
}
static bool service_resume(void *context) {
 ++((service_state *)context)->resume_count;
 return true;
}
static void service_release(void *context) {
 ++((service_state *)context)->release_count;
}

static zarc_service_descriptor independent_service(service_state *state) {
 return (zarc_service_descriptor){
  .id=42u,.required=true,.dependency_count=1u,.dependencies={1u},.context=state,
  .init=service_init,.start=service_start,.quiesce=service_quiesce,
  .suspend=service_suspend,.resume=service_resume,.release=service_release
 };
}

static bool legacy_simulated_sample(const zarc_composition *composition,
                                    const service_state *legacy,
                                    unsigned *feedback) {
 if(!composition||!legacy||!feedback||!zarc_composition_admits(composition,1u))return false;
 *feedback=legacy->simulated_feedback;
 return true;
}

static int optional_service_coexistence(void) {
 service_state legacy_state={.simulated_feedback=1u},optional_state={0};
 zarc_service_descriptor legacy={
  .id=1u,.required=true,.context=&legacy_state,.init=service_init,.start=service_start,
  .quiesce=service_quiesce,.suspend=service_suspend,.resume=service_resume,
  .release=service_release
 };
 zarc_service_descriptor optional=independent_service(&optional_state);
 optional.required=false;
 zarc_composition composition={0};
 unsigned feedback=0u;

 /* An absent optional integration must not disable the established path. */
 CHECK(zarc_composition_configure(&composition,&legacy,1u)==ZARC_COMPOSITION_OK);
 CHECK(zarc_composition_start(&composition)==ZARC_COMPOSITION_OK);
 CHECK(zarc_composition_required_ready(&composition));
 CHECK(zarc_composition_admits(&composition,1u));
 CHECK(!zarc_composition_admits(&composition,42u));
 CHECK(legacy_simulated_sample(&composition,&legacy_state,&feedback)&&feedback==1u);
 CHECK(zarc_composition_quiesce(&composition,1u,2u)==ZARC_COMPOSITION_OK);
 CHECK(zarc_composition_stop(&composition)==ZARC_COMPOSITION_OK);

 /* A failed optional integration is degraded locally, but core stays usable. */
 legacy_state=(service_state){.simulated_feedback=0u};
 optional_state=(service_state){.fail_init=true};
 optional=independent_service(&optional_state);
 optional.required=false;
 zarc_service_descriptor with_optional[]={optional,legacy};
 composition=(zarc_composition){0};
 CHECK(zarc_composition_configure(&composition,with_optional,2u)==ZARC_COMPOSITION_OK);
 CHECK(zarc_composition_start(&composition)==ZARC_COMPOSITION_DEGRADED);
 CHECK(zarc_composition_required_ready(&composition));
 CHECK(zarc_composition_admits(&composition,1u));
 CHECK(!zarc_composition_admits(&composition,42u));
 zarc_service_state state;
 CHECK(zarc_composition_state(&composition,42u,&state)&&state==ZARC_SERVICE_FAILED);
 CHECK(legacy_simulated_sample(&composition,&legacy_state,&feedback)&&feedback==0u);
 CHECK(zarc_composition_quiesce(&composition,1u,2u)==ZARC_COMPOSITION_OK);
 CHECK(zarc_composition_stop(&composition)==ZARC_COMPOSITION_OK);
 return 0;
}

int main(void) {
 CHECK(optional_service_coexistence()==0);
 service_state core_state={0},extension_state={0};
 zarc_service_descriptor core={
  .id=1u,.required=true,.context=&core_state,.init=service_init,.start=service_start,
  .quiesce=service_quiesce,.suspend=service_suspend,.resume=service_resume,
  .release=service_release
 };
 zarc_service_descriptor extension=independent_service(&extension_state);
 zarc_service_descriptor services[]={extension,core};
 zarc_composition composition={0};

 CHECK(zarc_composition_configure(&composition,services,2u)==ZARC_COMPOSITION_OK);
 CHECK(zarc_composition_start(&composition)==ZARC_COMPOSITION_OK);
 CHECK(zarc_composition_required_ready(&composition));
 CHECK(zarc_composition_admits(&composition,42u));
 CHECK(extension_state.init_count==1u&&extension_state.start_count==1u);
 CHECK(zarc_composition_quiesce(&composition,1u,2u)==ZARC_COMPOSITION_OK);
 CHECK(!zarc_composition_admits(&composition,42u));
 CHECK(zarc_composition_suspend(&composition)==ZARC_COMPOSITION_OK);
 CHECK(zarc_composition_resume(&composition)==ZARC_COMPOSITION_OK);
 CHECK(extension_state.suspend_count==1u&&extension_state.resume_count==1u);
 CHECK(zarc_composition_quiesce(&composition,3u,4u)==ZARC_COMPOSITION_OK);
 CHECK(zarc_composition_stop(&composition)==ZARC_COMPOSITION_OK);
 CHECK(extension_state.quiesce_count==2u&&extension_state.release_count==1u);

 extension=independent_service(&extension_state);
 CHECK(zarc_composition_configure(&composition,&extension,1u)==
       ZARC_COMPOSITION_MISSING_DEPENDENCY);
 extension.dependency_count=0u;
 extension.start=NULL;
 CHECK(zarc_composition_configure(&composition,&extension,1u)==ZARC_COMPOSITION_INVALID);
 puts("Independent service extension contract passed");
 return 0;
}
