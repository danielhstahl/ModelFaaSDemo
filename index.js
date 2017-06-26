'use strict';  
const WebSocket = require('ws');
// Requiring the add-on that we'll build in a moment...
const addon = require('./build/Release/buffer_example');
const testJson=require('./test.json')
// Allocates memory holding ASCII "ABC" outside of V8.
//const buffer = Buffer.from("ABC");

// synchronous, rotates each character by +13
const myObj=new addon.LoanCF(Buffer.from(JSON.stringify(testJson), 'utf8'))
const ws = new WebSocket('ws://localhost:3000');
ws.on('open', ()=>{
    ws.send('retrieveData')
    let numMessages=0
    ws.on('message', (res)=>{
        myObj.supplyCF(res)
        numMessages++
        console.log(`got through ${numMessages}`)
    })
    ws.on('close', function close() {
        console.log("closed")//myObj.computeDensity())
    })
    
})
//myObj.supplyCF()
//console.log(buffer.toString('ascii'));  