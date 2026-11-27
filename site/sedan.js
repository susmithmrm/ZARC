(()=>{"use strict";
const svg=document.getElementById("sedan-model");if(!svg)return;
const ns="http://www.w3.org/2000/svg", $=id=>document.getElementById(id);
const colors={Ethernet:"#43e0ee",Services:"#ca8cff",CAN:"#ffcb69",LIN:"#7de79a",IPC:"#81aaff",UART:"#b6a3ff",Sensor:"#b8dce6",Cloud:"#8bceff"};
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
["switch","Automotive Ethernet switch",[0.05,0.29,-0.55],"host","Selected switched backbone connecting application compute and zone controllers; isolation and QoS require an approved configuration.","Architecture candidate."],
["gateway","Vehicle network gateway",[0.84,0.26,-0.12],"legacy","Approved gateway separates legacy and independently allocated domains from high-level service intent.","Product-specific gateway required."],
["charge","Charging / onboard charger",[-1.85,0.35,0.23],"legacy","Independent charging and high-voltage energy domain.","Not a qualified ZARC function."],
["restraint","Restraint / airbag ECU",[0.48,0.24,0],"legacy","Independent occupant protection ECU; no ZARC actuation path is claimed.","Not a qualified ZARC function."],
["suspension","Chassis / suspension ECU",[-1.65,0.27,-0.28],"legacy","Separate chassis and suspension control allocation.","Not a qualified ZARC function."],
["seat","Seat / comfort controller",[-0.25,0.42,0.46],"device","Selected comfort endpoint, potentially hosted on a local intelligent edge node.","Candidate local service."],
["mirror","Mirror controller",[0.72,0.8,-0.99],"device","Local mirror and convenience functions; distinct from camera perception.","Candidate local service."],
["wiper","Wiper / washer ECU",[1.22,0.73,0.35],"device","Allocated visibility service; safety and availability require product validation.","Candidate local service."],
["tpms","Tire pressure receiver",[-1.53,0.5,0.34],"sensor","Receiver for wheel-mounted pressure measurements.","Illustrative sensor interface."],
["rearEdge","ZARC rear smart edge",[-1.92,0.46,-0.52],"zarc","Optional Zephyr-native local endpoint for allocated rear-body behavior.","Candidate smart-edge placement."],
["frontEdge","ZARC front smart edge",[1.92,0.55,0.46],"zarc","Optional Zephyr-native local endpoint near front peripherals.","Candidate smart-edge placement."],
["thermal","Thermal management ECU",[1.54,0.38,0.44],"legacy","Independent thermal control for battery/powertrain, where allocated.","Not a qualified ZARC function."],
["dcdc","DC/DC converter ECU",[-0.93,0.3,-0.37],"legacy","Independent low-voltage power supply and conversion domain.","Not a qualified ZARC function."],
["access","Access / keyless ECU",[-0.55,0.78,-0.83],"device","Approved access and authentication boundary; not raw actuator access.","Candidate body interface."],
["cloud","Fleet / cloud",[-2.75,1.95,-.2],"host","Managed OTA campaigns and telemetry via telematics, not a direct actuator command path.","Production fleet deployment open."]
].map(([id,name,pos,kind,description,maturity])=>({id,name,pos,kind,description,maturity}));
const byId=Object.fromEntries(components.map(c=>[c.id,c]));
const links=[
["host","switch","Ethernet"],["switch","fl","Ethernet"],["switch","fr","Ethernet"],["switch","rl","Ethernet"],["switch","rr","Ethernet"],["switch","gateway","Ethernet"],["host","adas","Ethernet"],["host","gnss","Ethernet"],["host","ioc","UART"],["host","rt","IPC"],["host","cockpit","Services"],["host","fl","Services"],["host","rr","Services"],
["gateway","drive","CAN"],["gateway","brake","CAN"],["gateway","steer","CAN"],["gateway","battery","CAN"],["gateway","charge","CAN"],["gateway","suspension","CAN"],["gateway","thermal","CAN"],["gateway","dcdc","CAN"],["gateway","restraint","CAN"],["rl","rearEdge","LIN"],["fr","frontEdge","LIN"],["edge","seat","LIN"],["fl","mirror","LIN"],["frontEdge","wiper","LIN"],["rearEdge","access","LIN"],["rl","tpms","Sensor"],["fr","hvac","LIN"],["fl","doorl","LIN"],["rr","edge","LIN"],["edge","doorr","LIN"],["fl","lightf","LIN"],["rr","lightr","LIN"],
["adas","fc","Sensor"],["adas","rc","Sensor"],["adas","lc","Sensor"],["adas","sc","Sensor"],["adas","radar","Sensor"],["adas","lidar","Sensor"],["adas","imu","Sensor"],["fr","usf","Sensor"],["rl","usr","Sensor"],["gnss","cloud","Cloud"]];
const visible=new Set(Object.keys(colors));let yaw=-.62,pitch=.34,opacity=.18,zoom=1,focus=[0,.45,0],showInterior=true,cutaway=true,showLabels=true,selected="host",drag=null;
const kindColor={zarc:"#7bf6d7",host:"#65d9ff",sensor:"#d5ecf4",legacy:"#f2b971",device:"#a8b9cc"};
function el(tag,attrs={},text){const node=document.createElementNS(ns,tag);for(const [k,v] of Object.entries(attrs))node.setAttribute(k,String(v));if(text!==undefined)node.textContent=text;return node;}
function project(point){const [x,y,z]=point.map((v,i)=>v-focus[i]);const c=Math.cos(yaw),s=Math.sin(yaw),a=Math.cos(pitch),b=Math.sin(pitch);const u=x*c-z*s,d=x*s+z*c;return [480+u*143*zoom,280-(y*a-d*b)*143*zoom,d*a+y*b];}
function polygon(points,attrs){return el("polygon",{points:points.map(p=>project(p).slice(0,2).join(",")).join(" "),...attrs});}
function stroke(points,color,width,dash){return el("polyline",{points:points.map(p=>project(p).slice(0,2).join(",")).join(" "),fill:"none",stroke:color,"stroke-width":width,"stroke-linecap":"round","stroke-linejoin":"round",...(dash?{"stroke-dasharray":dash}:{})});}
function ring(x,y,r,z){return Array.from({length:25},(_,i)=>{const a=i/24*Math.PI*2;return [x+Math.cos(a)*r,y+Math.sin(a)*r,z];});}
function faceDepth(points){return points.reduce((sum,p)=>sum+project(p)[2],0)/points.length;}
// Original procedural geometry: all positions are illustrative, not vehicle CAD.
const faces=[],details=[];
function face(p,fill,kind="interior",edge="#73818a",alpha=1){faces.push({p,fill,kind,edge,alpha});}
function box(x,y,z,l,h,w,fill,kind="interior",edge="#687b87"){
 const p=[[-1,-1,-1],[1,-1,-1],[1,-1,1],[-1,-1,1],[-1,1,-1],[1,1,-1],[1,1,1],[-1,1,1]].map(([a,b,c])=>[x+a*l/2,y+b*h/2,z+c*w/2]);
 const indices=[[0,1,2,3],[4,7,6,5],[0,4,5,1],[1,5,6,2],[2,6,7,3],[3,7,4,0]];
 indices.forEach((f,i)=>face(f.map(j=>p[j]),fill,kind,edge,i===1?1:.88));
}
function shell(sections,kind,fill){
 const rings=sections.map(([x,w,lo,hi])=>[[x,lo,-w*.80],[x,lo,w*.80],[x,lo+.14,w*.98],[x,hi-.13,w],[x,hi,w*.78],[x,hi,-w*.78],[x,hi-.13,-w],[x,lo+.14,-w*.98]]);
 for(let i=0;i<rings.length-1;i++)for(let j=0;j<8;j++)face([rings[i][j],rings[i][(j+1)%8],rings[i+1][(j+1)%8],rings[i+1][j]],fill,kind,"#a8c6d2");
 face(rings[0],fill,kind);face(rings[rings.length-1],fill,kind);
}
function cylinder(center,r,width,fill,kind="interior",segments=24){
 const [x,y,z]=center,near=ring(x,y,r,z+width/2),far=ring(x,y,r,z-width/2);
 face(near.slice(0,segments),fill,kind,"#70818b");face(far.slice(0,segments),fill,kind,"#70818b");
 for(let i=0;i<segments;i++)face([near[i],near[i+1],far[i+1],far[i]],fill,kind,"#263943",.95);
}
function detail(p,color,width=1.5,kind="interior"){details.push({p,color,width,kind});}
shell([[-2.46,.67,.34,.61],[-2.25,.90,.23,.72],[-1.70,.99,.22,.79],[-1.25,1,.22,.81],[.45,1,.22,.82],[1.30,.99,.22,.78],[2.15,.94,.23,.65],[2.42,.72,.32,.54]],"body","#8faebd");
shell([[-1.42,.80,.76,.82],[-.91,.72,.82,1.32],[-.65,.69,.82,1.43],[.25,.70,.82,1.45],[.52,.72,.82,1.34],[1.00,.81,.78,.83]],"glass","#6c9db2");
// Structural floor, sill rails, axle and battery modules.
box(0,.19,0,4.25,.13,1.64,"#344752");
for(const z of [-.83,.83])box(0,.24,z,3.72,.20,.13,"#9daab1");
for(const x of [-1.53,1.53])box(x,.36,0,.16,.16,1.75,"#82909a");
for(let i=0;i<7;i++)for(const z of [-.33,.33])box(-1.0+i*.31,.32,z,.29,.17,.60,"#465c69","interior","#97a8b0");
box(1.91,.37,0,.54,.33,.77,"#697a83","interior","#adb8be");
for(let i=0;i<8;i++)box(1.69+i*.063,.56,0,.026,.045,.70,"#b4c4cb");
// Front seats with cushions, sculpted backs, head restraints, rails.
for(const z of [-.43,.43]){
 box(.12,.58,z,.61,.17,.56,"#5d6972");
 box(-.15,.84,z,.18,.59,.55,"#7a858e");
 box(-.17,1.16,z,.14,.20,.31,"#8f9ca4");
 for(const dz of [-.18,.18])box(.12,.44,z+dz,.64,.07,.055,"#a1b3bc");
 box(.20,.63,z-.27,.50,.12,.045,"#929ea6");
 box(.20,.63,z+.27,.50,.12,.045,"#929ea6");
}
// Rear bench, cabin console and dashboard.
box(-1.12,.58,0,.56,.17,1.42,"#626f78");
box(-1.39,.84,0,.18,.54,1.42,"#78848c");
for(const z of [-.48,0,.48])box(-1.41,1.14,z,.14,.16,.28,"#8a989f");
box(.05,.57,0,.95,.23,.25,"#293e49");
box(.77,.91,0,.34,.25,1.54,"#374c58");
box(.67,1.07,.0,.055,.30,.48,"#122f40","interior","#9de1f5");
box(.66,1.03,-.51,.055,.20,.40,"#143743","interior","#82c6d8");
for(let i=0;i<4;i++)detail([[.637,1.01+i*.036,-.19],[.637,1.01+i*.036,.19]],"#60c9e7",1.2);
const steering=Array.from({length:33},(_,i)=>[.58,.97+Math.sin(i/32*Math.PI*2)*.17,-.43+Math.cos(i/32*Math.PI*2)*.17]);
detail(steering,"#bccbd3",4.8);
detail([[.58,.97,-.60],[.58,.97,-.26]],"#a4b4be",3);
detail([[.58,.80,-.43],[.58,.97,-.43]],"#a4b4be",3);
// Pillars and roof rails remain recognizable with the skin removed.
for(const z of [-.76,.76]){
 detail([[-1.42,.80,z],[-.85,1.38,z*.86],[.40,1.40,z*.86],[1.0,.80,z]],"#b9c9d1",4.5,"frame");
 detail([[-2.20,.70,z],[-1.42,.80,z],[1,.80,z],[2.24,.60,z]],"#a8bec8",2.2,"frame");
 detail([[-.40,.80,z],[-.40,1.39,z*.86]],"#b5c8d0",2.7,"frame");
}
for(const x of [-1.53,1.53])for(const z of [-1.0,1.0]){
 cylinder([x,.37,z],.41,.23,"#1a252c","wheel");
 cylinder([x,.37,z+(z>0?.12:-.12)],.29,.02,"#6a7d86","wheel");
 const outer=z+(z>0?.137:-.137);
 for(let i=0;i<10;i++){const a=i*Math.PI/5;detail([[x,.37,outer],[x+Math.cos(a)*.265,.37+Math.sin(a)*.265,outer]],"#b8c8d0",2.7,"wheel");}
}
for(const c of components){
 if(c.kind==="zarc"||c.id==="host"||c.id==="adas"){
  const [x,y,z]=c.pos,l=c.id==="host"?.48:.28,w=c.id==="host"?.38:.23,h=c.id==="host"?.18:.11;
  box(x,y,z,l,h,w,"#647d8b","electronics","#b0d1db");
  for(let i=0;i<5;i++)box(x-l*.39+i*l*.195,y+h*.62,z,l*.075,h*.32,w*.86,"#b1c4cc","electronics","#d4e8ed");
  box(x,y-h*.56,z,l*1.04,.023,w*1.04,c.kind==="zarc"?"#22bca8":"#39bce9","electronics");
 }else if(c.kind==="sensor"){
  const [x,y,z]=c.pos;box(x,y,z,.105,.09,.10,"#76858d","electronics","#d3e7ee");
 }
}
function shade(f){
 const p=f.p,a=p[1].map((v,i)=>v-p[0][i]),b=p[2].map((v,i)=>v-p[0][i]),n=[a[1]*b[2]-a[2]*b[1],a[2]*b[0]-a[0]*b[2],a[0]*b[1]-a[1]*b[0]],len=Math.hypot(...n)||1;
 const light=.60+.40*Math.abs((n[0]*.34+n[1]*.87+n[2]*.35)/len);
 return "#"+f.fill.slice(1).match(/../g).map(v=>Math.min(255,Math.round(parseInt(v,16)*light)).toString(16).padStart(2,"0")).join("");
}
function render(){
 svg.replaceChildren(el("title",{},"Transparent sedan, selected component "+byId[selected].name));
 const defs=el("defs"),glow=el("filter",{id:"sedan-glow",x:"-50%",y:"-50%",width:"200%",height:"200%"});
 glow.append(el("feGaussianBlur",{stdDeviation:2.6}));defs.append(glow);svg.append(defs);
 for(let x=-3;x<=3;x++)svg.append(stroke([[x,0,-1.7],[x,0,1.7]],"#264452",.7));
 for(let z=-1.5;z<=1.5;z+=.5)svg.append(stroke([[-3,0,z],[3,0,z]],"#264452",.7));
 const drawFaces=faces.filter(f=>showInterior||f.kind!=="interior").sort((a,b)=>faceDepth(a.p)-faceDepth(b.p));
 for(const f of drawFaces){
  const skin=f.kind==="body"||f.kind==="glass";
  const side=f.p.reduce((sum,p)=>sum+p[2],0)/f.p.length;
  // Cut the near-side skin away, leaving the recognizable structural rails.
  if(skin&&cutaway&&Math.abs(side)>.38&&side*Math.cos(yaw)>0)continue;
  svg.append(polygon(f.p,{fill:shade(f),"fill-opacity":skin?opacity*(f.kind==="glass"?.67:1):f.alpha,stroke:f.edge,"stroke-opacity":skin?.52:.53,"stroke-width":skin?.9:.45}));
 }
 for(const d of details)if(showInterior||d.kind!=="interior")svg.append(stroke(d.p,d.color,d.width));
 // Bundled schematic cable paths along the floor rails, not electrical wiring specifications.
 if(visible.has("Ethernet")||visible.has("CAN"))for(const z of [-.67,.67])for(let i=0;i<3;i++){
  const color=i===2?colors.Ethernet:"#b87050";
  if(i===2&&!visible.has("Ethernet")||i!==2&&!visible.has("CAN"))continue;
  svg.append(stroke([[-2.1,.29,z+i*.022],[-1.4,.29,z+i*.022],[.1,.29,z+i*.022],[1.45,.29,z+i*.022],[2.05,.40,z+i*.022]],color,2));
 }
 for(const [a,b,kind] of links){if(!visible.has(kind))continue;const p=byId[a].pos,q=byId[b].pos,active=a===selected||b===selected;
 const route=kind==="Ethernet"?[p,[p[0],.25,p[2]],[q[0],.25,p[2]],[q[0],.25,q[2]],q]:[p,q];
 if(active){const halo=stroke(route,colors[kind],5);halo.setAttribute("filter","url(#sedan-glow)");halo.setAttribute("opacity",".55");svg.append(halo);} svg.append(stroke(route,colors[kind],active?3:1.4,kind==="Services"||kind==="Cloud"?"6 5":null));}
 for(const c of components.slice().sort((a,b)=>project(a.pos)[2]-project(b.pos)[2])){
 const [x,y]=project(c.pos),active=c.id===selected,large=c.kind==="zarc"||c.id==="host";const color=kindColor[c.kind];
 const g=el("g",{cursor:"pointer","data-component":c.id});g.append(el("title",{},c.name));
 g.append(el("circle",{cx:x,cy:y,r:active?12:large?8:5,fill:color,"fill-opacity":.15,stroke:color,"stroke-width":active?2.5:1.3}));
 g.append(el("rect",{x:x-(large?6:3),y:y-(large?5:3),width:large?12:6,height:large?10:6,rx:2,fill:color}));
 if(showLabels&&c.kind==="zarc"&&!active){const short={ioc:"IOC",rt:"RT island",fl:"FL zone",fr:"FR zone",rl:"RL zone",rr:"RR zone",edge:"Edge",frontEdge:"Front edge",rearEdge:"Rear edge"}[c.id];g.append(el("text",{x:x+10,y:y-10,fill:color,stroke:"#081c29","stroke-width":3,"paint-order":"stroke","font-size":11,"font-weight":700,"font-family":"Arial,sans-serif"},short));}
 g.addEventListener("click",()=>{if(!drag?.moved)select(c.id);});svg.append(g);
 }
 const chosen=byId[selected],[x,y]=project(chosen.pos),labelX=Math.max(12,Math.min(720,x+24)),labelY=Math.max(22,Math.min(490,y-42));
 svg.append(el("path",{d:"M"+x+" "+y+" L"+labelX+" "+(labelY+15),stroke:kindColor[chosen.kind],"stroke-width":1.5,fill:"none"}));
 const group=el("g",{"pointer-events":"none"});group.append(el("rect",{x:labelX,y:labelY,width:228,height:32,rx:5,fill:"#102b39",stroke:kindColor[chosen.kind]}));group.append(el("text",{x:labelX+9,y:labelY+20,fill:"#f1ffff","font-size":13,"font-weight":700,"font-family":"Arial,sans-serif"},chosen.name));svg.append(group);
 svg.append(el("text",{x:18,y:24,fill:"#a8c9d2","font-size":12,"font-family":"Arial,sans-serif"},"CONCEPT SEDAN  /  FRONT → +X  /  NOT A WIRING DIAGRAM"));
}
function select(id){selected=id;const c=byId[id],detail=$("sedan-detail");detail.replaceChildren();for(const [tag,content] of [["h3",c.name],["p",c.description],["small",c.maturity]]){const e=document.createElement(tag);e.textContent=content;detail.append(e);}
 for(const b of $("sedan-components").querySelectorAll("button"))b.setAttribute("aria-pressed",String(b.dataset.id===id));render();globalThis.zarcSedan3D?.select(id);}
for(const c of components){const b=document.createElement("button");b.type="button";b.textContent=c.name;b.dataset.id=c.id;b.style.borderLeft="3px solid "+kindColor[c.kind];b.addEventListener("click",()=>select(c.id));$("sedan-components").append(b);}
for(const [kind,color] of Object.entries(colors)){const label=document.createElement("label"),input=document.createElement("input"),name=document.createElement("span");input.type="checkbox";input.checked=true;input.style.accentColor=color;name.textContent=({Services:"SOME/IP / SDV services",CAN:"CAN / CAN FD",Sensor:"Sensor links",Cloud:"Fleet link"})[kind]||kind;name.style.color=color;label.append(input,name);$("sedan-layers").append(label);input.addEventListener("change",()=>{if(input.checked)visible.add(kind);else visible.delete(kind);render();});}
function view(a,b,z=1,f=[0,.45,0]){yaw=a;pitch=b;zoom=z;focus=f;$("sedan-zoom").value=Math.round(z*100);render();}
$("sedan-left").onclick=()=>{yaw-=.22;render();};$("sedan-right").onclick=()=>{yaw+=.22;render();};
$("sedan-top").onclick=()=>view(0,1.56);$("sedan-side").onclick=()=>view(0,.12,1.12);$("sedan-cabin").onclick=()=>view(-.35,.23,1.65,[.04,.70,0]);
$("sedan-reset").onclick=()=>{opacity=.18;showInterior=true;cutaway=true;showLabels=true;$("sedan-opacity").value=18;$("sedan-interior").checked=$("sedan-cutaway").checked=$("sedan-labels").checked=true;view(-.62,.34);};
$("sedan-opacity").oninput=e=>{opacity=Number(e.target.value)/100;render();};$("sedan-zoom").oninput=e=>{zoom=Number(e.target.value)/100;render();};
$("sedan-interior").onchange=e=>{showInterior=e.target.checked;render();};$("sedan-cutaway").onchange=e=>{cutaway=e.target.checked;render();};$("sedan-labels").onchange=e=>{showLabels=e.target.checked;render();};
svg.addEventListener("pointerdown",e=>{drag={x:e.clientX,y:e.clientY,moved:false};});svg.addEventListener("pointermove",e=>{if(!drag)return;const dx=e.clientX-drag.x,dy=e.clientY-drag.y;if(Math.abs(dx)+Math.abs(dy)>2){drag.moved=true;svg.setPointerCapture(e.pointerId);yaw+=dx*.008;pitch=Math.max(.08,Math.min(1.56,pitch+dy*.008));drag.x=e.clientX;drag.y=e.clientY;render();}});svg.addEventListener("pointerup",()=>{drag=null;});svg.addEventListener("pointercancel",()=>{drag=null;});svg.addEventListener("pointerleave",()=>{if(!drag?.moved)drag=null;});select("host");
globalThis.zarcSedanData={components,links,colors,kindColor,select};
})();