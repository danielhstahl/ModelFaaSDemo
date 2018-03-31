#define _USE_MATH_DEFINES
#include "easywsclient.hpp"
#include "easywsclient.cpp"
#include <iostream>
#include "CreditUtilities.h"
#include "CFDistUtilities.h"
#include "document.h" //rapidjson
#include "writer.h" //rapidjson
#include "stringbuffer.h" //rapidjson
#include "schema.h"//rapidjson
#include "error/en.h" //rapidjson
#include "FangOost.h"
#include "FunctionalUtilities.h"
#include "Vasicek.h"  
#include "CheckSchema.h" //handleSchema
#include <vector>
#include <complex>
#include <memory>
#include <assert.h>
#include <string>

/**this retrieves data in text file*/
const char *inputSchema =
#include "inputschema.json"
;
const char *serverSchema =
#include "serverschema.json"
;
const char *metaSchema =
#include "metaschema.json"
;

//const double maxPercentLoss=.14;//if more than this, in big trouble

//std::mutex mtx;    
bool ensureEnoughArgs(int argc){
    return argc>1?true:false;
}
 
auto getCF(int xSteps, int m, double xMin, double xMax, double tau, double lambda, double q, double alphL, double bL, double sigL){
    return creditutilities::getFullCFFn(xMin, xMax,
        creditutilities::getLiquidityRiskFn(lambda, q),//u->complex
        creditutilities::logLPMCF( 
            m,
            creditutilities::getLGDCFFn(alphL, bL, sigL, tau, bL),
            [](const rapidjson::Value& loan){return loan["pd"].GetDouble();},
            [](const rapidjson::Value& loan){return loan["l"].GetDouble();},
            [](const rapidjson::Value& loan, const int& index){return loan["w"][index].GetDouble();}
        )//u, loans->v, n
    );
    //return cfDist;
}

template<typename ExpectationArray, typename VarianceArray, typename CFVector>
auto getES(
    double alpha, 
    const ExpectationArray& expectation, 
    const VarianceArray& variance,
    const CFVector& cf, 
    int uSteps, 
    double xMin, 
    double xMax
){
    auto vasicekLogFN=vasicek::getLogVasicekMFGFn(expectation, variance);
    double prec=.0000000001;
    return cfdistutilities::computeESDiscrete(alpha, prec, xMin, xMax, fangoost::convertLogCFToRealExp(xMin,xMax, futilities::for_each_parallel(0, uSteps, [&](const auto& index){
        return vasicekLogFN(cf[index]);
    })));
}
template<typename CFVector, typename CFFn, typename WSp, typename ExpectationArray, typename VarianceArray >
int handleLoans(
    const std::string& msg,  
    const CFFn& fn, 
    CFVector* cf, 
    const ExpectationArray& expectation,
    const VarianceArray& variance,
    int numSend, 
    int increment, 
    int uSteps, 
    double xMin, 
    double xMax, 
    WSp ws
){
    rapidjson::Document loans;
    if(!handleSchema(serverSchema, msg.c_str(), loans, true)){
        std::cout<<"Handle Loans Crash!!!"<<std::endl;
    }
    *cf=fn(
        std::move(*cf), 
        loans.GetArray()
    );
    /**if done sending*/
    if(numSend==increment){
        auto ES=getES(.01, expectation, variance, *cf, uSteps, xMin, xMax);
        std::cout<<"{\"ES\":"<<std::get<cfdistutilities::ES>(ES)<<"}"<<std::endl;
        std::cout<<"{\"VaR\":"<<std::get<cfdistutilities::VAR>(ES)<<"}"<<std::endl;
        ws->close();
    }
    return increment+1;
}
auto handleMetaLoans(const std::string& msg, int* numSend, double* xMin, double* xMax,int m, int xSteps, double tau, double lambda, double q, double alphL, double bL, double sigL){
    rapidjson::Document metadata;
    if(!handleSchema(metaSchema, msg.c_str(), metadata, true)){
        std::cout<<"Meta Loans Crash!!!"<<std::endl;
    }
    *numSend=metadata["numSend"].GetInt();
    *xMin=metadata["xMin"].GetDouble();
    *xMax=metadata["xMax"].GetDouble();
    return getCF(xSteps, m, *xMin, *xMax, tau, lambda, q, alphL, bL, sigL);
}
int main(int argc, char* argv[]){
    using easywsclient::WebSocket;
    if(!ensureEnoughArgs(argc)){
        std::cout<<"not enough args"<<std::endl;
        return 0;
    }
    rapidjson::Document parms;
    /**Note!  handleSchema modifies parms*/
    if(!handleSchema(inputSchema, argv[1], parms)){
        return 0;
    }
    /**set some default paramters and helper functions*/
    int numSend=100000;//something rediculously large
    const auto retrieveJSONValue=[](const auto& val){
        return val.GetDouble();
    };
    int increment=1;
    double xMin;
    double xMax;
    /**get parameters*/
    auto params=parms["params"].GetObject();
    const auto y0=params["y0"].GetArray();
    const auto sigma=params["sigma"].GetArray();
    const auto alpha=params["alpha"].GetArray();
    const auto rho=params["rho"].GetArray();
    const int uSteps=params["uSteps"].GetInt();
    const int m=alpha.Size();
    const int xSteps=params["xSteps"].GetInt();
    const double tau=params["tau"].GetDouble();
    const double lambda=params["lambda"].GetDouble();
    const double q=params["q"].GetDouble();
    const double alphL=params["alphL"].GetDouble();
    const double bL=params["bL"].GetDouble();
    const double sigL=params["sigL"].GetDouble();

    /**this will hold the vectorized CF*/
    auto cf=std::vector<std::vector<std::complex<double> > >(uSteps, std::vector<std::complex<double> >(m, 0));
    /**expectation, used for the vasicek MGF*/
    const auto expectation=vasicek::computeIntegralExpectationLongRunOne(y0, alpha, m, tau, retrieveJSONValue);
    /**variance, used for the vasicek MGF*/ 
    const auto variance=vasicek::computeIntegralVarianceVasicek(alpha, sigma, rho, m, tau, retrieveJSONValue);

    std::string url=parms["url"].GetString();
    std::unique_ptr<WebSocket> ws1(WebSocket::from_url(url));
    std::unique_ptr<WebSocket> ws2(WebSocket::from_url(url));
    //assert(ws);


    //initiate connection.  Will start receiving metadata
    ws1->send("init");
    ws1->poll(); //send message
    while (ws1->getReadyState() != WebSocket::CLOSED) {
        ws1->poll();
        ws1->dispatch([&](const std::string& message) {
            auto fn=handleMetaLoans(message, &numSend, &xMin, &xMax, m, xSteps, tau, lambda, q, alphL, bL, sigL);
            ws2->send("getloans");
            ws2->poll();//send message
            /*std::cout<<"xMin: "<<xMin<<std::endl;
            std::cout<<"xMax: "<<xMax<<std::endl;
            std::cout<<"numSend: "<<numSend<<std::endl;*/
            while (ws2->getReadyState() != WebSocket::CLOSED) {
                WebSocket::pointer wsp = &*ws2;
                //WebSocket::pointer wsp = &*ws; // <-- because a unique_ptr cannot be copied into a lambda
                ws2->poll(-1);//wait for next message
                ws2->dispatch([&, wsp](const std::string& message) {
                    //std::cout<<increment<<std::endl;
                    increment=handleLoans(message, fn, &cf, expectation, variance, numSend, increment, uSteps, xMin, xMax, wsp);
                });
            }
            ws1->close();
        });
    }
    //delete ws;
    return 0;
    
}


