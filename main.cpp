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


#include "CreditUtilities.h"
#include <functional>
#include "document.h" //rapidjson
#include "writer.h" //rapidjson
#include "stringbuffer.h" //rapidjson
#include "schema.h"//rapidjson
#include "error/en.h" //rapidjson
#include "FangOost.h"
#include "FunctionalUtilities.h"
#include "Vasicek.h"
#include <mutex>   
#include "CheckSchema.h" //handleSchema
#include <vector>
#include <complex>
#include <stdio.h> //for binary data
#include <set>
#include <websocketpp/config/asio_no_tls_client.hpp>
#include <websocketpp/client.hpp>

#include <typeinfo> //for checking types..remove in production

typedef websocketpp::client<websocketpp::config::asio_client> client;

#include <iostream>
extern "C" {
    extern char _binary_inputschema_json_start; //binary data
    extern char _binary_inputschema_json_end;//binary data
    extern char _binary_serverschema_json_start; //binary data
    extern char _binary_serverschema_json_end;//binary data
    extern char _binary_metaschema_json_start; //binary data
    extern char _binary_metaschema_json_end;//binary data
}

const double maxPercentLoss=.14;//if more than this, in big trouble

std::mutex mtx;    
bool ensureEnoughArgs(int argc){
    return argc>1?true:false;
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
std::string getMetaSchema(){
    std::string metaschema;
    char*  schemaItem = &_binary_metaschema_json_start; //is this compile time?
    while ( schemaItem != &_binary_metaschema_json_end ) metaschema+=(*schemaItem++); //is this compile time?
    return metaschema;
}


double curriedPD(const rapidjson::Value& loan){return loan["pd"].GetDouble();}


double curriedL(const rapidjson::Value& loan){return loan["l"].GetDouble();} 

double curriedW(const rapidjson::Value& loan, const int& index){return loan["w"][index].GetDouble();} 


class websocket_endpoint {
public:
    websocket_endpoint (std::string uri, const rapidjson::Value& params): m_status("Connecting"), m_uri(uri), m_server("N/A") {
        m_endpoint.clear_access_channels(websocketpp::log::alevel::all);
        m_endpoint.clear_error_channels(websocketpp::log::elevel::all);
        m_endpoint.init_asio();
        m_endpoint.start_perpetual();
        increment=0;
        numSend=100000;//something large
        serverschema=getServerSchema();
        metaschema=getMetaSchema();
        auto alpha=params["alpha"].GetArray();
        tau=params["tau"].GetDouble();
        xSteps=params["xSteps"].GetInt();
        uSteps=params["uSteps"].GetInt();
        xMin=params["xMin"].GetDouble();
        xMax=params["xMax"].GetDouble();
        lambda=params["lambda"].GetDouble();
        q=params["q"].GetDouble();
        alphL=params["alphL"].GetDouble();
        bL=params["bL"].GetDouble();
        sigL=params["sigL"].GetDouble();
        m=alpha.Size(); 
        cf=std::vector<std::vector<std::complex<double> > >(uSteps, std::vector<std::complex<double> >(m, 0));
        /**prepare functions for CF inversion*/
        /**function to retreive vector values from JSON*/
        auto retreiveJSONValue=[](const auto& val){
            return val.GetDouble();
        }; 
        /**expectation, used for the vasicek MGF*/
        expectation=vasicek::computeIntegralExpectationLongRunOne(params["y0"].GetArray(), alpha, m, tau, retreiveJSONValue);
        /**variance, used for the vasicek MGF*/ 
        variance=vasicek::computeIntegralVarianceVasicek(alpha, params["sigma"].GetArray(), params["rho"].GetArray(), m, tau, retreiveJSONValue);
    }
    ~websocket_endpoint() {
        m_endpoint.stop_perpetual();
    }
    int connect() {
        websocketpp::lib::error_code ec;

        client::connection_ptr con = m_endpoint.get_connection(m_uri, ec);
        
        if (ec) {
            std::cout << "> Connect initialization error: " << ec.message() << std::endl;
            return -1;
        }
        m_hdl=con->get_handle();
        con->set_open_handler(websocketpp::lib::bind(
            &websocket_endpoint::on_open,
            this,
            &m_endpoint,
            websocketpp::lib::placeholders::_1
        ));
        con->set_fail_handler(websocketpp::lib::bind(
            &websocket_endpoint::on_fail,
            this,
            &m_endpoint,
            websocketpp::lib::placeholders::_1
        ));
        con->set_close_handler(websocketpp::lib::bind(
            &websocket_endpoint::on_close,
            this,
            &m_endpoint,
            websocketpp::lib::placeholders::_1
        ));
        con->set_message_handler(websocketpp::lib::bind(
            &websocket_endpoint::on_message,
            this,
            websocketpp::lib::placeholders::_1,
            websocketpp::lib::placeholders::_2
        ));
        m_endpoint.connect(con);
        return 1;
    }
    void on_open(client * c, websocketpp::connection_hdl hdl) {
        m_status = "Open";
        client::connection_ptr con = c->get_con_from_hdl(hdl);
       
        m_server = con->get_response_header("Server");
        websocketpp::lib::error_code ec;
        c->send(hdl, std::string("getSummaryStats"), websocketpp::frame::opcode::text, ec);
    }
    void on_message(websocketpp::connection_hdl hdl, client::message_ptr msg) {
        websocketpp::lib::error_code ec;
        rapidjson::Document metadata;
        if(handleSchema(metaschema.c_str(), msg->get_payload().c_str(), metadata, true)){
            numSend=metadata["numSend"].GetInt();
            xMin=-metadata["exposure"].GetDouble()*maxPercentLoss;
            return;
        }
        rapidjson::Document loans;
        if(!handleSchema(serverschema.c_str(), msg->get_payload().c_str(), loans)){
            return;
        }
        
        /**it appears I don't need this lock...if receive segmentation errors...look here first*/
        //mtx.lock(); 

        cf=creditutilities::getFullCFFn(xMin, xMax,
            creditutilities::getLiquidityRiskFn(lambda, q),//u->complex
            creditutilities::logLPMCF( 
                m,
                creditutilities::getLGDCFFn(alphL, bL, sigL, tau, bL),
                curriedL, curriedPD, curriedW
            )//u, loans->v, n
        )(
            std::move(cf), 
            loans.GetArray() 
        ); 
        
        //mtx.unlock();
        increment++;

        /**if done sending*/
        if(numSend==increment){
            auto density=getDensity();
            auto dx=fangoost::computeDX(xSteps, xMin, xMax);
            std::cout<<"[";
            for(int i=0; i<density.size()-1;++i){
                std::cout<<"{\"x\":"<<xMin+i*dx<<", \"density\":"<<density[i]<<"},";
            }
            std::cout<<"{\"x\":"<<xMin+(density.size()-1)*dx<<", \"density\":"<<density[density.size()-1]<<"}]"<<std::endl;
            m_status="done";
            close(websocketpp::close::status::normal, "end of program");
            return;
            
        }
    }
    void on_fail(client * c, websocketpp::connection_hdl hdl) {
        m_status = "Failed";
        client::connection_ptr con = c->get_con_from_hdl(hdl);
        m_server = con->get_response_header("Server");
        m_error_reason = con->get_ec().message();
    }
    void on_close(client * c, websocketpp::connection_hdl hdl) {
        m_status = "Closed";
        client::connection_ptr con = c->get_con_from_hdl(hdl);
        std::stringstream s;
        s << "close code: " << con->get_remote_close_code() << " (" 
          << websocketpp::close::status::get_string(con->get_remote_close_code()) 
          << "), close reason: " << con->get_remote_close_reason();
        m_error_reason = s.str();
    }
    std::vector<double> getDensity(){
        auto vasicekLogFN=vasicek::getLogVasicekMFGFn(expectation, variance);
        return fangoost::computeInvDiscreteLog(xSteps, xMin, xMax, futilities::for_each_parallel(0, uSteps, [&](const auto& index){
            return vasicekLogFN(cf[index]);
        }));
    }
    void run(){
        m_endpoint.run();
    }
    void close(websocketpp::close::status::value code, std::string reason) {
        websocketpp::lib::error_code ec;
        /*tell "run" to return on close*/
        m_endpoint.stop_perpetual();
        m_endpoint.close(m_hdl, code, reason, ec);
        if (ec) {
            std::cout << "> Error initiating close: " << ec.message() << std::endl;
        }
    }
    void send(std::string message) {
        websocketpp::lib::error_code ec;        
        m_endpoint.send(m_hdl, message, websocketpp::frame::opcode::text, ec);
        if (ec) {
            std::cout << "> Error sending message: " << ec.message() << std::endl;
            return;
        }
    }
private:
    client m_endpoint;
    websocketpp::connection_hdl m_hdl;
    std::string m_status;
    std::string m_uri;
    std::string m_server;
    std::string m_error_reason;
    std::string serverschema;
    std::string metaschema;

    /**start of parameters*/
    int xSteps;
    int uSteps;
    double xMin;
    double xMax;
    std::vector<double> expectation;
    std::vector<std::vector<double> > variance;
    /**number of macro variables*/
    int m;
    /**time horizon*/
    double tau;
    /**Liquidity parameters*/
    double lambda;
    double q;
    /**CIR/LGD CF parameters*/
    double alphL;
    double bL;
    double sigL;
    /**Total number of received messages.  The server will tell you this as part of metadata...initially set to a high number so that program doesn't return prior to receiving the metadata*/
    int numSend;
    /**this counts how many times have received messages.  once this equals numSend, the program aggregates, cleans up, and exits*/
    int increment;


    std::vector<std::vector<std::complex<double> > > cf;




};

int main(int argc, char* argv[]){
    if(!ensureEnoughArgs(argc)){
        std::cout<<"not enough args"<<std::endl;
        return 0;
    }
    rapidjson::Document parms;
    /**Note!  handleSchema modifies parms*/
    if(!handleSchema(getInputSchema().c_str(), argv[1], parms)){
        return 0;
    }
    websocket_endpoint endpoint(
        parms["url"].GetString(), 
        parms["params"].GetObject()
    );
    int id=endpoint.connect();
    if(id==-1){
        return 0;
    }
    endpoint.run();
    
}


