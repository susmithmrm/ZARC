(() => {
 "use strict";
 const $=id=>document.getElementById(id);
 const endpoint=$("telemetry-endpoint"),button=$("telemetry-connect"),state=$("telemetry-state");
 const sample=$("telemetry-sample"),quality=$("telemetry-quality"),epoch=$("telemetry-epoch");
 if(!endpoint||!button||!state)return;
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
  button.textContent="Disconnect";
  poll();timer=setInterval(poll,500);
 });
 paint("simulated","SIMULATED · illustrative baseline; no controller connected");
 globalThis.zarcTelemetry={render,poll,freshness};
})();
