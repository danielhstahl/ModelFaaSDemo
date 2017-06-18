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


bool handleSchema(const char* schemaJson, const char* inputJson, rapidjson::Document& d){
    rapidjson::Document sd;
    if (sd.Parse(schemaJson).HasParseError()) {
        std::cout << "parse error:" << rapidjson::GetParseError_En(sd.GetParseError()) << " offset:" << sd.GetErrorOffset() << std::endl;
        return false;
        // the schema is not a valid JSON.
        // ...       
    }
    rapidjson::SchemaDocument schema(sd); // Compile a Document to SchemaDocument

    if (d.Parse(inputJson).HasParseError()) {
        std::cout << "parse error:" << rapidjson::GetParseError_En(sd.GetParseError()) << " offset:" << sd.GetErrorOffset() << std::endl;
        return false;
        // the input is not a valid JSON.
        // ...       
    }
    rapidjson::SchemaValidator validator(schema);

    if (!d.Accept(validator)) {
        // Input JSON is invalid according to the schema
        // Output diagnostic information
        rapidjson::StringBuffer sb;
        validator.GetInvalidSchemaPointer().StringifyUriFragment(sb);
        std::cout<<"Invalid schema: %s\n"<<sb.GetString()<<std::endl;
        printf("Invalid keyword: %s\n", validator.GetInvalidSchemaKeyword());
        sb.Clear();
        validator.GetInvalidDocumentPointer().StringifyUriFragment(sb);
        printf("Invalid document: %s\n", sb.GetString());
        return false;
    }
    return true;
}
auto handleWSConnect(const std::string wsconnect){
    
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
    /**Note!  handleSchema modifies parms*/
    if(!handleSchema(schema.c_str(), argv[1], parms)){
        return 0;
    }
    auto restConnectionFn=handleHttpConnect(parms["url"].GetString(), parms["port"].GetInt());
    
    /**prepare functions for CF inversion*/
    /*std::vector<std::complex<double> > cf(m);
    const auto expectation=creditutilities::computeExpectationVasicek(y0, alpha, tau);
	const auto variance=creditutilities::computeVarianceVasicek(alpha, sigma, rho, tau);
    auto curriedLiquidity=creditutilities::getLiquidityRiskFn(lambda, q);
    auto curriedPD=[](const auto& loan){return loan.pd;}; 
    auto curriedL=[](const auto& loan){return loan.l;}; 
    auto curriedW=[](const auto& loan, const auto& index){return loan.w[index];};
    auto curriedLGDCF=creditutilities::getLGDCFFn(alphL, bL, sigL, tau, bL);
    auto curriedVasicek=creditutilities::getVasicekMFGFn(expectation, variance);
    auto curriedlogLPMCF=creditutilities::logLPMCF(curriedLGDCF, curriedL, curriedPD, curriedW);
    auto curriedFullCF=creditutilities::getFullCFFn(xMin, xMax, 
        curriedVasicek,
        curriedLiquidity,
        curriedLogLPMCF
    );
    //After here, do the http request
    //restConnectionFn(parms["endpoint"]);
    cf=curriedFullCF(
        std::move(cf), 
        loans
    );
    const auto density=fangoost::computeInvDiscrete(xNum, xmin, xmax, std::move(cf));
*/
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