const fs=require("node:fs"),vm=require("node:vm"),assert=require("node:assert/strict");
const elements={};
class Element{
 constructor(){this.dataset={};this.textContent="";this.value="http://127.0.0.1:8765/snapshot";this.events={};}
 addEventListener(k,fn){this.events[k]=fn;}
}
for(const id of ["telemetry-endpoint","telemetry-connect","telemetry-state","telemetry-sample","telemetry-quality","telemetry-epoch"])elements[id]=new Element();
let now=10000;
const context={document:{getElementById:id=>elements[id]},Date:{now:()=>now},setInterval:()=>1,clearInterval:()=>{},fetch:async()=>({ok:false})};
vm.runInNewContext(fs.readFileSync("site/telemetry.js","utf8"),context);
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
console.log("Sedan telemetry provenance and freshness contract passed");
