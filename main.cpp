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
#include <websocketpp/config/asio_no_tls_client.hpp>
#include <websocketpp/client.hpp>

typedef websocketpp::client<websocketpp::config::asio_client> client;

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

double curriedPD(const rapidjson::Value& loan){return loan["pd"].GetDouble();}

double curriedL(const rapidjson::Value& loan){return loan["l"].GetDouble();} 

double curriedW(const rapidjson::Value& loan, const int& index){return loan["w"][index].GetDouble();} 


struct CreditRiskBatch{
private:
    std::vector<std::vector<std::complex<double> > > cf;
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
    std::function<std::vector<std::vector<std::complex<double> > >(CreditRiskBatch&)> incrementalCF;
public:
    CreditRiskBatch(int uSteps_, int xSteps_, double tau_){
        xSteps=xSteps_;
        uSteps=uSteps_;
        tau=tau_;
       
    }
    void setXRange(double xMin_, double xMax_){
        xMin=xMin_;
        xMax=xMax_;
        //m=numSystemic; 
    }
    void setLiquidityParameters(double lambda_, double q_){
        lambda=lambda_;
        q=q_;
    }
    void setLGDParameters(double alphL_, double bL_, double sigL_){
        alphL=alphL_;
        bL=bL_;
        sigL=sigL_;
    }
    template<typename Array, typename TwoDArray>
    void setSystemicParameters(const Array& y0, const Array& sigma, const Array& alpha, const TwoDArray& rho){
        m=alpha.Size();
        /**function to retreive vector values from JSON*/
        auto retrieveJSONValue=[](const auto& val){
            return val.GetDouble();
        };
        /**expectation, used for the vasicek MGF*/
        expectation=vasicek::computeIntegralExpectationLongRunOne(y0, alpha, m, tau, retrieveJSONValue);
        /**variance, used for the vasicek MGF*/ 
        variance=vasicek::computeIntegralVarianceVasicek(alpha, sigma, rho, m, tau, retrieveJSONValue);
        cf=std::vector<std::vector<std::complex<double> > >(uSteps, std::vector<std::complex<double> >(m, 0));
        
        incrementalCF=[this](CreditRiskBatch& crb){
            creditutilities::getFullCFFn(xMin, xMax,
                creditutilities::getLiquidityRiskFn(lambda, q),//u->complex
                creditutilities::logLPMCF( 
                    m,
                    creditutilities::getLGDCFFn(alphL, bL, sigL, tau, bL),
                    curriedL, curriedPD, curriedW
                )//u, loans->v, n
            )
        }
    }
    template<typename Array>
    void nextBatch(const Array& loans){
        //In theory, this can be made much more efficient by making this lambda a member variable.  
        //however, I'm struggling to get it to work.  
        cf=creditutilities::getFullCFFn(xMin, xMax,
            creditutilities::getLiquidityRiskFn(lambda, q),//u->complex
            creditutilities::logLPMCF( 
                m,
                creditutilities::getLGDCFFn(alphL, bL, sigL, tau, bL),
                curriedL, curriedPD, curriedW
            )//u, loans->v, n
        )(
            std::move(cf), 
            loans
        ); 
    }
    double getMin(){
        return xMin;
    }
    double getMax(){
        return xMax;
    }
    int getXSteps(){
        return xSteps;
    }
    std::vector<double> getDensity(){
        auto vasicekLogFN=vasicek::getLogVasicekMFGFn(expectation, variance);
        return fangoost::computeInvDiscreteLog(xSteps, xMin, xMax, futilities::for_each_parallel(0, uSteps, [&](const auto& index){
            return vasicekLogFN(cf[index]);
        }));
    }
    std::vector<double> getCDF(){
        auto vasicekLogFN=vasicek::getLogVasicekMFGFn(expectation, variance);
        return cfdistutilities::computeCDF(xSteps, xMin, xMax, fangoost::convertLogCFToRealExp(xMin, xMax, futilities::for_each_parallel(0, uSteps, [&](const auto& index){
            return vasicekLogFN(cf[index]);
        })));
    } 
    double getVaR(double alpha){
        auto vasicekLogFN=vasicek::getLogVasicekMFGFn(expectation, variance);
        double prec=.0000000001;//this seems to work pretty well
        const auto EL=getEL(vasicekLogFN);
        return cfdistutilities::computeVaRNewtonDiscrete(alpha, prec, prec, xMin, xMax, EL, fangoost::convertLogCFToRealExp(xMin, xMax, futilities::for_each_parallel(0, uSteps, [&](const auto& index){
            return vasicekLogFN(cf[index]);
        })));
        
    }
    auto getES(double alpha){
        auto vasicekLogFN=vasicek::getLogVasicekMFGFn(expectation, variance);
        double prec=.0000000001;
        return cfdistutilities::computeESDiscrete(alpha, prec, xMin, xMax, fangoost::convertLogCFToRealExp(xMin,xMax, futilities::for_each_parallel(0, uSteps, [&](const auto& index){
            return vasicekLogFN(cf[index]);
        })));
    }
    double getEL(){
        auto vasicekLogFN=vasicek::getLogVasicekMFGFn(expectation, variance);
        return cfdistutilities::computeELDiscrete(xMin, xMax, fangoost::convertLogCFToRealExp(xMin,xMax, futilities::for_each_parallel(0, uSteps, [&](const auto& index){
            return vasicekLogFN(cf[index]);
        })));
    }
    template<typename VasLogFN>
    double getEL(const VasLogFN& vasicekLogFN){
        return cfdistutilities::computeELDiscrete(xMin, xMax, fangoost::convertLogCFToRealExp(xMin,xMax, futilities::for_each_parallel(0, uSteps, [&](const auto& index){
            return vasicekLogFN(cf[index]);
        })));
    }
};


