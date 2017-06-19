/**This is a wrapper for the c++...ie a "client" for testServer */
const testJson=require('./test.json')
const {spawn, exec}= require('child_process');
//console.log(JSON.stringify(testJson))
const model=spawn("./httpCreditRisk", [JSON.stringify(testJson)]);
let modelOutput
let modelErr
model.stdout.on('data', (data)=>{
    modelOutput+=data;
});
model.stderr.on('data', (data)=>{
    modelErr+=data;
});
model.on('close', (code)=>{
    if(modelErr){
        return console.log(modelErr)
    }
    return console.log(modelOutput)
});