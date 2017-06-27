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

/*#include <cstddef>  
#include <cstdint> 
#define RAPIDJSON_NO_SIZETYPEDEFINE
namespace rapidjson { typedef size_t SizeType; }*/

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
}



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


double curriedPD(const rapidjson::Value& loan){return loan["pd"].GetDouble();}


double curriedL(const rapidjson::Value& loan){return loan["l"].GetDouble();} 

double curriedW(const rapidjson::Value& loan, const int& index){return loan["w"][index].GetDouble();} 

class connection_functions {
public:
    typedef websocketpp::lib::shared_ptr<connection_functions> ptr;

    connection_functions(int id, websocketpp::connection_hdl hdl, std::string uri, const rapidjson::Value& params)
      : m_id(id)
      , m_hdl(hdl)
      , m_status("Connecting")
      , m_uri(uri)
      , m_server("N/A")
    {
        serverschema=getServerSchema();
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
       //std::cout<<"successfully assigned variables"<<std::endl;
        /**prepare functions for CF inversion*/
        /**function to retreive vector values from JSON*/
        auto retreiveJSONValue=[](const auto& val){
            return val.GetDouble();
        }; 
        /**expectation, used for the vasicek MGF*/
        expectation=vasicek::computeIntegralExpectationLongRunOne(params["y0"].GetArray(), alpha, m, tau, retreiveJSONValue);
        //std::cout<<"successfully computed expectation"<<std::endl;
        /**variance, used for the vasicek MGF*/ 
        variance=vasicek::computeIntegralVarianceVasicek(alpha, params["sigma"].GetArray(), params["rho"].GetArray(), m, tau, retreiveJSONValue);
    }

