#include "zarc/composition.h"
#include <stdio.h>
#define CHECK(x) do {if(!(x)){fprintf(stderr,"line %d: %s\n",__LINE__,#x);return 1;}}while(0)
typedef struct {bool fail_init,fail_start,fail_quiesce;unsigned inits,releases,resumes;uint32_t id;} fixture;
static unsigned order[32],count;
static bool init(void *p){fixture *f=p;++f->inits;return !f->fail_init;}
static bool start(void *p){return !((fixture *)p)->fail_start;}
static bool quiesce(void *p,uint64_t deadline){(void)deadline;return !((fixture *)p)->fail_quiesce;}
static bool suspend_service(void *p){(void)p;return true;}
static bool resume(void *p){++((fixture *)p)->resumes;return true;}
static void release(void *p){fixture *f=p;++f->releases;order[count++]=f->id;}
static zarc_service_descriptor desc(fixture *f){return (zarc_service_descriptor){.id=f->id,.required=true,.context=f,.init=init,.start=start,.quiesce=quiesce,.suspend=suspend_service,.resume=resume,.release=release};}
int main(void){
 fixture a={.id=1},b={.id=2},d={.id=3};
 zarc_composition c={0};zarc_service_descriptor s[]={desc(&b),desc(&a),desc(&d)};
 s[0].dependency_count=1;s[0].dependencies[0]=1;
 CHECK(zarc_composition_configure(&c,s,3)==ZARC_COMPOSITION_OK);
 CHECK(zarc_composition_start(&c)==ZARC_COMPOSITION_OK);
 CHECK(zarc_composition_admits(&c,2)&&zarc_composition_required_ready(&c));
 CHECK(zarc_composition_stop(&c)==ZARC_COMPOSITION_INVALID);
 CHECK(zarc_composition_quiesce(&c,1,2)==ZARC_COMPOSITION_OK);
 CHECK(!zarc_composition_admits(&c,2));
 CHECK(zarc_composition_suspend(&c)==ZARC_COMPOSITION_OK);
 CHECK(zarc_composition_resume(&c)==ZARC_COMPOSITION_OK&&a.resumes==1&&b.resumes==1);
 CHECK(zarc_composition_quiesce(&c,3,4)==ZARC_COMPOSITION_OK);
 CHECK(zarc_composition_stop(&c)==ZARC_COMPOSITION_OK);
 CHECK(count==3&&order[0]==3&&order[1]==2&&order[2]==1);
 CHECK(zarc_composition_resume(&c)==ZARC_COMPOSITION_INVALID);
 a.fail_start=true;
 CHECK(zarc_composition_start(&c)==ZARC_COMPOSITION_UNAVAILABLE);
 CHECK(!zarc_composition_required_ready(&c));
 CHECK(a.releases==2&&b.inits==1&&zarc_composition_admits(&c,3));
 CHECK(!zarc_composition_admits(&c,1)&&!zarc_composition_admits(&c,2));
 CHECK(zarc_composition_quiesce(&c,5,6)==ZARC_COMPOSITION_OK);
 CHECK(zarc_composition_stop(&c)==ZARC_COMPOSITION_OK);
 s[0].dependencies[0]=99;
 CHECK(zarc_composition_configure(&c,s,3)==ZARC_COMPOSITION_MISSING_DEPENDENCY);
 s[0].dependencies[0]=1;s[1].dependency_count=1;s[1].dependencies[0]=2;
 CHECK(zarc_composition_configure(&c,s,3)==ZARC_COMPOSITION_CYCLE);
 s[1].dependency_count=0;s[1].id=2;
 CHECK(zarc_composition_configure(&c,s,3)==ZARC_COMPOSITION_DUPLICATE);
 a.fail_start=false;CHECK(zarc_composition_start(&c)==ZARC_COMPOSITION_OK);
 b.fail_quiesce=true;CHECK(zarc_composition_quiesce(&c,7,8)==ZARC_COMPOSITION_DEGRADED);
 CHECK(!zarc_composition_admits(&c,1)&&!zarc_composition_admits(&c,2));
 CHECK(zarc_composition_suspend(&c)==ZARC_COMPOSITION_INVALID);
 CHECK(zarc_composition_stop(&c)==ZARC_COMPOSITION_OK);
 b.fail_quiesce=false;
 s[1].id=1;s[1].dependency_count=0;s[2].required=false;
 d.fail_start=true;
 CHECK(zarc_composition_configure(&c,s,3)==ZARC_COMPOSITION_OK);
 CHECK(zarc_composition_start(&c)==ZARC_COMPOSITION_DEGRADED);
 CHECK(zarc_composition_required_ready(&c)&&!zarc_composition_admits(&c,3));
 CHECK(zarc_composition_quiesce(&c,9,10)==ZARC_COMPOSITION_OK);
 CHECK(zarc_composition_stop(&c)==ZARC_COMPOSITION_OK);
 puts("Composition contracts passed");return 0;
}

