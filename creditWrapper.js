/**This is a wrapper for the c++...ie a "client" for testServer */
const testJson=require('./test.json')
const fs=require('fs')
const {spawn, exec}= require('child_process');
//console.log(JSON.stringify(testJson))
const model=spawn("./httpCreditRisk", [JSON.stringify(testJson)]);
let modelOutput
let modelErr
const logStream = fs.createWriteStream('./results.json', {'flags': 'a'});
model.stdout.on('data', (data)=>{
    console.log(""+data)
    logStream.write(data)
});
model.stderr.on('data', (data)=>{
    console.log("error!")
    console.log(""+data)
});
model.on('close', (code)=>{
    console.log("closed")
});