class websocket_endpoint {
public:
    websocket_endpoint (std::string uri, const rapidjson::Value& params): m_status("Connecting"), m_uri(uri), m_server("N/A") {
        m_endpoint.clear_access_channels(websocketpp::log::alevel::all);
        m_endpoint.clear_error_channels(websocketpp::log::elevel::all);
        m_endpoint.init_asio();
        m_endpoint.start_perpetual();
        increment=0;
        numSend=100000;//something large
        batchCF=new CreditRiskBatch(
            params["uSteps"].GetInt(),
            params["xSteps"].GetInt(),
            params["tau"].GetDouble()
        );
        batchCF->setLiquidityParameters(
            params["lambda"].GetDouble(), 
            params["q"].GetDouble()
        );
        batchCF->setLGDParameters(
            params["alphL"].GetDouble(),
            params["bL"].GetDouble(),
            params["sigL"].GetDouble()
        );
        batchCF->setSystemicParameters(
            params["y0"].GetArray(),
            params["sigma"].GetArray(),
            params["alpha"].GetArray(),
            params["rho"].GetArray()
        );
    }
    ~websocket_endpoint() {
        m_endpoint.stop_perpetual();
        delete batchCF;
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
    bool handleLoans(client::message_ptr msg){
        rapidjson::Document loans;
        if(!handleSchema(serverSchema, msg->get_payload().c_str(), loans, true)){
            return false;
        }
        batchCF->nextBatch(loans.GetArray());
        //mtx.unlock();
        increment++;
        /**if done sending*/
        if(numSend==increment){
            //auto VaR=batchCF->getVaR(.01);//.9997
            //std::cout<<"{\"VaR\":"<<VaR<<"}"<<std::endl;
            //VaR=batchCF->getVaR(.0003);
            //std::cout<<"{\"VaR\":"<<VaR<<"}"<<std::endl;
            auto ES=batchCF->getES(.01);
            std::cout<<"{\"ES\":"<<std::get<cfdistutilities::ES>(ES)<<"}"<<std::endl;
            std::cout<<"{\"VaR\":"<<std::get<cfdistutilities::VAR>(ES)<<"}"<<std::endl;
            //ES=batchCF->getES(.0003);
            //std::cout<<"{\"ES\":"<<ES<<"}"<<std::endl;
 
            m_status="done";
            close(websocketpp::close::status::normal, "end of program");
        }
        return true;
    }
    bool handleMetaLoans(client::message_ptr msg){
        websocketpp::lib::error_code ec;
        rapidjson::Document metadata;
        if(!handleSchema(metaSchema, msg->get_payload().c_str(), metadata, true)){
            return false;
        }
        numSend=metadata["numSend"].GetInt();
        batchCF->setXRange(
            metadata["xMin"].GetDouble(),
            metadata["xMax"].GetDouble()
        );
        return true;
    }
    void on_open(client * c, websocketpp::connection_hdl hdl) {
        m_status = "Open";
        client::connection_ptr con = c->get_con_from_hdl(hdl);
        m_server = con->get_response_header("Server");
        websocketpp::lib::error_code ec;
        c->send(hdl, std::string("getSummaryStats"), websocketpp::frame::opcode::text, ec);
    }
    void on_message(websocketpp::connection_hdl hdl, client::message_ptr msg) {
        if(!handleLoans(msg)){
            handleMetaLoans(msg);
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
    CreditRiskBatch *batchCF;
    /**Total number of received messages.  The server will tell you this as part of metadata...initially set to a high number so that program doesn't return prior to receiving the metadata*/
    int numSend;
    /**this counts how many times have received messages.  once this equals numSend, the program aggregates, cleans up, and exits*/
    int increment;
};

int main(int argc, char* argv[]){
    if(!ensureEnoughArgs(argc)){
        std::cout<<"not enough args"<<std::endl;
        return 0;
    }
    rapidjson::Document parms;
    /**Note!  handleSchema modifies parms*/
    if(!handleSchema(inputSchema, argv[1], parms)){
        return 0;
    }
    websocket_endpoint endpoint(
        parms["url"].GetString(), 
        parms["params"].GetObject()
    );
    int id=endpoint.connect();
    if(id==-1){//then something went wrong
        return 0;
    }
    endpoint.run();
    
}


