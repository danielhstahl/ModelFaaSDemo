#define CATCH_CONFIG_MAIN  // This tells Catch to provide a main() - only do this in one cpp file
#include "catch.hpp"
#include "CheckSchema.h"
#include "document.h" //rapidjson
#include "writer.h" //rapidjson
#include "stringbuffer.h" //rapidjson
#include "schema.h"//rapidjson
#include "error/en.h" //rapidjson
#include <string>
#include "Vasicek.h"
#include "CreditUtilities.h"
#include <vector>

const char *inputSchema =
#include "inputschema.json"
;
const char *serverSchema =
#include "serverschema.json"
;
const char *metaSchema =
#include "metaschema.json"
;


struct loan{
	double pd;
	//std::function<Complex(const Complex&)> lgdCF;//characteristic function
	double exposure;
	std::vector<double> w;
	loan(double pd_, double exposure_, std::vector<double>&& w_){
			pd=pd_;
			exposure=exposure_;
			w=w_;
	}
	loan(){
    }
};
TEST_CASE("Returns false when not given proper schema", "[schema]"){
    std::string badSchema="hello";
    std::string goodJson="{\"url\":\"localhost\",\"port\":3000,\"endpoint\":\"hello\",\"params\":{\"y0\":[1,2,3],\"sigma\":[0.2, 0.3, 0.1],\"alpha\":[0.4,0.5,0.6],\"rho\":[[1,0.5,0.3],[-0.3,1,-0.4],[0.3,0.5,1]],\"tau\":1,\"lambda\":0.003,\"q\":400,\"uSteps\":256,\"xSteps\":1024,\"alphL\":0.4,\"bL\":0.5,\"sigL\":0.2,\"loans\":[{\"w\":[0.1,0.3,0.6],\"l\":3000,\"pd\":0.02},{\"w\":[0.1,0.3,0.6],\"l\":3000,\"pd\":0.02},{\"w\":[0.1,0.3,0.6],\"l\":3000,\"pd\":0.02},{\"w\":[0.1,0.3,0.6],\"l\":3000,\"pd\":0.02}]}}";
    rapidjson::Document parms;
    REQUIRE(handleSchema(badSchema.c_str(), goodJson.c_str(), parms)==false); 
}
TEST_CASE("Returns false when not given proper json", "[schema]"){
    //std::string goodSchema="{\"$schema\":\"http://json-schema.org/draft-04/schema#\",\"type\":\"object\",\"properties\":{\"url\":{\"type\":\"string\"},\"port\":{\"type\":\"integer\"},\"endpoint\":{\"type\":\"string\"},\"params\":{\"type\":\"object\",\"properties\":{\"xMin\":{\"type\":\"number\"},\"xMax\":{\"type\":\"number\"},\"y0\":{\"type\":\"array\",\"items\":{\"type\":\"number\"}},\"sigma\":{\"type\":\"array\",\"items\":{\"type\":\"number\"}},\"alpha\":{\"type\":\"array\",\"items\":{\"type\":\"number\"}},\"rho\":{\"type\":\"array\",\"items\":{\"type\":\"array\",\"items\":{\"type\":\"number\"}}},\"tau\":{\"type\":\"number\"},\"lambda\":{\"type\":\"number\"},\"q\":{\"type\":\"number\"},\"uSteps\":{\"type\":\"integer\"}, \"xSteps\":{\"type\":\"integer\"}, \"alphL\":{\"type\":\"number\"},\"bL\":{\"type\":\"number\"},\"sigL\":{\"type\":\"number\"},\"loans\":{\"type\":\"array\",\"items\":{\"type\":\"object\",\"properties\":{\"w\":{\"type\":\"array\",\"items\":{\"type\":\"number\"}},\"pd\":{\"type\":\"number\"},\"l\":{\"type\":\"number\"}}}}},\"additionalProperties\":false,\"required\":[\"xMin\",\"xMax\",\"y0\",\"alpha\",\"sigma\", \"rho\",\"tau\",\"lambda\",\"q\",\"alphL\",\"bL\",\"sigL\",\"loans\"]}},\"additionalProperties\":false,\"required\":[\"url\",\"port\",\"endpoint\",\"params\"]}";
    std::string badJson="Hello";
    rapidjson::Document parms;
    REQUIRE(handleSchema(inputSchema, badJson.c_str(), parms)==false); 
}
TEST_CASE("Returns false when not given json that conforms to schema", "[schema]"){
    //std::string goodSchema="{\"$schema\":\"http://json-schema.org/draft-04/schema#\",\"type\":\"object\",\"properties\":{\"url\":{\"type\":\"string\"},\"port\":{\"type\":\"integer\"},\"endpoint\":{\"type\":\"string\"},\"params\":{\"type\":\"object\",\"properties\":{\"xMin\":{\"type\":\"number\"},\"xMax\":{\"type\":\"number\"},\"y0\":{\"type\":\"array\",\"items\":{\"type\":\"number\"}},\"sigma\":{\"type\":\"array\",\"items\":{\"type\":\"number\"}},\"alpha\":{\"type\":\"array\",\"items\":{\"type\":\"number\"}},\"rho\":{\"type\":\"array\",\"items\":{\"type\":\"array\",\"items\":{\"type\":\"number\"}}},\"tau\":{\"type\":\"number\"},\"lambda\":{\"type\":\"number\"},\"q\":{\"type\":\"number\"},\"uSteps\":{\"type\":\"integer\"}, \"xSteps\":{\"type\":\"integer\"}, \"alphL\":{\"type\":\"number\"},\"bL\":{\"type\":\"number\"},\"sigL\":{\"type\":\"number\"},\"loans\":{\"type\":\"array\",\"items\":{\"type\":\"object\",\"properties\":{\"w\":{\"type\":\"array\",\"items\":{\"type\":\"number\"}},\"pd\":{\"type\":\"number\"},\"l\":{\"type\":\"number\"}}}}},\"additionalProperties\":false,\"required\":[\"xMin\",\"xMax\",\"y0\",\"alpha\",\"sigma\", \"rho\",\"tau\",\"lambda\",\"q\",\"alphL\",\"bL\",\"sigL\",\"loans\"]}},\"additionalProperties\":false,\"required\":[\"url\",\"port\",\"endpoint\",\"params\"]}";
    std::string badJson="{\"hello\":\"world\"}";
    rapidjson::Document parms;
    REQUIRE(handleSchema(inputSchema, badJson.c_str(), parms)==false); 
}
TEST_CASE("Returns true when  given json that conforms to schema", "[schema]"){
    //std::string goodSchema="{\"$schema\":\"http://json-schema.org/draft-04/schema#\",\"type\":\"object\",\"properties\":{\"url\":{\"type\":\"string\"},\"port\":{\"type\":\"integer\"},\"endpoint\":{\"type\":\"string\"},\"params\":{\"type\":\"object\",\"properties\":{\"xMin\":{\"type\":\"number\"},\"xMax\":{\"type\":\"number\"},\"y0\":{\"type\":\"array\",\"items\":{\"type\":\"number\"}},\"sigma\":{\"type\":\"array\",\"items\":{\"type\":\"number\"}},\"alpha\":{\"type\":\"array\",\"items\":{\"type\":\"number\"}},\"rho\":{\"type\":\"array\",\"items\":{\"type\":\"array\",\"items\":{\"type\":\"number\"}}},\"tau\":{\"type\":\"number\"},\"lambda\":{\"type\":\"number\"},\"q\":{\"type\":\"number\"},\"uSteps\":{\"type\":\"integer\"}, \"xSteps\":{\"type\":\"integer\"}, \"alphL\":{\"type\":\"number\"},\"bL\":{\"type\":\"number\"},\"sigL\":{\"type\":\"number\"},\"loans\":{\"type\":\"array\",\"items\":{\"type\":\"object\",\"properties\":{\"w\":{\"type\":\"array\",\"items\":{\"type\":\"number\"}},\"pd\":{\"type\":\"number\"},\"l\":{\"type\":\"number\"}}}}},\"additionalProperties\":false,\"required\":[\"xMin\",\"xMax\",\"y0\",\"alpha\",\"sigma\", \"rho\",\"tau\",\"lambda\",\"q\",\"alphL\",\"bL\",\"sigL\",\"loans\"]}},\"additionalProperties\":false,\"required\":[\"url\",\"port\",\"endpoint\",\"params\"]}";
    std::string goodJson="{\"url\":\"localhost\",\"endpoint\":\"hello\",\"params\":{\"y0\":[1,2,3],\"sigma\":[0.2, 0.3, 0.1],\"alpha\":[0.4,0.5,0.6],\"rho\":[[1,0.5,0.3],[-0.3,1,-0.4],[0.3,0.5,1]],\"tau\":1,\"lambda\":0.003,\"q\":400,\"uSteps\":256,\"xSteps\":1024,\"alphL\":0.4,\"bL\":0.5,\"sigL\":0.2}}";
    rapidjson::Document parms;
    REQUIRE(handleSchema(inputSchema, goodJson.c_str(), parms)==true); 
}

