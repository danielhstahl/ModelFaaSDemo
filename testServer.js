const WebSocket = require('ws');
const testJson=require('./test.json')
/*const express = require('express')
const bodyParser=require('body-parser')
const app = express()
app.use(bodyParser.json())*/
const numSend=100
const sendPer=1000
const numLoans=numSend*sendPer;
const minLoanSize=10000
const maxLoanSize=50000
const maxP=.09
const minP=.0001
const roughTotalExposure=(minLoanSize, maxLoanSize, numLoans)=>numLoans*(minLoanSize+.5*(maxLoanSize-minLoanSize))
const roughXMin=(exposure, bL, maxP, tau)=>-exposure*bL*maxP*.5*5*tau
console.log(roughXMin(roughTotalExposure(minLoanSize, maxLoanSize, numLoans), testJson.params.bL, maxP, testJson.params.tau))

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
            l:generateRandom(minLoanSize, maxLoanSize),
            pd:generateRandom(minP, maxP)
        })
    }
    return loans
}

const convertObjToBuffer=obj=>new Buffer.from(JSON.stringify(obj))
const wss = new WebSocket.Server({ port: 3000 });
wss.on('connection', ws=>{
    console.log("Connected")
    ws.on('message', message=>{
        
        for(i=0; i<numSend;++i){
            ws.send(convertObjToBuffer(generateFakeLoanData(sendPer, testJson.params.alpha.length)))
        }
        //
        setTimeout(()=>{
            ws.send("terminate")
        }, 100000)//100 seconds*/
    });
    
    //ws.send('something');
});