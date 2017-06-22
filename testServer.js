const WebSocket = require('ws');
/*const express = require('express')
const bodyParser=require('body-parser')
const app = express()
app.use(bodyParser.json())*/
const generateWeights=(numWeights)=>{
    let myWeights=[]
    for(let i=0; i<numWeights; ++i){
        myWeights.push(Math.random())
    }
    const total=myWeights.reduce((cum, curr)=>cum+curr)
    return myWeights.map(val=>val/total)
}
const generateRandom=(min, max)=>{
    return Math.random()*(max-min)+min;
}
const generateFakeLoanData=(numLoans, numMacroWeight)=>{
    let loans=[]
    for(let i=0; i<numLoans;++i){
        loans.push({
            w:generateWeights(numMacroWeight),
            l:generateRandom(1000, 4000),
            pd:generateRandom(.001, .02)
        })
    }
    return loans
}
/*app.get('/hello', (req, res)=>{
    console.log("got here");
    const data=generateFakeLoanData(100, 3)
    //console.log(data)
    res.send(data)//generateFakeLoanData(1000, 3))
})
app.listen(3000,  ()=>{
    console.log('Example app listening on port 3000!')
})*/
const wss = new WebSocket.Server({ port: 3000 });
wss.on('connection', ws=>{
    console.log("Connected")
    ws.on('message', message=>{
        console.log('received: %s', message);
    });
    ws.send('something');
});