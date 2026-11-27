import * as T from './vendor/three/three.module.min.js';
import {OrbitControls} from './vendor/three/OrbitControls.js';

const $=id=>document.getElementById(id);
function launch(){
 const data=globalThis.zarcSedanData,stage=$('sedan-webgl'),fallback=$('sedan-model');
 if(!data||!stage||!fallback)return;
 const status=$('sedan-renderer-status');
 let renderer;
 try{renderer=new T.WebGLRenderer({antialias:true,alpha:true,powerPreference:'high-performance'});}
 catch{status.textContent='Vector fallback · WebGL is unavailable.';return;}
 try{
 const {components,links,colors,kindColor}=data;
 const scene=new T.Scene(),camera=new T.PerspectiveCamera(36,1,.1,60);
 renderer.setPixelRatio(Math.min(devicePixelRatio||1,2));
 renderer.outputColorSpace=T.SRGBColorSpace;
 renderer.toneMapping=T.ACESFilmicToneMapping;renderer.toneMappingExposure=1.25;
 renderer.domElement.setAttribute('aria-label','Drag to rotate the three-dimensional sedan');
 renderer.domElement.tabIndex=0;
 stage.append(renderer.domElement);
 const labels=document.createElement('div');labels.className='sedan-label-plane';stage.append(labels);
 const caption=document.createElement('div');caption.className='sedan-render-caption';caption.textContent='ZARC / VEHICLE ELECTRONICS STUDY · CONCEPT ONLY';stage.append(caption);
 const controls=new OrbitControls(camera,renderer.domElement);controls.enableDamping=true;controls.dampingFactor=.09;
 controls.minDistance=2.4;controls.maxDistance=13;controls.maxPolarAngle=Math.PI*.49;controls.minPolarAngle=.08;controls.enablePan=true;
 const hemi=new T.HemisphereLight(0xe3f4ff,0x37404a,2.5);scene.add(hemi);
 for(const [color,intensity,pos] of [[0xffffff,4,[3,7,4]],[0xb1defe,2,[-4,3,-4]],[0xffdfc2,1.2,[-1,4,5]]]){
  const light=new T.DirectionalLight(color,intensity);light.position.set(...pos);scene.add(light);
 }
 // Local procedural studio environment; no third-party vehicle asset.
 const env=new T.Scene();env.background=new T.Color(0x4e5a62);
 for(const [x,y,z,w,h] of [[0,4,0,9,4],[-4,1,2,3,5],[4,1,-2,4,3]]){
  const panel=new T.Mesh(new T.PlaneGeometry(w,h),new T.MeshBasicMaterial({color:0xeaf4ff,side:T.DoubleSide}));panel.position.set(x,y,z);panel.lookAt(0,0,0);env.add(panel);
 }
 const pmrem=new T.PMREMGenerator(renderer),environment=pmrem.fromScene(env,.05);
 scene.environment=environment.texture;scene.environmentIntensity=.8;pmrem.dispose();
 const car=new T.Group(),interior=new T.Group(),chassis=new T.Group(),skin=new T.Group(),electronics=new T.Group(),network=new T.Group();
 scene.add(car);car.add(chassis,interior,skin,electronics,network);
 const mat=(c,m=.15,r=.5,more={})=>new T.MeshStandardMaterial({color:c,metalness:m,roughness:r,...more});
 const silver=mat(0x9caab2,.78,.26),chrome=mat(0xb9c8d1,.9,.2),black=mat(0x182126,.18,.69),rubber=mat(0x15191d,0,.9);
 const leather=mat(0x899299,.05,.65),leatherDark=mat(0x5b666d,.05,.7),floor=mat(0x303b43,.35,.5),screen=mat(0x0c2837,.25,.25,{emissive:0x12566c,emissiveIntensity:.5});
 const glass=mat(0x86b3c5,.18,.25,{transparent:true,opacity:.13,depthWrite:false,side:T.DoubleSide});
 const body=mat(0x95abb7,.78,.28,{transparent:true,opacity:.18,depthWrite:false,side:T.DoubleSide});
 const bodySurfaces=[];
 const v=p=>new T.Vector3(...p);
 function mesh(geometry,material,parent,x=0,y=0,z=0){
  const m=new T.Mesh(geometry,material);m.position.set(x,y,z);parent.add(m);return m;
 }
 function block(parent,p,size,material,bevel=.025){
  const [l,h,w]=size,r=Math.min(bevel,l*.2,h*.2,w*.2);
  if(r<=.001)return mesh(new T.BoxGeometry(l,h,w),material,parent,...p);
  const s=new T.Shape();s.moveTo(-l/2+r,-h/2);s.lineTo(l/2-r,-h/2);s.quadraticCurveTo(l/2,-h/2,l/2,-h/2+r);s.lineTo(l/2,h/2-r);s.quadraticCurveTo(l/2,h/2,l/2-r,h/2);s.lineTo(-l/2+r,h/2);s.quadraticCurveTo(-l/2,h/2,-l/2,h/2-r);s.lineTo(-l/2,-h/2+r);s.quadraticCurveTo(-l/2,-h/2,-l/2+r,-h/2);
  const g=new T.ExtrudeGeometry(s,{depth:w-2*r,bevelEnabled:true,bevelThickness:r*.7,bevelSize:r*.45,bevelSegments:2,steps:1,curveSegments:5});g.translate(0,0,-(w-2*r)/2);g.computeVertexNormals();return mesh(g,material,parent,...p);
 }
 function pipe(parent,pts,r,material){
  const curve=new T.CatmullRomCurve3(pts.map(v),false,'centripetal');
  return mesh(new T.TubeGeometry(curve,Math.max(12,pts.length*6),r,7,false),material,parent);
 }
 function surface(parent,points,material,side=0){
  const g=new T.BufferGeometry(),arr=[];for(let i=1;i<points.length-1;i++)arr.push(...points[0],...points[i],...points[i+1]);
  g.setAttribute('position',new T.Float32BufferAttribute(arr,3));g.computeVertexNormals();const m=mesh(g,material,parent);if(parent===skin)bodySurfaces.push({m,side});return m;
 }
 function axleCylinder(parent,p,r,width,material){
  const m=mesh(new T.CylinderGeometry(r,r,width,48),material,parent,...p);m.rotation.x=Math.PI/2;return m;
 }
 // Classic four-door proportions, pronounced hood and separate rear deck.
 const sections=[[-2.48,.68,.31,.61],[-2.31,.88,.23,.69],[-2.03,.97,.23,.76],[-1.47,.99,.22,.80],[-1.05,1,.22,.81],[0,1,.22,.83],[.65,1,.22,.81],[1.18,.99,.23,.78],[1.78,.97,.23,.72],[2.23,.90,.25,.61],[2.45,.71,.33,.51]];
 for(let i=0;i<sections.length-1;i++){
  const [x,w,lo,hi]=sections[i],[x2,w2,lo2,hi2]=sections[i+1];
  surface(skin,[[x,hi,-w*.82],[x2,hi2,-w2*.82],[x2,hi2,w2*.82],[x,hi,w*.82]],body);
  for(const s of [-1,1]){
   surface(skin,[[x,hi,s*w*.82],[x2,hi2,s*w2*.82],[x2,hi2-.15,s*w2],[x,hi-.15,s*w]],body,s);
   // Lower side panel interrupted by wheel openings.
   if(Math.abs((x+x2)/2-1.53)>.5&&Math.abs((x+x2)/2+1.53)>.5)
    surface(skin,[[x,hi-.15,s*w],[x2,hi2-.15,s*w2],[x2,lo2,s*w2*.86],[x,lo,s*w*.86]],body,s);
  }
 }
 surface(skin,[[-2.48,.31,-.68],[-2.48,.31,.68],[-2.48,.61,.68],[-2.48,.61,-.68]],body);
 surface(skin,[[2.45,.33,-.71],[2.45,.33,.71],[2.45,.51,.71],[2.45,.51,-.71]],body);
 const roof=[[-1.43,.80,.81],[-.94,.70,1.30],[-.69,.67,1.42],[.22,.68,1.43],[.54,.72,1.33],[1.01,.82,.80]];
 for(let i=0;i<roof.length-1;i++){
  const [x,w,y]=roof[i],[x2,w2,y2]=roof[i+1];
  surface(skin,[[x,y,-w],[x2,y2,-w2],[x2,y2,w2],[x,y,w]],i===2?body:glass);
  for(const s of [-1,1])surface(skin,[[x,.80,s*.84],[x,y,s*w],[x2,y2,s*w2],[x2,.80,s*.84]],glass,s);
 }
 for(const z of [-1,1]){
  pipe(chassis,roof.map(([x,w,y])=>[x,y,z*w]),.022,chrome);
  pipe(chassis,[[-2.23,.72,z*.91],[-1.25,.81,z*.98],[.60,.82,z*.98],[2.22,.60,z*.91]],.016,chrome);
  pipe(chassis,[[-.44,.81,z*.86],[-.44,1.43,z*.68]],.033,silver);
  block(chassis,[0,.27,z*.85],[3.45,.15,.10],silver);
  for(const x of [-1.53,1.53]){
   const pts=[];for(let i=0;i<=20;i++){const a=i/20*Math.PI;pts.push([x+Math.cos(a)*.465,.37+Math.sin(a)*.465,z*.975]);}
   pipe(chassis,pts,.014,chrome);
  }
  for(const x of [-.92,.42])block(chassis,[x,.72,z*.98],[.16,.035,.025],chrome,.006);
 }
 // Undercarriage and mechanical assemblies.
 block(chassis,[0,.19,0],[4.23,.12,1.64],floor);
 for(const x of [-1.53,1.53])block(chassis,[x,.35,0],[.18,.14,1.65],silver);
 for(const x of [-1.53,1.53])for(const s of [-1,1]){
  const z=s*1.01;
  axleCylinder(chassis,[x,.37,z],.40,.23,rubber);
  const tire=mesh(new T.TorusGeometry(.337,.062,12,64),rubber,chassis,x,.37,z+s*.113);
  axleCylinder(chassis,[x,.37,z+s*.123],.283,.024,chrome);
  axleCylinder(chassis,[x,.37,z+s*.14],.238,.012,black);
  axleCylinder(chassis,[x,.37,z+s*.15],.074,.03,chrome);
  for(let i=0;i<10;i++){
   const a=i*Math.PI/5,spoke=block(chassis,[x+Math.cos(a)*.151,.37+Math.sin(a)*.151,z+s*.157],[.235,.036,.021],silver,.005);
   spoke.rotation.z=a;
  }
 }
 for(let i=0;i<7;i++)for(const z of [-.33,.33])block(chassis,[-1.02+i*.31,.27,z],[.29,.14,.61],mat(0x465c68,.42,.45),.012);
 block(chassis,[1.88,.34,0],[.52,.30,.72],silver);
 for(let i=0;i<9;i++)block(chassis,[1.66+i*.052,.51,0],[.024,.052,.65],chrome,.004);
 for(const s of [-1,1]){
  const lamp=mat(0xe4f6ff,.25,.2,{emissive:0xb6e6ff,emissiveIntensity:.7});
  block(chassis,[2.30,.59,s*.58],[.22,.074,.35],lamp,.016);
  block(chassis,[-2.36,.65,s*.59],[.16,.065,.33],mat(0xb12931,.25,.35,{emissive:0xb52932,emissiveIntensity:.5}),.012);
  block(chassis,[.66,.84,s*1.03],[.19,.12,.20],silver);
 }
 // Sculpted cabin: four seating positions, center console and cockpit.
 for(const z of [-.43,.43]){
  const seat=new T.Group();seat.position.set(.12,.48,z);interior.add(seat);
  block(seat,[0,.10,0],[.60,.18,.55],leather,.055);
  const back=block(seat,[-.26,.36,0],[.20,.59,.54],leather,.06);back.rotation.z=.16;
  block(seat,[-.31,.69,0],[.15,.19,.32],leather,.045);
  for(const side of [-1,1])block(seat,[.07,.17,side*.23],[.45,.11,.10],leatherDark,.035);
  for(const side of [-1,1])block(seat,[0,-.06,side*.17],[.65,.045,.055],chrome);
 }
 block(interior,[-1.16,.58,0],[.55,.19,1.42],leather,.05);
 const rear=block(interior,[-1.42,.83,0],[.18,.51,1.39],leather,.055);rear.rotation.z=.13;
 for(const z of [-.47,0,.47])block(interior,[-1.46,1.13,z],[.14,.17,.27],leather,.04);
 block(interior,[.04,.60,0],[1.03,.25,.27],black,.04);
 block(interior,[.76,.91,0],[.38,.24,1.53],black,.05);
 pipe(interior,[[.66,.93,-.73],[.65,.93,-.26],[.65,.93,.26],[.66,.93,.73]],.012,chrome);
 const display=block(interior,[.55,1.04,.03],[.045,.27,.51],screen,.015);display.rotation.z=-.08;
 block(interior,[.54,1.02,-.49],[.045,.18,.36],screen,.012);
 const steering=mesh(new T.TorusGeometry(.165,.022,12,48),black,interior,.47,.95,-.44);steering.rotation.y=Math.PI/2;
 block(interior,[.47,.95,-.44],[.037,.054,.30],chrome,.008);block(interior,[.47,.88,-.44],[.036,.15,.045],chrome,.008);
 block(interior,[.47,.95,-.44],[.052,.10,.12],black,.015);
 // Each controller is a pickable object, with differentiated finned enclosures.
 const nodes=new Map(),pickables=[],labelMap=new Map(),baseMats=new Map();
 for(const c of components){
  const g=new T.Group();g.position.set(...c.pos);g.userData.id=c.id;electronics.add(g);nodes.set(c.id,g);
  const color=new T.Color(kindColor[c.kind]),isZ=c.kind==='zarc',isCompute=c.id==='host'||c.id==='adas',isController=isZ||isCompute||c.kind==='legacy'||c.kind==='host';
  if(c.id==='cloud'){
   const cloudMat=mat(0x77a7ba,.25,.42,{transparent:true,opacity:.65});
   for(const [x,y,z,r] of [[-.13,0,0,.13],[0,.08,0,.18],[.16,0,0,.13]])mesh(new T.SphereGeometry(r,20,12),cloudMat,g,x,y,z);
  }else if(isController){
   const l=isCompute?.40:isZ?.26:.22,w=isCompute?.31:isZ?.20:.17,h=isCompute?.16:isZ?.105:.09;
   block(g,[0,0,0],[l,h,w],silver,.012);
   const accent=mat(color,.3,.32,{emissive:color,emissiveIntensity:.48});baseMats.set(c.id,accent);
   block(g,[0,-h*.52,0],[l*1.02,.018,w*1.02],accent,.003);
   for(let i=0;i<6;i++)block(g,[-l*.40+i*l*.16,h*.65,0],[l*.065,h*.35,w*.88],chrome,.003);
   for(const s of [-1,1])block(g,[s*l*.49,0,0],[.035,h*.55,w*.68],black,.003);
  }else if(c.kind==='sensor'){
   const sensor=block(g,[0,0,0],[.10,.085,.095],silver,.009);
   mesh(new T.SphereGeometry(.038,16,10),screen,g,.047,.004,0);
  }else block(g,[0,0,0],[.13,.075,.11],mat(color,.4,.4),.008);
  const hit=mesh(new T.SphereGeometry(isController?.15:.092,12,8),new T.MeshBasicMaterial({visible:false}),g);
  hit.userData.id=c.id;pickables.push(hit);
  const button=document.createElement('button');button.type='button';button.className='sedan-pin';button.textContent=c.name.replace('ZARC ','');button.title=c.name;button.addEventListener('click',()=>data.select(c.id));labels.append(button);labelMap.set(c.id,button);
 }
 const lineObjects=[];
 for(const [a,b,kind] of links){
  if(!nodes.has(a)||!nodes.has(b))throw Error('Invalid vehicle connection');
  const material=new T.LineBasicMaterial({color:colors[kind],transparent:true,opacity:.62,depthTest:false});
  const geometry=new T.BufferGeometry();const line=new T.Line(geometry,material);line.renderOrder=7;network.add(line);
  const dot=mesh(new T.SphereGeometry(.018,10,8),new T.MeshBasicMaterial({color:colors[kind],depthTest:false}),network);dot.renderOrder=8;
  lineObjects.push({a,b,kind,line,dot,points:[]});
 }
 // Low-voltage schematic harnesses along sills, separate from logical overlays.
 const harness=new T.Group();chassis.add(harness);
 for(const side of [-1,1])for(let i=0;i<3;i++)pipe(harness,[[-2.03,.27,side*.68+i*.025],[-1.50,.26,side*.68+i*.025],[0,.26,side*.68+i*.025],[1.48,.26,side*.68+i*.025],[2.02,.40,side*.62+i*.025]],.012,mat(i===2?0x38b8cb:0xb26b48,.3,.5));
 const ground=mesh(new T.PlaneGeometry(200,200),mat(0x1c2a32,0,.92),scene,0,-.047,0);ground.rotation.x=-Math.PI/2;
 const grid=new T.GridHelper(14,28,0x526773,0x354650);grid.position.y=-.035;grid.material.transparent=true;grid.material.opacity=.24;scene.add(grid);
 let selected='host',exploded=false,only=false,showLabels=true,cut=true,bodyOpacity=.18,active=true,frameId=0,frameTime=0;
 const enabled=new Set(Object.keys(colors)),tmp=new T.Vector3(),ray=new T.Raycaster(),pointer=new T.Vector2();
 function route(a,b,kind){
  const p=nodes.get(a).position,q=nodes.get(b).position;
  return kind==='Ethernet'||kind==='CAN'||kind==='LIN'?[p.clone(),new T.Vector3(p.x,.20,p.z),new T.Vector3(q.x,.20,p.z),new T.Vector3(q.x,.20,q.z),q.clone()]:[p.clone(),q.clone()];
 }
 function refresh(){
  for(const c of components){
   const g=nodes.get(c.id);g.position.set(...c.pos);
   if(exploded&&c.kind!=='sensor'&&c.id!=='cloud')g.position.y+=c.kind==='zarc'?.70:.42;
   g.visible=!only||c.kind==='zarc'||c.id===selected;
   if(baseMats.has(c.id))baseMats.get(c.id).emissiveIntensity=c.id===selected?1.4:.38;
   labelMap.get(c.id).classList.toggle('active',c.id===selected);
  }
  for(const entry of lineObjects){
   entry.points=route(entry.a,entry.b,entry.kind);
   entry.line.geometry.dispose();entry.line.geometry=new T.BufferGeometry().setFromPoints(entry.points);
   const lit=entry.a===selected||entry.b===selected;
   entry.line.material.opacity=lit?1:.38;
   entry.line.visible=enabled.has(entry.kind)&&(!only||lit||nodes.get(entry.a).visible&&nodes.get(entry.b).visible);
   entry.dot.visible=entry.line.visible&&lit;
  }
 }
 function view(position,target=[0,.57,0]){
  camera.position.set(...position);controls.target.set(...target);controls.update();
  $('sedan-zoom').value='100';lastZoom=1;
 }
 let lastZoom=1;
 const initial=[4.5,3.0,5.8];
 function orbit(delta){const p=camera.position.clone().sub(controls.target);p.applyAxisAngle(new T.Vector3(0,1,0),delta);camera.position.copy(controls.target).add(p);controls.update();}
 $('sedan-left').onclick=()=>orbit(-.25);$('sedan-right').onclick=()=>orbit(.25);
 $('sedan-top').onclick=()=>view([0,8.4,.02],[0,.3,0]);
 $('sedan-side').onclick=()=>view([.1,1.8,7.2]);
 $('sedan-cabin').onclick=()=>view([2.1,2.0,3.2],[0,.74,0]);
 $('sedan-reset').onclick=()=>{
  exploded=only=false;cut=showLabels=true;bodyOpacity=.18;interior.visible=true;
  $('sedan-opacity').value=18;$('sedan-interior').checked=$('sedan-cutaway').checked=$('sedan-labels').checked=true;
  $('sedan-exploded').checked=$('sedan-only').checked=false;view(initial);refresh();
 };
 $('sedan-zoom').oninput=e=>{const n=Number(e.target.value)/100;const offset=camera.position.clone().sub(controls.target).multiplyScalar(lastZoom/n);camera.position.copy(controls.target).add(offset);lastZoom=n;controls.update();};
 $('sedan-opacity').oninput=e=>{bodyOpacity=Number(e.target.value)/100;};
 $('sedan-interior').onchange=e=>{interior.visible=e.target.checked;};
 $('sedan-cutaway').onchange=e=>{cut=e.target.checked;};
 $('sedan-labels').onchange=e=>{showLabels=e.target.checked;};
 $('sedan-exploded').onchange=e=>{exploded=e.target.checked;refresh();};
 $('sedan-only').onchange=e=>{only=e.target.checked;refresh();};
 const checks=$('sedan-layers').querySelectorAll('input');Object.keys(colors).forEach((k,i)=>checks[i].addEventListener('change',()=>{if(checks[i].checked)enabled.add(k);else enabled.delete(k);refresh();}));
 let down;
 renderer.domElement.addEventListener('pointerdown',e=>{down=[e.clientX,e.clientY];});
 renderer.domElement.addEventListener('pointerup',e=>{
  if(!down||Math.hypot(e.clientX-down[0],e.clientY-down[1])>5)return;
  const r=renderer.domElement.getBoundingClientRect();pointer.set((e.clientX-r.left)/r.width*2-1,-(e.clientY-r.top)/r.height*2+1);ray.setFromCamera(pointer,camera);
  const hits=ray.intersectObjects(pickables.filter(p=>p.parent.visible));if(hits.length)data.select(hits[0].object.userData.id);
 });
 function resize(){const w=stage.clientWidth,h=stage.clientHeight;if(!w||!h)return;renderer.setSize(w,h,false);camera.aspect=w/h;camera.updateProjectionMatrix();}
 const resizeObserver=new ResizeObserver(resize);resizeObserver.observe(stage);
 globalThis.zarcSedan3D={select(id){selected=id;refresh();}};
 function draw(time){
  if(!active)return;frameId=requestAnimationFrame(draw);controls.update();
  body.opacity=bodyOpacity;glass.opacity=bodyOpacity*.62;
  for(const {m,side} of bodySurfaces)m.visible=!(cut&&side&&side*camera.position.z>0);
  harness.visible=enabled.has('CAN')||enabled.has('Ethernet');
  const dt=Math.min((time-frameTime)/1000,.1);frameTime=time;
  for(const e of lineObjects)if(e.dot.visible&&e.points.length>1){
   const phase=(time*.00035)%1*(e.points.length-1),i=Math.min(e.points.length-2,Math.floor(phase));e.dot.position.copy(e.points[i]).lerp(e.points[i+1],phase-i);
  }
  for(const c of components){
   const b=labelMap.get(c.id),g=nodes.get(c.id);
   const isSelected=c.id===selected;
   const visible=g.visible&&(isSelected||showLabels&&c.kind==='zarc');
   if(!visible){b.hidden=true;continue;}
   tmp.copy(g.position);tmp.y+=.17;tmp.project(camera);b.hidden=tmp.z< -1||tmp.z>1||Math.abs(tmp.x)>.98||Math.abs(tmp.y)>.95;
   b.style.left=(tmp.x*.5+.5)*100+'%';b.style.top=(-tmp.y*.5+.5)*100+'%';
  }
  renderer.render(scene,camera);
 }
 renderer.domElement.addEventListener('webglcontextlost',e=>{
  e.preventDefault();active=false;cancelAnimationFrame(frameId);resizeObserver.disconnect();
  // Preserve access to the static topology rather than leave a blank panel.
  stage.hidden=true;fallback.hidden=false;status.textContent='WebGL context lost · reload to restore interactive controls; vector view remains available.';
 });
 stage.hidden=false;fallback.hidden=true;$('sedan-exploded-control').hidden=false;$('sedan-only-control').hidden=false;
 status.textContent='Shaded WebGL model · 44 reference components · selectable electronics and network layers';
 view(initial);resize();refresh();frameId=requestAnimationFrame(draw);
 }catch(error){
  console.error('ZARC sedan WebGL initialization:',error);renderer.dispose();stage.hidden=true;fallback.hidden=false;status.textContent='Vector fallback · shaded model could not initialize.';
 }
}
if(document.readyState==='loading')document.addEventListener('DOMContentLoaded',launch,{once:true});else launch();
