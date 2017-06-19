#define _USE_MATH_DEFINES
/*#define _WEBSOCKETPP_CPP11_THREAD_
#define _WEBSOCKETPP_CPP11_CHRONO_
#define _WEBSOCKETPP_CPP11_TYPE_TRAITS_
#define ASIO_STANDALONE
#define ASIO_HAS_STD_ARRAY
#define ASIO_HAS_STD_ATOMIC
#define ASIO_HAS_CSTDINT
#define ASIO_HAS_STD_ADDRESSOF
#define ASIO_HAS_STD_SHARED_PTR
#define ASIO_HAS_STD_TYPE_TRAITS*/
#include "httplib.h"
#include "CreditUtilities.h"
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
#include <stdio.h> //for binary data
#include <set>
//#include "easywsclient.hpp"
//#include <thread>
/*#include <asio.hpp>
#include <websocketpp/config/asio_no_tls_client.hpp>
#include <websocketpp/client.hpp>*/
#include <iostream>
extern "C" {
    extern char _binary_inputschema_json_start; //binary data
    extern char _binary_inputschema_json_end;//binary data
    extern char _binary_serverschema_json_start; //binary data
    extern char _binary_serverschema_json_end;//binary data
}


/*typedef websocketpp::client<websocketpp::config::asio_client> client;

using websocketpp::lib::placeholders::_1;
using websocketpp::lib::placeholders::_2;
using websocketpp::lib::bind;

// pull out the type of messages sent by our config
typedef websocketpp::config::asio_client::message_type::ptr message_ptr;*/


bool ensureEnoughArgs(int argc){
    return argc>1?true:false;
}
auto handleHttpConnect(const char* url, int port){
    return [url, port](const std::string& endpoint, const auto&& cb){
        httplib::Client cli(url, port);
        auto res = cli.get((std::string("/")+endpoint).c_str());
        if (res && res->status == 200) {
            cb(res->body);
        }
    };
}

std::string getInputSchema(){
    std::string inputschema;
    char*  schemaItem = &_binary_inputschema_json_start; //is this compile time?
    while ( schemaItem != &_binary_inputschema_json_end ) inputschema+=(*schemaItem++); //is this compile time?
    return inputschema;
}
std::string getServerSchema(){
    std::string serverschema;
    char*  schemaItem = &_binary_serverschema_json_start; //is this compile time?
    while ( schemaItem != &_binary_serverschema_json_end ) serverschema+=(*schemaItem++); //is this compile time?
    return serverschema;
}
/**argv contains a json object which contains the following keys:["url", "port", "endpoint", "params"].  params is an object holding parameters for the model**/
int main(int argc, char* argv[])
{
    if(!ensureEnoughArgs(argc)){
        std::cout<<"not enough args"<<std::endl;
        return 0;
    }
    /**see http://www.linuxjournal.com/content/embedding-file-executable-aka-hello-world-version-5967
    */
    std::string inputschema=getInputSchema();
    std::string serverschema=getServerSchema();



    rapidjson::Document parms;
    std::cout<<argv[1]<<std::endl;
    //std::cout<<inputschema<<std::endl;
    /**Note!  handleSchema modifies parms*/
    if(!handleSchema(inputschema.c_str(), argv[1], parms)){
        return 0;
    }
    auto getHttpFn=handleHttpConnect(parms["url"].GetString(), parms["port"].GetInt());
    std::cout<<"successfully parsed!"<<std::endl;
    //auto restConnectionFn=handleWSConnect(parms["wsconnect"].GetString());
    auto params=parms["params"].GetObject();
    //auto loans=params["loans"].GetArray();
    auto alpha=params["alpha"].GetArray();
    auto tau=params["tau"].GetDouble();
    auto xSteps=params["xSteps"].GetInt();
    auto uSteps=params["uSteps"].GetInt();
    auto xMin=params["xMin"].GetDouble();
    auto xMax=params["xMax"].GetDouble();
    auto lambda=params["lambda"].GetDouble();
    auto q=params["q"].GetDouble();
    auto alphL=params["alphL"].GetDouble();
    auto bL=params["bL"].GetDouble();
    auto sigL=params["sigL"].GetDouble();
    
    std::cout<<"successfully assigned variables"<<std::endl;
    /**prepare functions for CF inversion*/
    std::vector<std::complex<double> > cf(uSteps);
    /**function to retreive vector values from JSON*/
    auto retreiveJSONValue=[](const auto& val){
        return val.GetDouble();
    };
    /**expectation, used for the vasicek MGF*/
    const auto expectation=vasicek::computeIntegralExpectationLongRunOne(params["y0"].GetArray(), alpha, alpha.Size(), tau, retreiveJSONValue);
    std::cout<<"successfully computed expectation"<<std::endl;
    /**variance, used for the vasicek MGF*/

    const auto variance=vasicek::computeIntegralVarianceVasicek(alpha, params["sigma"].GetArray(), params["rho"].GetArray(), alpha.Size(), tau, retreiveJSONValue);
    std::cout<<"successfully computed variance"<<std::endl;
    auto curriedPD=[](const auto& loan){return loan["pd"].GetDouble();}; 
    auto curriedL=[](const auto& loan){return loan["l"].GetDouble();}; 
    auto curriedW=[](const auto& loan, const auto& index){return loan["w"][index].GetDouble();};
   // auto testResults=curriedLogLPMCF(.5, loans);
    auto curriedFullCF=creditutilities::getFullCFFn(xMin, xMax,
        vasicek::getVasicekMFGFn(expectation, variance),//v->value
        creditutilities::getLiquidityRiskFn(lambda, q),//u->complex
        creditutilities::logLPMCF(
            alpha.Size(),
            creditutilities::getLGDCFFn(alphL, bL, sigL, tau, bL), 
            curriedL, curriedPD, curriedW
        )//u, loans->v
    );

    
    getHttpFn(parms["endpoint"].GetString(), [&](const std::string& message){
        rapidjson::Document loans;
        //std::cout<<message<<std::endl;
        if(!handleSchema(serverschema.c_str(), message.c_str(), loans)){
            return 0;
        }
        cf=curriedFullCF(
            std::move(cf), 
            loans.GetArray()
        );
        std::cout<<"example CF: "<<cf[0].real()<<std::endl;
        const auto density=fangoost::computeInvDiscrete(xSteps, xMin, xMax, std::move(cf));
        /*for(auto& val:density){
            std::cout<<val<<std::endl;
        }*/
    });
    
}