TEST_CASE("getLiquidityRiskFn returns 0 when given 0", "[CreditUtilities]"){
    auto liquidFn=creditutilities::getLiquidityRiskFn(.5, .2);
    REQUIRE(liquidFn(0)==Approx(0.0));

}
TEST_CASE("getLiquidityRiskFn returns -u when lambda is zero", "[CreditUtilities]"){
    auto liquidFn=creditutilities::getLiquidityRiskFn(0.0, .2);
    REQUIRE(liquidFn(.5)==Approx(-.5));

}
TEST_CASE("getLiquidityRiskFn returns correctly", "[CreditUtilities]"){
    REQUIRE(creditutilities::getLiquidityRiskFn(0.5, .2)(.5)==Approx(-0.4557602));
}

TEST_CASE("getlogLPMCF returns zero when u is zero", "[CreditUtilities]"){
    auto getDouble=[](const auto& v){
        return v;
    };
    auto getW=[](const auto& v, const int& index){
        return index;
    };
    auto tmplgdCF=[](const auto& u, const auto& l){
        return exp(u*l);
    };
    int n=10;
    int m=3;
    auto logCF=creditutilities::logLPMCF(m, tmplgdCF, getDouble, getDouble, getW);
    std::vector<double> loans(n, 1.5);
    auto result=logCF(0.0, loans);
    for(int i=0; i<m;++i){
        REQUIRE(result[i]==Approx(0.0));
    }
    
}
TEST_CASE("getlogLPMCF returns correctly", "[CreditUtilities]"){
    auto getDouble=[](const auto& v){
        return v;
    };
    auto getW=[](const auto& v, const int& index){
        return index+1;
    };
    auto tmplgdCF=[](const auto& u, const auto& l){
        return exp(u*l);
    };
    int n=10;
    int m=3;
    auto logCF=creditutilities::logLPMCF(m, tmplgdCF, getDouble, getDouble, getW);
    std::vector<double> loans(n, 1.5);
    auto result=logCF(.5, loans);
    std::vector<double> expected(3, 0);
    expected[0]=16.755;
    expected[1]=33.510;
    expected[2]=50.265; 
    for(int i=0; i<m;++i){
        REQUIRE(result[i]==Approx(expected[i]));
    }
}

