(() => {
 "use strict";
 const $=id=>document.getElementById(id);
 const endpoint=$("telemetry-endpoint"),button=$("telemetry-connect"),state=$("telemetry-state");
 const sample=$("telemetry-sample"),quality=$("telemetry-quality"),epoch=$("telemetry-epoch");
 const simulatedState=$("telemetry-simulated-state"),simulatedChannels=$("telemetry-simulated-channels");
 if(!endpoint||!button||!state||!simulatedState||!simulatedChannels)return;
 const staleAfterMs=1500;
 let timer=null,lastGood=null,inFlight=false;
 const qualities=["UNKNOWN","VALID","STALE","FAULT"];
 const values=["OFF","ON","FAULT","INVALID"];
 function paint(name,message){state.dataset.state=name;state.textContent=message;}
 function freshness(){
  if(lastGood&&Date.now()-lastGood.captured_at_ms>staleAfterMs){
   paint("stale","STALE · last controller sample is too old");
  }
 }
 function render(record){
  if(!record||record.schema!==1||record.provenance!=="simulated_controller")return;
  if(record.status==="COMPLETED"&&Number.isInteger(record.value)&&values[record.value]&&
     Number.isInteger(record.quality)&&qualities[record.quality]&&
     Number.isFinite(record.captured_at_ms)&&Date.now()-record.captured_at_ms<=staleAfterMs){
   lastGood={value:record.value,quality:record.quality,generation:record.generation,
             boot:record.boot,captured_at_ms:record.captured_at_ms};
   sample.textContent=values[lastGood.value];
   quality.textContent=qualities[lastGood.quality];
   epoch.textContent=String(lastGood.boot)+" / "+String(lastGood.generation);
   if(lastGood.quality===1)paint("live","LIVE · simulated reference controller");
   else paint("stale","STALE · controller quality "+qualities[lastGood.quality]);
  }else{
   freshness();
   if(lastGood&&Date.now()-lastGood.captured_at_ms>staleAfterMs)paint("stale","STALE · last controller sample is too old");
  }
 }
 function renderSimulatedCatalogue(catalogue){
  if(!catalogue||catalogue.schema!==1||catalogue.catalogue_id!=="reference-bench-50-v1"||catalogue.status!=="SIMULATED"||
     catalogue.provenance!=="simulated_reference_bench"||catalogue.sample_tick_ms!==0||
     !Array.isArray(catalogue.channels)||catalogue.channels.length!==50)return false;
  const seen=new Set();
  for(const channel of catalogue.channels){
   if(!channel||typeof channel.id!=="string"||!channel.id||seen.has(channel.id)||
      typeof channel.name!=="string"||!channel.name||typeof channel.group!=="string"||!channel.group||
      typeof channel.source!=="string"||!channel.source||typeof channel.unit!=="string"||!channel.unit||
      !["input","feedback","status"].includes(channel.direction)||
      channel.quality!=="SIMULATED"||channel.provenance!=="simulated_reference_bench"||
      !Number.isFinite(channel.value)||!Number.isFinite(channel.value_min)||
      !Number.isFinite(channel.value_max)||channel.value_min>channel.value||
      channel.value>channel.value_max||channel.sample_tick_ms!==0)return false;
   seen.add(channel.id);
  }
  const table=document.createElement("table");
  table.setAttribute("aria-label","Simulated reference bench channels");
  const head=document.createElement("thead"),headRow=document.createElement("tr");
  for(const label of ["ID","Signal","Value","Unit","Quality / provenance","Scenario tick"]){
   const cell=document.createElement("th");cell.scope="col";cell.textContent=label;headRow.append(cell);
  }
  head.append(headRow);
  const body=document.createElement("tbody");
  for(const channel of catalogue.channels){
   const row=document.createElement("tr");
   for(const value of [channel.id,channel.name,String(channel.value),channel.unit,
                       "SIMULATED · reference fixture",String(channel.sample_tick_ms)]){
    const cell=document.createElement("td");cell.textContent=value;
    if(value.startsWith("SIMULATED"))cell.dataset.provenance="simulated";
    row.append(cell);
   }
   row.dataset.provenance=channel.provenance;
   body.append(row);
  }
  table.append(head,body);
  simulatedChannels.replaceChildren(table);
  simulatedState.textContent="SIMULATED · 50 deterministic channels; separate from controller feedback";
  return true;
 }
 async function loadSimulatedCatalogue(){
  try{
   const url=new URL(endpoint.value);
   if(url.protocol!=="http:"||!(url.hostname==="127.0.0.1"||url.hostname==="localhost")||url.pathname!=="/snapshot")
    throw new Error("Use the localhost telemetry bridge");
   url.pathname="/channels";
   const response=await fetch(url.href,{cache:"no-store",mode:"cors"});
   if(!response.ok||!renderSimulatedCatalogue(await response.json()))throw new Error("invalid simulated catalogue");
  }catch(_error){
   simulatedState.textContent="SIMULATED · reference catalogue unavailable; no controller feedback is used";
   simulatedChannels.replaceChildren();
  }
 }
 async function poll(){
  if(inFlight)return;
  inFlight=true;
  try{
   const url=new URL(endpoint.value);
   if(url.protocol!=="http:"||!(url.hostname==="127.0.0.1"||url.hostname==="localhost")||url.pathname!=="/snapshot")
    throw new Error("Use the localhost /snapshot bridge URL");
   const response=await fetch(url.href,{cache:"no-store",mode:"cors"});
   if(!response.ok)throw new Error("bridge unavailable");
   render(await response.json());
  }catch(_error){freshness();}
  finally{inFlight=false;}
 }
 button.addEventListener("click",()=>{
  if(timer!==null){clearInterval(timer);timer=null;button.textContent="Connect read only";freshness();return;}
  try{
   const url=new URL(endpoint.value);
   if(url.protocol!=="http:"||!(url.hostname==="127.0.0.1"||url.hostname==="localhost")||url.pathname!=="/snapshot")
    throw new Error();
  }catch(_error){paint("error","Enter http://127.0.0.1:PORT/snapshot");return;}
  loadSimulatedCatalogue();
  button.textContent="Disconnect";
  poll();timer=setInterval(poll,500);
 });
 paint("simulated","SIMULATED · illustrative baseline; no controller connected");
 loadSimulatedCatalogue();
 globalThis.zarcTelemetry={render,poll,freshness,renderSimulatedCatalogue,loadSimulatedCatalogue};
})();
