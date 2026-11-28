(() => {
 "use strict";
 const status=document.getElementById("support-matrix-status");
 const body=document.getElementById("support-matrix-rows");
 if(!status||!body)return;
 function render(data){
  const valid=data&&data.schema===1&&Array.isArray(data.records)&&
   Array.isArray(data.production_supported)&&data.production_supported.length===0&&
   data.records.every(row=>row&&row.status==="CANDIDATE"&&
    ["target","profile","board","soc","revision","zephyr","toolchain","maintainer","manifest"]
     .every(key=>typeof row[key]==="string"&&row[key].length>0)&&
    /^config\/targets\/[a-z0-9-]+\.json$/.test(row.manifest)&&Array.isArray(row.limitations));
  if(!valid){status.textContent="Target records unavailable: generated support data failed validation.";return false;}
  body.replaceChildren();
  for(const row of data.records){
   const tr=document.createElement("tr");
   const enabled=Object.entries(row.features||{}).filter(([,value])=>value===true).map(([key])=>key).join(", ")||"None declared";
   const cells=[row.target,row.profile,row.board+" / "+row.soc,row.zephyr+" / "+row.toolchain,
    row.status,row.maintainer,enabled,row.limitations.join("; ")||"None listed"];
   for(const value of cells){const td=document.createElement("td");td.textContent=value;tr.append(td);}
   const recordCell=document.createElement("td"),link=document.createElement("a");
   link.href="https://github.com/susmithmrm/ZARC/blob/main/"+row.manifest;
   link.textContent="Candidate manifest";recordCell.append(link);tr.append(recordCell);
   body.append(tr);
  }
  status.textContent=data.scope;
  return true;
 }
 globalThis.zarcSupportMatrix={render};
 fetch("support-matrix.json",{cache:"no-store"})
  .then(response=>{if(!response.ok)throw new Error("matrix unavailable");return response.json();})
  .then(data=>{if(!render(data))throw new Error("invalid support matrix");})
  .catch(()=>{status.textContent="Target records unavailable. No support claim is made.";});
})();
