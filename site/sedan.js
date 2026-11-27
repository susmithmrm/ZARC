(()=>{"use strict";
const svg=document.getElementById("sedan-model");if(!svg)return;
const ns="http://www.w3.org/2000/svg", $=id=>document.getElementById(id);
const colors={Ethernet:"#43e0ee",Services:"#ca8cff",CAN:"#ffcb69",LIN:"#7de79a",IPC:"#81aaff",Sensor:"#b8dce6",Cloud:"#8bceff"};
const components=[
["host","Central SoC / ZARC Host",[0,0.40,0],"host","AAOS / SDV applications run on the application processor; trusted ZARC Host mediates typed intent, identity and observed outcomes.","Reference broker / generated gRPC tested; OEM host deployment open."],
["ioc","ZARC companion IOC",[.65,.35,0],"zarc","Independent MCU with native Zephyr for allocated local policy, health, wake and power coordination.","Simulator profile; physical power/reset integration open."],
["rt","Integrated RT island",[-.60,.36,0],"zarc","Optional on-SoC real-time core running ZARC + Zephyr using a selected IPC backend. Alternative to external ownership for the same resource.","Simulated IPC profile; actual SoC isolation open."],
["fl","ZARC front-left zone",[1.35,.30,-.68],"zarc","Local front-left I/O ownership and bounded service admission.","Proposed zone; physical hardware qualification open."],
["fr","ZARC front-right zone",[1.35,.30,.68],"zarc","Local front-right I/O ownership, sensors and body peripherals.","Proposed zone; physical hardware qualification open."],
["rl","ZARC rear-left zone",[-1.40,.30,-.68],"zarc","Local rear-left body I/O, switches and lighting.","Proposed zone; physical hardware qualification open."],
["rr","ZARC rear-right zone",[-1.40,.30,.68],"zarc","Local rear-right I/O and independent observations.","Proposed zone; physical hardware qualification open."],
["edge","ZARC intelligent edge",[-.65,.55,.78],"zarc","A small standalone Zephyr node close to its allocated sensor or peripheral; host optional.","Simulator autonomy tested; device-specific validation open."],
["fc","Front camera",[.93,1.26,0],"sensor","Forward vision; perception processed in an independently allocated compute domain.","Illustrative sensor."],
["rc","Rear camera",[-2.24,.72,0],"sensor","Rear visibility and parking perception.","Illustrative sensor."],
["lc","Left surround camera",[.53,.85,-1.03],"sensor","Surround-view input; selected sensor link to perception compute.","Illustrative sensor."],
["sc","Right surround camera",[.53,.85,1.03],"sensor","Surround-view input; selected sensor link to perception compute.","Illustrative sensor."],
["radar","Front radar",[2.32,.48,0],"sensor","Independent perception input; not a ZARC safety function.","Illustrative sensor."],
["lidar","Optional lidar",[.05,1.53,0],"sensor","Optional high-bandwidth perception sensor.","Illustrative sensor."],
["usf","Front ultrasonic",[2.31,.33,.63],"sensor","Close-range sensing through a selected local interface.","Illustrative sensor."],
["usr","Rear ultrasonic",[-2.30,.33,-.63],"sensor","Close-range rear sensing through a selected local interface.","Illustrative sensor."],
["gnss","GNSS / telematics",[-.95,1.44,0],"host","Positioning, connectivity, and fleet update coordination via approved gateway.","Proposed integration."],
["imu","IMU",[0,.24,-.34],"sensor","Motion and attitude observations; placement and time synchronization depend on vehicle design.","Illustrative sensor."],
["adas","ADAS compute",[1.03,.48,-.32],"host","Separate perception and driving functions with independent safety allocation.","Not qualified by ZARC."],
["cockpit","Digital cockpit",[.78,.92,0],"host","Vehicle UI and infotainment consume semantic services through the host.","OEM integration open."],
["hvac","HVAC",[.98,.60,.34],"device","Selected body and comfort service candidate.","Illustrative endpoint."],
["doorl","Left doors / windows",[.00,.65,-.94],"device","Local body I/O through an allocated zone.","Illustrative endpoint."],
["doorr","Right doors / windows",[.00,.65,.94],"device","Local body I/O through an allocated zone or smart edge.","Illustrative endpoint."],
["lightf","Front lights",[2.17,.57,-.72],"device","Benign indicator and lighting concept; no production light safety claim.","Illustrative endpoint."],
["lightr","Rear lights",[-2.21,.57,.72],"device","Local rear lighting and observed status.","Illustrative endpoint."],
["drive","Powertrain / inverter",[1.78,.36,0],"legacy","Independently allocated propulsion control, exposed only through approved service or gateway boundaries.","Not a qualified ZARC function."],
["battery","Battery management",[-.15,.14,.40],"legacy","Independently allocated battery supervision and energy management.","Not a qualified ZARC function."],
["brake","Brake ECU",[1.69,.25,.33],"legacy","Retains independently approved braking authority.","Not a qualified ZARC function."],
["steer","Steering ECU",[1.51,.25,-.33],"legacy","Retains independently approved steering authority.","Not a qualified ZARC function."],
["cloud","Fleet / cloud",[-2.75,1.95,-.2],"host","Managed OTA campaigns and telemetry via telematics, not a direct actuator command path.","Production fleet deployment open."]
].map(([id,name,pos,kind,description,maturity])=>({id,name,pos,kind,description,maturity}));
const byId=Object.fromEntries(components.map(c=>[c.id,c]));
const links=[
["host","fl","Ethernet"],["host","fr","Ethernet"],["host","rl","Ethernet"],["host","rr","Ethernet"],["host","adas","Ethernet"],["host","gnss","Ethernet"],["host","ioc","IPC"],["host","rt","IPC"],["host","cockpit","Services"],["host","fl","Services"],["host","rr","Services"],
["fl","drive","CAN"],["fr","brake","CAN"],["fl","steer","CAN"],["rl","battery","CAN"],["fr","hvac","LIN"],["fl","doorl","LIN"],["rr","edge","LIN"],["edge","doorr","LIN"],["fl","lightf","LIN"],["rr","lightr","LIN"],
["adas","fc","Sensor"],["adas","rc","Sensor"],["adas","lc","Sensor"],["adas","sc","Sensor"],["adas","radar","Sensor"],["adas","lidar","Sensor"],["adas","imu","Sensor"],["fr","usf","Sensor"],["rl","usr","Sensor"],["gnss","cloud","Cloud"]];
const visible=new Set(Object.keys(colors));let yaw=-.74,pitch=.40,opacity=.18,selected="host",drag=null;
const kindColor={zarc:"#7bf6d7",host:"#65d9ff",sensor:"#d5ecf4",legacy:"#f2b971",device:"#a8b9cc"};
function el(tag,attrs={},text){const node=document.createElementNS(ns,tag);for(const [k,v] of Object.entries(attrs))node.setAttribute(k,String(v));if(text!==undefined)node.textContent=text;return node;}
function project([x,y,z]){const c=Math.cos(yaw),s=Math.sin(yaw),a=Math.cos(pitch),b=Math.sin(pitch);const u=x*c-z*s,d=x*s+z*c;return [480+u*143,288-(y*a-d*b)*143,d*a+y*b];}
function polygon(points,attrs){return el("polygon",{points:points.map(p=>project(p).slice(0,2).join(",")).join(" "),...attrs});}
function stroke(points,color,width,dash){return el("polyline",{points:points.map(p=>project(p).slice(0,2).join(",")).join(" "),fill:"none",stroke:color,"stroke-width":width,"stroke-linecap":"round","stroke-linejoin":"round",...(dash?{"stroke-dasharray":dash}:{})});}
function ring(x,y,r,z){return Array.from({length:25},(_,i)=>{const a=i/24*Math.PI*2;return [x+Math.cos(a)*r,y+Math.sin(a)*r,z];});}
function faceDepth(points){return points.reduce((sum,p)=>sum+project(p)[2],0)/points.length;}
const faces=[];
function loft(sections,fill,strokeColor){
 const rings=sections.map(([x,w,lo,hi])=>[[x,lo,-w*.82],[x,lo,w*.82],[x,hi,w],[x,hi,-w]]);
 for(let i=0;i<rings.length-1;i++)for(let j=0;j<4;j++)faces.push({p:[rings[i][j],rings[i][(j+1)%4],rings[i+1][(j+1)%4],rings[i+1][j]],fill,strokeColor});
 faces.push({p:rings[0],fill,strokeColor},{p:rings[rings.length-1],fill,strokeColor});
}
loft([[-2.45,.70,.28,.62],[-2.20,.92,.20,.72],[-1.55,.99,.18,.75],[0,1,.18,.80],[1.60,.98,.18,.76],[2.23,.92,.23,.62],[2.45,.70,.30,.53]],"#3a8aa9","#85bed1");
loft([[-1.34,.83,.74,.80],[-.83,.69,.79,1.35],[.33,.69,.79,1.42],[.96,.81,.78,.80]],"#5caac0","#a3dbe8");
function render(){
 svg.replaceChildren(el("title",{},"Transparent sedan, selected component "+byId[selected].name));
 for(let x=-3;x<=3;x++)svg.append(stroke([[x,0,-1.7],[x,0,1.7]],"#264452",.7));
 for(let z=-1.5;z<=1.5;z+=.5)svg.append(stroke([[-3,0,z],[3,0,z]],"#264452",.7));
 for(const x of [-1.55,1.55])for(const z of [-1.02,1.02]){
  const near=ring(x,.34,.43,z),far=ring(x,.34,.43,z+(z>0?-.18:.18));
  svg.append(polygon(far,{fill:"#10212c",stroke:"#617d88","stroke-width":1.4}));
  for(let i=0;i<24;i++)svg.append(polygon([near[i],near[i+1],far[i+1],far[i]],{fill:"#172b36",stroke:"#273f4a","stroke-width":.6}));
  svg.append(polygon(near,{fill:"#122530",stroke:"#9ab3bf","stroke-width":2}));
  svg.append(polygon(ring(x,.34,.30,z),{fill:"#324a55",stroke:"#aac4d1","stroke-width":1.5}));
  for(let i=0;i<8;i++){const a=i*Math.PI/4;svg.append(stroke([[x,.34,z],[x+Math.cos(a)*.29,.34+Math.sin(a)*.29,z]],"#a0b6c0",2));}
 }
 for(const f of faces.slice().sort((a,b)=>faceDepth(a.p)-faceDepth(b.p)))svg.append(polygon(f.p,{fill:f.fill,"fill-opacity":opacity,stroke:f.strokeColor,"stroke-opacity":.6,"stroke-width":1.1}));
 for(const z of [-.85,.85]){svg.append(stroke([[-2.18,.72,z],[-1.32,.77,z],[.96,.80,z],[2.17,.62,z]],"#c0d7df",1.5));svg.append(stroke([[-.42,.78,z],[-.42,.25,z]],"#81a5b5",1));}
 for(const [a,b,kind] of links){if(!visible.has(kind))continue;const p=byId[a].pos,q=byId[b].pos,active=a===selected||b===selected;
 const route=kind==="Ethernet"?[p,[p[0],.25,p[2]],[q[0],.25,p[2]],[q[0],.25,q[2]],q]:[p,q];
 svg.append(stroke(route,colors[kind],active?3.2:1.5,kind==="Services"||kind==="Cloud"?"6 5":null));}
 for(const c of components.slice().sort((a,b)=>project(a.pos)[2]-project(b.pos)[2])){
 const [x,y]=project(c.pos),active=c.id===selected,large=c.kind==="zarc"||c.id==="host";const color=kindColor[c.kind];
 const g=el("g",{cursor:"pointer","data-component":c.id});g.append(el("title",{},c.name));
 g.append(el("circle",{cx:x,cy:y,r:active?12:large?8:5,fill:color,"fill-opacity":.15,stroke:color,"stroke-width":active?2.5:1.3}));
 g.append(el("rect",{x:x-(large?6:3),y:y-(large?5:3),width:large?12:6,height:large?10:6,rx:2,fill:color}));
 g.addEventListener("click",()=>{if(!drag?.moved)select(c.id);});svg.append(g);
 }
 const chosen=byId[selected],[x,y]=project(chosen.pos),labelX=Math.max(12,Math.min(720,x+24)),labelY=Math.max(22,Math.min(490,y-42));
 svg.append(el("path",{d:"M"+x+" "+y+" L"+labelX+" "+(labelY+15),stroke:kindColor[chosen.kind],"stroke-width":1.5,fill:"none"}));
 const group=el("g",{"pointer-events":"none"});group.append(el("rect",{x:labelX,y:labelY,width:228,height:32,rx:5,fill:"#102b39",stroke:kindColor[chosen.kind]}));group.append(el("text",{x:labelX+9,y:labelY+20,fill:"#f1ffff","font-size":13,"font-weight":700,"font-family":"Arial,sans-serif"},chosen.name));svg.append(group);
 svg.append(el("text",{x:18,y:24,fill:"#a8c9d2","font-size":12,"font-family":"Arial,sans-serif"},"CONCEPT SEDAN  /  FRONT → +X  /  NOT A WIRING DIAGRAM"));
}
function select(id){selected=id;const c=byId[id],detail=$("sedan-detail");detail.replaceChildren();for(const [tag,content] of [["h3",c.name],["p",c.description],["small",c.maturity]]){const e=document.createElement(tag);e.textContent=content;detail.append(e);}
 for(const b of $("sedan-components").querySelectorAll("button"))b.setAttribute("aria-pressed",String(b.dataset.id===id));render();}
for(const c of components){const b=document.createElement("button");b.type="button";b.textContent=c.name;b.dataset.id=c.id;b.style.borderLeft="3px solid "+kindColor[c.kind];b.addEventListener("click",()=>select(c.id));$("sedan-components").append(b);}
for(const [kind,color] of Object.entries(colors)){const label=document.createElement("label"),input=document.createElement("input"),name=document.createElement("span");input.type="checkbox";input.checked=true;input.style.accentColor=color;name.textContent=({Services:"SOME/IP / SDV services",CAN:"CAN / CAN FD",Sensor:"Sensor links",Cloud:"Fleet link"})[kind]||kind;name.style.color=color;label.append(input,name);$("sedan-layers").append(label);input.addEventListener("change",()=>{if(input.checked)visible.add(kind);else visible.delete(kind);render();});}
$("sedan-left").onclick=()=>{yaw-=.22;render();};$("sedan-right").onclick=()=>{yaw+=.22;render();};$("sedan-top").onclick=()=>{yaw=0;pitch=1.56;render();};$("sedan-reset").onclick=()=>{yaw=-.74;pitch=.40;opacity=.18;$("sedan-opacity").value=18;render();};$("sedan-opacity").oninput=e=>{opacity=Number(e.target.value)/100;render();};
svg.addEventListener("pointerdown",e=>{drag={x:e.clientX,y:e.clientY,moved:false};});svg.addEventListener("pointermove",e=>{if(!drag)return;const dx=e.clientX-drag.x,dy=e.clientY-drag.y;if(Math.abs(dx)+Math.abs(dy)>2){drag.moved=true;svg.setPointerCapture(e.pointerId);yaw+=dx*.008;pitch=Math.max(.08,Math.min(1.56,pitch+dy*.008));drag.x=e.clientX;drag.y=e.clientY;render();}});svg.addEventListener("pointerup",()=>{drag=null;});svg.addEventListener("pointercancel",()=>{drag=null;});svg.addEventListener("pointerleave",()=>{if(!drag?.moved)drag=null;});select("host");
})();