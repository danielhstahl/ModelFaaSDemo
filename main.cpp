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
#include "CheckSchema.h" //handleSchema
#include <vector>
#include <complex>
#include <stdio.h> //for binary data
#include <set>
#include <nan.h>
using namespace Nan;  
using namespace v8;

#include <iostream>
extern "C" {
    extern char _binary_inputschema_json_start; //binary data
    extern char _binary_inputschema_json_end;//binary data
    extern char _binary_serverschema_json_start; //binary data
    extern char _binary_serverschema_json_end;//binary data
}




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


class LoanCF : public Nan::ObjectWrap {
 public:
  static NAN_MODULE_INIT(Init) {
    v8::Local<v8::FunctionTemplate> tpl = Nan::New<v8::FunctionTemplate>(New);
    tpl->SetClassName(Nan::New("LoanCF").ToLocalChecked());
    tpl->InstanceTemplate()->SetInternalFieldCount(1);

    Nan::SetPrototypeMethod(tpl, "supplyCF", supplyCF);
    Nan::SetPrototypeMethod(tpl, "computeDensity", computeDensity);

    constructor().Reset(Nan::GetFunction(tpl).ToLocalChecked());
    Nan::Set(target, Nan::New("LoanCF").ToLocalChecked(),
      Nan::GetFunction(tpl).ToLocalChecked());
  }

 private:
  explicit LoanCF(char* inputJson)/* : value_(value)*/ {}
  ~LoanCF() {}

  static NAN_METHOD(New) {
    if (info.IsConstructCall()) {
        char* inputJson = (char*) node::Buffer::Data(info[0]->ToObject());
        LoanCF *obj = new LoanCF(inputJson);
        obj->Wrap(info.This());
        rapidjson::Document parms;
        obj->serverschema=getServerSchema();
        std::cout<<obj->serverschema<<std::endl;
        std::cout<<getInputSchema()<<std::endl;
        /**Note!  handleSchema modifies parms*/
        if(!handleSchema(getInputSchema().c_str(), inputJson, parms)){
            return;
        }
        auto params=parms["params"].GetObject();
        //auto loans=params["loans"].GetArray();
        auto alpha=params["alpha"].GetArray();
        auto tau=params["tau"].GetDouble();
        obj->xSteps=params["xSteps"].GetInt();
        auto uSteps=params["uSteps"].GetInt();
        obj->xMin=params["xMin"].GetDouble();
        obj->xMax=params["xMax"].GetDouble();
        auto lambda=params["lambda"].GetDouble();
        auto q=params["q"].GetDouble();
        auto alphL=params["alphL"].GetDouble();
        auto bL=params["bL"].GetDouble();
        auto sigL=params["sigL"].GetDouble();
        obj->cf=std::vector<double>(uSteps);
        std::cout<<"successfully assigned variables"<<std::endl;
        /**prepare functions for CF inversion*/
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
        obj->curriedFullCF=creditutilities::getFullCFFn(obj->xMin, obj->xMax,
            vasicek::getVasicekMFGFn(expectation, variance),//v->value
            creditutilities::getLiquidityRiskFn(lambda, q),//u->complex
            creditutilities::logLPMCF(
                alpha.Size(),
                creditutilities::getLGDCFFn(alphL, bL, sigL, tau, bL), 
                curriedL, curriedPD, curriedW
            )//u, loans->v
        );
        info.GetReturnValue().Set(info.This());



      //double value = info[0]->IsUndefined() ? 0 : Nan::To<double>(info[0]).FromJust();
      
    } else {
      const int argc = 1;
      v8::Local<v8::Value> argv[argc] = {info[0]};
      v8::Local<v8::Function> cons = Nan::New(constructor());
      info.GetReturnValue().Set(cons->NewInstance(argc, argv));
    }
  }

  static NAN_METHOD(supplyCF) {
    LoanCF* obj = Nan::ObjectWrap::Unwrap<LoanCF>(info.Holder());
    //info.GetReturnValue().Set(obj->handle());
    char* loanJson = (char*) node::Buffer::Data(info[0]->ToObject());
    rapidjson::Document loans;
    if(!handleSchema(obj->serverschema.c_str(), loanJson, loans)){
        return;
    }
    obj->cf=obj->curriedFullCF(
        std::move(obj->cf), 
        loans.GetArray()
    );   
    
  }

  static NAN_METHOD(computeDensity) {
      Nan:: HandleScope scope;

    LoanCF* obj = Nan::ObjectWrap::Unwrap<LoanCF>(info.Holder());
    std::vector<double> density=fangoost::computeInvDiscrete(obj->xSteps, obj->xMin, obj->xMax, std::move(obj->cf));
    int densitySize=density.size();
    v8::Local<v8::Array> array = Nan::New<v8::Array>(densitySize);
    for(int i=0; i<densitySize;++i){
        Nan::Set(array, i, Nan::New<v8::Number>(density[i]));
    }
    info.GetReturnValue().Set(array);
  }

  static inline Nan::Persistent<v8::Function> & constructor() {
    static Nan::Persistent<v8::Function> my_constructor;
    return my_constructor;
  }
  std::vector<double > cf;//(uSteps);
  std::function<std::vector<double>(std::vector<double>&&, const rapidjson::Value&)> curriedFullCF;
  //std::function<void()> curriedFullCF;
  double xMin;
  double xMax;
  int xSteps;
    std::string serverschema;
  //double value_;
};
NODE_MODULE(objectwrapper, LoanCF::Init);