TEST_CASE("getLGDCFFn returns 1 when u is 0", "[CreditUtilities]"){
    double lambda=.4;
    double theta=.3;
    double sigma=.3;
    double t=1.0;
    double x0=.6;

    auto lgdCF=creditutilities::getLGDCFFn(lambda, theta, sigma, t, x0);
    REQUIRE(lgdCF(0.0, 500.0)==Approx(1.0));
}
TEST_CASE("getLGDCFFn returns correctly", "[CreditUtilities]"){
    double lambda=.4;
    double theta=.3;
    double sigma=.3;
    double t=1.0;
    double x0=.6;

    auto lgdCF=creditutilities::getLGDCFFn(lambda, theta, sigma, t, x0);
    auto result=lgdCF(std::complex<double>(.5, .5), 500.0);
    /**these numbers are based off the results in "creditRisk" repo*/
    REQUIRE(result.real()==Approx(1.15727e-08));
    REQUIRE(result.imag()==Approx(6.31271e-09));
   // REQUIRE(lgdCF(0.0, 500.0)==Approx(0.0));
}
TEST_CASE("ensure logLPMCF returns correctly", "[CreditUtilities]"){
    auto testU=std::complex<double>(.5, .5);
        std::vector<loan> testLoans;
        std::vector<double> w(1);
        w[0]=1;
        int m=1;
        double alphL=.2;
        double bL=.5;
        double sigL=.3;
        double tau=1;
        testLoans.emplace_back(loan(.02, 5000.0, std::move(w)));
        auto result=creditutilities::logLPMCF( 
            m,
            creditutilities::getLGDCFFn(alphL, bL, sigL, tau, bL),
            [](const auto& v){return v.exposure;}, 
            [](const auto& v){return v.pd;}, 
            [](const auto& v, const auto& index){return v.w[index];}
        )(creditutilities::getLiquidityRiskFn(0.0, 0.0)(testU), testLoans);
        /**these numbers are based off the results in "creditRisk" repo*/
        REQUIRE(result[0].real()==Approx(-.02));
        REQUIRE(result[0].imag()==Approx(2.89279e-21));
}

