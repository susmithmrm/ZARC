// Run the actual renderer using a minimal DOM contract; not a pixel/browser test.
const fs=require("node:fs"),path=require("node:path"),vm=require("node:vm"),assert=require("node:assert/strict");
class Element{
 constructor(tag){this.tagName=tag;this.children=[];this.attributes={};this.style={};this.dataset={};this.events={};}
 setAttribute(k,v){this.attributes[k]=v;}
 append(...items){this.children.push(...items);}
 replaceChildren(...items){this.children=items;}
 addEventListener(k,fn){this.events[k]=fn;}
 querySelectorAll(tag){return this.children.filter(x=>x.tagName===tag);}
}
const ids=["sedan-model","sedan-components","sedan-detail","sedan-layers","sedan-left","sedan-right","sedan-top","sedan-side","sedan-cabin","sedan-reset","sedan-opacity","sedan-zoom","sedan-interior","sedan-cutaway","sedan-labels"];
const elements=Object.fromEntries(ids.map(id=>[id,new Element(id)]));
const document={getElementById:id=>elements[id],createElement:tag=>new Element(tag),createElementNS:(_,tag)=>new Element(tag)};
vm.runInNewContext(fs.readFileSync(path.join(__dirname,"..","site","sedan.js"),"utf8"),{document});
assert.equal(elements["sedan-components"].children.length,44);
assert.equal(elements["sedan-layers"].children.length,8);
const model=elements["sedan-model"];
assert.ok(model.children.filter(e=>e.tagName==="polygon").length>300);
for(const id of ["sedan-left","sedan-right","sedan-top","sedan-side","sedan-cabin","sedan-reset"])elements[id].onclick();
elements["sedan-zoom"].oninput({target:{value:"160"}});
elements["sedan-cutaway"].onchange({target:{checked:false}});
elements["sedan-interior"].onchange({target:{checked:false}});
elements["sedan-labels"].onchange({target:{checked:false}});
elements["sedan-components"].children.find(e=>e.dataset.id==="ioc").events.click();
assert.equal(elements["sedan-detail"].children[0].textContent,"ZARC companion IOC");
for(const node of model.children)for(const value of Object.values(node.attributes))assert.ok(!/NaN|Infinity/.test(value));
console.log("Sedan geometry and control contract passed");
