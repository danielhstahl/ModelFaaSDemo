#ifndef __CREDITUTILITIES_H_INCLUDED__
#define __CREDITUTILITIES_H_INCLUDED__

#include <cmath>
#include <vector>
#include <complex>
#include "FunctionalUtilities.h"
#include "FangOost.h"
#include <iostream>

/**This is not in its own repo because it is choices that must be made per implementation of each credit model*/
namespace creditutilities {
    
    template<typename Lambda, typename Q>
    auto getLiquidityRiskFn(Lambda&& lambdatmp, Q&& qtmp){
        return [lambda=std::move(lambdatmp), q=std::move(qtmp)](const auto& u){
            return -(exp(-u*lambda)-1.0)*q-u;
        };
    }

    template<typename Number, typename GetLiquidity, typename GetLogLPMCF>
    auto getFullCFFn(const Number& xMin, const Number& xMax, GetLiquidity&& getLiquiditytmp, GetLogLPMCF&& logLPMCFtmp){
        auto du=fangoost::computeDU(xMin, xMax);
        return [xMax,xMin, logLPMCF=std::move(logLPMCFtmp), getLiquidity=std::move(getLiquiditytmp), du](auto&& cf, auto&& loans){ 
            /**Note that val+!!!  This is so the cf can be recursively built from multiple runs over loans*/
            return futilities::for_each_parallel(std::move(cf), [&](const auto& val, const auto& iterateU){
                return futilities::for_each_parallel(
                    logLPMCF(
                        getLiquidity(
                            fangoost::getComplexU(fangoost::getU(du, iterateU))
                        ),
                        loans
                    ), 
                    [&](const auto& result, const auto& iterateY){
                        return val[iterateY]+result;
                    }
                );
            });
        };
    }

    /** This function returns the exponent for the credit risk characteristic function: namely sum_j p_j Y (phi_j(u)-1).  Note that to do liquidity risk, pass "getLiquidityRisk"'s results to "u". */
    template<typename GetL, typename GetW, typename GetPD, typename LGDCF>
    auto logLPMCF(int m, LGDCF&& lgdCFtmp, GetL&& getLtmp, GetPD&& getPDtmp, GetW&& getWtmp){
        return [
            lgdCF=std::move(lgdCFtmp), 
            getL=std::move(getLtmp), 
            getPD=std::move(getPDtmp), 
            getW=std::move(getWtmp), 
            m
        ](const auto &u, const auto& loans){
            return futilities::for_each_parallel(0, m, [&](const auto& indexM){
                return futilities::sum(loans, [&](const auto& loan, const auto& index){
                    return (lgdCF(u, getL(loan))-1.0)*getPD(loan)*getW(loan, indexM);
                });
            });
        };
    }

    /**Characteristic function for LGD.  Follows CIR process.  U is typically complex.  Pass by value to enable multiple lambdas*/
    template<typename Lambda, typename Theta, typename Sigma, typename T, typename X0>
    auto getLGDCFFn(const Lambda& lambda, Theta&& thetatmp, const Sigma& sigma,  const T& t, X0&& x0tmp){
        auto expt=exp(-lambda*t);
        auto sigL=-(sigma*sigma)/(2*lambda);
        return [expt, sigL, theta=std::move(thetatmp), x0=std::move(x0tmp)](const auto& u, const auto& l){
            auto uu=u*l;
            auto uP=uu*(1-expt)*sigL+1.0;
            return exp((uu*expt*x0)/uP)*pow(uP, theta/sigL);
        };
    }
}

#endif