TEST_CASE("ensure total inversion returns correctly", "[CreditUtilities]"){
    auto testU=std::complex<double>(.5, .5);
    double alphL=.2;
    double bL=.5;
    double sigL=.3;
    double tau=1;
    int m=1;
    int xUnits=8;
    int uUnits=8;
    std::vector<loan> testLoans;
    std::vector<double> w(1);
    w[0]=1;
    testLoans.emplace_back(loan(.02, 5000.0, std::move(w)));
    std::vector<std::vector<std::complex<double> > > tmpCf(uUnits, std::vector<std::complex<double> >(m, 0));
    auto tempxmin=0.0;
    auto tempxmax=1.0;
    std::vector<double> expectation(m, 0);
    expectation[0]=.546827;
    std::vector<std::vector<double> > variance(m, std::vector<double>(m, 0));
    variance[0][0]=.0258917;
    tmpCf=creditutilities::getFullCFFn(tempxmin, tempxmax,
        creditutilities::getLiquidityRiskFn(0.0, 0.0),
        creditutilities::logLPMCF( 
            m,
            creditutilities::getLGDCFFn(alphL, bL, sigL, tau, bL),
            [](const auto& v){return v.exposure;}, 
            [](const auto& v){return v.pd;}, 
            [](const auto& v, const auto& index){return v.w[index];}
        )
    )(
        std::move(tmpCf), 
        testLoans 
    );
    auto vasicekLogFN=vasicek::getLogVasicekMFGFn(std::move(expectation), std::move(variance));
    auto test=fangoost::computeInvDiscreteLog(xUnits, tempxmin, tempxmax, futilities::for_each_parallel(0, uUnits, [&](const auto& index){
        return vasicekLogFN(tmpCf[index]);
    }));
    /**these numbers are based off the results in "creditRisk" repo*/
    std::vector<double> expected(8, 0.0);
    expected[0]=14.8478;
    expected[1]=-.978256;
    expected[2]=1;
    expected[3]=-.978256;
    expected[4]=1;
    expected[5]=-.978256;
    expected[6]=1;
    expected[7]=-.978256;
    for(int i=0; i<8;++i){
        REQUIRE(test[i]==Approx(expected[i]));
    }
}