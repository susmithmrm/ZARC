const fs=require("node:fs"),path=require("node:path"),vm=require("node:vm"),assert=require("node:assert/strict");
const elements={};
class Element{
 constructor(tag){this.tagName=tag;this.dataset={};this.textContent="";this.value="http://127.0.0.1:8765/snapshot";this.events={};this.children=[];this.attributes={};}
 addEventListener(k,fn){this.events[k]=fn;}
 setAttribute(k,v){this.attributes[k]=v;}
 append(...nodes){this.children.push(...nodes);}
 replaceChildren(...nodes){this.children=[...nodes];}
}
for(const id of ["telemetry-endpoint","telemetry-connect","telemetry-state","telemetry-sample","telemetry-quality","telemetry-epoch","telemetry-simulated-state","telemetry-simulated-channels"])elements[id]=new Element(id);
let now=10000;
const context={document:{getElementById:id=>elements[id],createElement:tag=>new Element(tag)},Date:{now:()=>now},setInterval:()=>1,clearInterval:()=>{},fetch:async()=>({ok:false})};
vm.runInNewContext(fs.readFileSync(path.join(__dirname,"..","site","telemetry.js"),"utf8"),context);
assert.equal(elements["telemetry-state"].dataset.state,"simulated");
assert.match(elements["telemetry-state"].textContent,/SIMULATED/);
context.zarcTelemetry.render({schema:1,provenance:"simulated_controller",status:"COMPLETED",value:1,quality:1,generation:8,boot:42,captured_at_ms:now});
assert.equal(elements["telemetry-state"].dataset.state,"live");
assert.match(elements["telemetry-state"].textContent,/LIVE.*simulated reference controller/);
assert.equal(elements["telemetry-sample"].textContent,"ON");
assert.equal(elements["telemetry-epoch"].textContent,"42 / 8");
now+=2000;context.zarcTelemetry.freshness();
assert.equal(elements["telemetry-state"].dataset.state,"stale");
assert.match(elements["telemetry-state"].textContent,/STALE/);
context.zarcTelemetry.render({schema:1,provenance:"simulated_controller",status:"COMPLETED",value:0,quality:2,generation:9,boot:42,captured_at_ms:now});
assert.equal(elements["telemetry-state"].dataset.state,"stale");
assert.equal(elements["telemetry-quality"].textContent,"STALE");
const channels=Array.from({length:50},(_,i)=>({id:`BENCH-${String(i+1).padStart(3,"0")}`,name:`Signal ${i+1}`,group:"reference",direction:"input",source:"deterministic_reference_scenario",value:i===0?0:i,unit:"count",quality:"SIMULATED",sample_tick_ms:0,provenance:"simulated_reference_bench",value_min:0,value_max:49}));
const catalogue={schema:1,catalogue_id:"reference-bench-50-v1",status:"SIMULATED",provenance:"simulated_reference_bench",sample_tick_ms:0,channels};
assert.equal(context.zarcTelemetry.renderSimulatedCatalogue(catalogue),true);
assert.match(elements["telemetry-simulated-state"].textContent,/SIMULATED.*separate from controller feedback/);
const table=elements["telemetry-simulated-channels"].children[0];
assert.equal(table.tagName,"table");
assert.equal(table.children[1].children.length,50);
for(const row of table.children[1].children){
 assert.equal(row.dataset.provenance,"simulated_reference_bench");
 assert.equal(row.children[4].dataset.provenance,"simulated");
 assert.match(row.children[4].textContent,/SIMULATED/);
}
assert.equal(context.zarcTelemetry.renderSimulatedCatalogue({...catalogue,provenance:"simulated_controller"}),false);
assert.equal(context.zarcTelemetry.renderSimulatedCatalogue({...catalogue,channels:channels.slice(0,49)}),false);
console.log("Sedan telemetry provenance and freshness contract passed");
