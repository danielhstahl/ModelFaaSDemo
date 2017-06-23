'use strict';  
const WebSocket = require('ws');
// Requiring the add-on that we'll build in a moment...
const addon = require('./build/Release/buffer_example');
const testJson=require('./test.json')
// Allocates memory holding ASCII "ABC" outside of V8.
//const buffer = Buffer.from("ABC");

// synchronous, rotates each character by +13
const myObj=new addon.MyObject(JSON.stringify(testJson))
const ws = new WebSocket('ws://localhost:3000');
ws.on('open', ()=>{
    ws.on('message', (res)=>{
        console.log(res)
    })
})
//myObj.supplyCF()
//console.log(buffer.toString('ascii'));  