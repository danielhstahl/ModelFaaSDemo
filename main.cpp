#define _USE_MATH_DEFINES
#define _WEBSOCKETPP_CPP11_THREAD_
#define _WEBSOCKETPP_CPP11_CHRONO_
#define _WEBSOCKETPP_CPP11_TYPE_TRAITS_
#define ASIO_STANDALONE
#define ASIO_HAS_STD_ARRAY
#define ASIO_HAS_STD_ATOMIC
#define ASIO_HAS_CSTDINT
#define ASIO_HAS_STD_ADDRESSOF
#define ASIO_HAS_STD_SHARED_PTR
#define ASIO_HAS_STD_TYPE_TRAITS
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
#include <asio.hpp>
#include <websocketpp/config/asio_no_tls_client.hpp>
#include <websocketpp/client.hpp>
#include <iostream>
extern "C" {
    extern char _binary_inputschema_json_start; //binary data
    extern char _binary_inputschema_json_end;//binary data
}


typedef websocketpp::client<websocketpp::config::asio_client> client;

using websocketpp::lib::placeholders::_1;
using websocketpp::lib::placeholders::_2;
using websocketpp::lib::bind;

// pull out the type of messages sent by our config
typedef websocketpp::config::asio_client::message_type::ptr message_ptr;


bool ensureEnoughArgs(int argc){
    return argc>1?true:false;
}
auto handleHttpConnect(const char* url, int port){
    httplib::Client cli(url, port);
    return [&cli](const std::string& endpoint, const auto&& cb){
        auto res = cli.get((std::string("/")+endpoint).c_str());
        if (res && res->status == 200) {
            cb(res->body);
        }
    };
}

/*bool hasMember(const rapidjson::Document& parms, const std::string& key){
    return parms.FindMember(key)!=parms.MemberEnd();
}*/


auto handleWSConnect(const std::string& wsconnect){
    c.set_access_channels(websocketpp::log::alevel::all);
    c.clear_access_channels(websocketpp::log::alevel::frame_payload);

    // Initialize ASIO
    c.init_asio();

    // Register our message handler
    c.set_message_handler(bind(&on_message,&c,::_1,::_2));

    websocketpp::lib::error_code ec;
    client::connection_ptr con = c.get_connection(uri, ec);
    if (ec) {
        std::cout << "could not create connection because: " << ec.message() << std::endl;
        return 0;
    }

    // Note that connect here only requests a connection. No network messages are
    // exchanged until the event loop starts running in the next line.
    c.connect(con);

    // Start the ASIO io_service run loop
    // this will cause a single connection to be made to the server. c.run()
    // will exit when this connection is closed.
    c.run();
    /*return [&cli](const std::string& endpoint, const auto&& cb){
        auto res = cli.get(std::string("/")+endpoint);
        if (res && res->status == 200) {
            cb(res->body);
        }
    };*/
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
    std::string schema;
    char*  schemaItem = &_binary_inputschema_json_start; //is this compile time?
    while ( schemaItem != &_binary_inputschema_json_end ) schema+=(*schemaItem++); //is this compile time?
    rapidjson::Document parms;
    std::cout<<argv[1]<<std::endl;
    /**Note!  handleSchema modifies parms*/
    if(!handleSchema(schema.c_str(), argv[1], parms)){
        return 0;
    }
    std::cout<<"successfully parsed!"<<std::endl;
    auto restConnectionFn=handleHttpConnect(parms["url"].GetString(), parms["port"].GetInt());
    auto params=parms["params"].GetObject();
    auto loans=params["loans"].GetArray();
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
        vasicek::getVasicekMFGFn(expectation, variance),
        creditutilities::getLiquidityRiskFn(lambda, q),
        creditutilities::logLPMCF(
            alpha.Size(),
            creditutilities::getLGDCFFn(alphL, bL, sigL, tau, bL), 
            curriedL, curriedPD, curriedW
        )
    );
    //After here, do the http request
    //restConnectionFn(parms["endpoint"]);
    cf=curriedFullCF(
        std::move(cf), 
        loans
    );
    const auto density=fangoost::computeInvDiscrete(xSteps, xMin, xMax, std::move(cf));

    /*Server svr;

    svr.get("/hi", [](const auto& req, auto& res) {
        res.set_content("Hello World!", "text/plain");
    });

    svr.get(R"(/numbers/(\d+))", [&](const auto& req, auto& res) {
        auto numbers = req.matches[1];
        res.set_content(numbers, "text/plain");
    });

    svr.listen("localhost", 1234);*/
}