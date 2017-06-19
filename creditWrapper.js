/**This is a wrapper for the c++...ie a "client" for testServer */
const testJson=require('./test.json')
const {spawn, exec}= require('child_process');
//console.log(JSON.stringify(testJson))
const model=spawn("./httpCreditRisk", [JSON.stringify(testJson)]);
let modelOutput
let modelErr
model.stdout.on('data', (data)=>{
    console.log(""+data)
});
model.stderr.on('data', (data)=>{
    console.log("error!")
    console.log(""+data)
});
model.on('close', (code)=>{
    console.log("closed")
});