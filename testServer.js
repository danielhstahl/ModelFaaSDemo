const WebSocket = require('ws');
const testJson=require('./test.json')
const numSend=1000
const sendPer=1000
const numLoans=numSend*sendPer;
const minLoanSize=10000
const maxLoanSize=50000
const maxP=.09
const minP=.0001
const maxPossibleLoss=.14//more than this an we are in big trouble...
const roughTotalExposure=(minLoanSize, maxLoanSize, numLoans)=>numLoans*(minLoanSize+.5*(maxLoanSize-minLoanSize))
//const roughXMin=(exposure, bL, maxP, tau)=>-exposure*bL*maxP*.5*5*tau

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
    console.time("doEC");
    console.log(maxPossibleLoss*roughTotalExposure(minLoanSize, maxLoanSize, numLoans))
    ws.on('message', message=>{
        console.log(message)
        switch(message){
            case 'init':{
                ws.send(convertObjToBuffer({numLoans, xMin:-maxPossibleLoss*roughTotalExposure(minLoanSize, maxLoanSize, numLoans), numSend, xMax:0}))
                break
            }
            case 'getloans':{
                for(i=0; i<numSend;++i){
                    ws.send(convertObjToBuffer(generateFakeLoanData(sendPer, testJson.params.alpha.length)))
                }
                ws.send("done")
                break
            }
            default: {
                console.log("incorrect message")
            }
        }
        
    });
    ws.on('close', ()=>{
        console.timeEnd("doEC");
        console.log("connection closed")
    })
});