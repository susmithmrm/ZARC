const fs=require("node:fs"),path=require("node:path"),vm=require("node:vm"),assert=require("node:assert/strict");
class Element{
 constructor(){this.children=[];this.textContent="";}
 append(...items){this.children.push(...items);}
 replaceChildren(...items){this.children=items;}
}
const status=new Element(),body=new Element();
const document={getElementById:id=>id==="support-matrix-status"?status:id==="support-matrix-rows"?body:null,
 createElement:()=>new Element()};
const record={target:"sim-edge",profile:"edge",board:"native_sim/native/64",soc:"native_sim",
 revision:"candidate",zephyr:"v4.2.0",toolchain:"host-gcc",maintainer:"ZARC project",
 manifest:"config/targets/native-sim-edge.json",status:"CANDIDATE",features:{zbus:true,can:false},limitations:["simulation only"]};
const context={document,fetch:async()=>({ok:true,json:async()=>({schema:1,scope:"Candidate records only",production_supported:[],records:[record]})})};
vm.runInNewContext(fs.readFileSync(path.join(__dirname,"..","site","support-matrix.js"),"utf8"),context);
assert.equal(context.zarcSupportMatrix.render({schema:1,scope:"Candidate records only",production_supported:[],records:[record]}),true);
assert.equal(body.children.length,1);
assert.equal(body.children[0].children[0].textContent,"sim-edge");
assert.equal(body.children[0].children[4].textContent,"CANDIDATE");
assert.equal(body.children[0].children[5].textContent,"ZARC project");
assert.equal(body.children[0].children[8].children[0].href,
 "https://github.com/susmithmrm/ZARC/blob/main/config/targets/native-sim-edge.json");
assert.equal(context.zarcSupportMatrix.render({schema:1,production_supported:["sim-edge"],records:[record]}),false);
assert.match(status.textContent,/failed validation/);
console.log("Generated target support matrix renderer passed");