    void on_open(client * c, websocketpp::connection_hdl hdl) {
        m_status = "Open";
        client::connection_ptr con = c->get_con_from_hdl(hdl);
        m_server = con->get_response_header("Server");
        websocketpp::lib::error_code ec;
        c->send(hdl, std::string("hello"), websocketpp::frame::opcode::text, ec);
    }
    void on_message(websocketpp::connection_hdl hdl, client::message_ptr msg) {
        websocketpp::lib::error_code ec;
        if(!msg->get_payload().compare("terminate")){
            //std::cout<<msg->get_payload()<<std::endl;
            std::cout<<cf[0][0]<<std::endl;
            std::cout<<cf[uSteps-1][0]<<std::endl;

            auto density=getDensity();
            //std::cout<<cf[0]<<std::endl;
            //std::cout<<cf[uSteps-1]<<std::endl;
            auto dx=fangoost::computeDX(xSteps, xMin, xMax);
            std::cout<<"[";
            for(int i=0; i<density.size()-1;++i){
                std::cout<<"{\"x\":"<<xMin+i*dx<<", \"density\":"<<density[i]<<"},";
            }
            std::cout<<"{\"x\":"<<xMin+(density.size()-1)*dx<<", \"density\":"<<density[density.size()-1]<<"}]"<<std::endl;
            return;
        }
        rapidjson::Document loans;
        if(!handleSchema(serverschema.c_str(), msg->get_payload().c_str(), loans)){
            return;
        }
        //std::cout<<"Got passed schema check"<<std::endl;
        //std::cout<<"Number of loans:"<<loans.Size()<<std::endl;
        mtx.lock(); 

        cf=creditutilities::getFullCFFn(xMin, xMax,
            //vasicek::getLogVasicekMFGFn(expectation, variance),//v->value
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
        
        mtx.unlock();
        /*auto density=getDensity();
        auto dx=fangoost::computeDX(xSteps, xMin, xMax);
        std::cout<<"[";
        for(int i=0; i<density.size()-1;++i){
            std::cout<<"{\"x\":"<<xMin+i*dx<<", \"density\":"<<density[i]<<"},";
        }
        std::cout<<"{\"x\":"<<xMin+(density.size()-1)*dx<<", \"density\":"<<density[density.size()-1]<<"}]"<<std::endl;
        return;*/
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
        /**convert this into computeInvDiscrete...more efficient*/
        return fangoost::computeInvDiscreteLog(xSteps, xMin, xMax, futilities::for_each_parallel(0, uSteps, [&](const auto& index){
            return vasicekLogFN(cf[index]);
        }));
    }

    websocketpp::connection_hdl get_hdl() const {
        return m_hdl;
    }
    
    int get_id() const {
        return m_id;
    }
    
    std::string get_status() const {
        return m_status;
    }
private:
    int m_id;
    websocketpp::connection_hdl m_hdl;
    std::string m_status;
    std::string m_uri;
    std::string m_server;
    std::string m_error_reason;
    std::string serverschema;
    int xSteps;
    int uSteps;
    double xMin;
    double xMax;
    std::vector<double> expectation;
    std::vector<std::vector<double> > variance;

    int m;
    double tau;
    double lambda;
    double q;
    double alphL;
    double bL;
    double sigL;



    std::vector<std::vector<std::complex<double> > > cf;

    //std::function<std::vector<double>(std::vector<double>&&, const rapidjson::Value&)> curriedFullCF;


};


class websocket_endpoint {
public:
    websocket_endpoint () : m_next_id(0) {
        m_endpoint.clear_access_channels(websocketpp::log::alevel::all);
        m_endpoint.clear_error_channels(websocketpp::log::elevel::all);
        m_endpoint.init_asio();
        m_endpoint.start_perpetual();
        //m_thread.reset(new websocketpp::lib::thread(&client::run, &m_endpoint));
    }
    ~websocket_endpoint() {
        m_endpoint.stop_perpetual();
    }
    int connect(std::string const & uri, const rapidjson::Value& inputJson) {
        websocketpp::lib::error_code ec;

        client::connection_ptr con = m_endpoint.get_connection(uri, ec);

        if (ec) {
            std::cout << "> Connect initialization error: " << ec.message() << std::endl;
            return -1;
        }

        int new_id = m_next_id++;
        function_ptr=websocketpp::lib::make_shared<connection_functions>(new_id, con->get_handle(), uri, inputJson);

        con->set_open_handler(websocketpp::lib::bind(
            &connection_functions::on_open,
            function_ptr,
            &m_endpoint,
            websocketpp::lib::placeholders::_1
        ));
        con->set_fail_handler(websocketpp::lib::bind(
            &connection_functions::on_fail,
            function_ptr,
            &m_endpoint,
            websocketpp::lib::placeholders::_1
        ));
        con->set_close_handler(websocketpp::lib::bind(
            &connection_functions::on_close,
            function_ptr,
            &m_endpoint,
            websocketpp::lib::placeholders::_1
        ));
        con->set_message_handler(websocketpp::lib::bind(
            &connection_functions::on_message,
            function_ptr,
            websocketpp::lib::placeholders::_1,
            websocketpp::lib::placeholders::_2
        ));

        m_endpoint.connect(con);

        return new_id;
    }
    void run(){
        m_endpoint.run();
    }
    void close(int id, websocketpp::close::status::value code, std::string reason) {
        websocketpp::lib::error_code ec;
        m_endpoint.close(function_ptr->get_hdl(), code, reason, ec);
        if (ec) {
            std::cout << "> Error initiating close: " << ec.message() << std::endl;
        }
    }
    /*std::string status(){
        return function_ptr->get_status();
    }
    std::vector<double> getDensity(){
        return function_ptr->getDensity();
    }*/
private:
    connection_functions::ptr function_ptr;
    client m_endpoint;
    int m_next_id;
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
    websocket_endpoint endpoint;
    int id=endpoint.connect(
        parms["url"].GetString(), 
        parms["params"].GetObject()
    );
    //std::cout<<"this is the connection id: "<<id<<std::endl;
    endpoint.run();
    /*while(!endpoint.status().compare("open")) { //returns zero when equal
        std::cout<<endpoint.status()<<std::endl;
        sleep(1);//might want this to be less at some point
    }*/
    
}


