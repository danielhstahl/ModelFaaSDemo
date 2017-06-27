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
    auto getLiquidityRiskFn(const Lambda& lambda, const Q& q){
        return [&lambda, &q](const auto& u){
            return -(exp(-u*lambda)-1.0)*q-u;
        };
    }

    template<typename Number, typename GetLiquidity, typename GetLogLPMCF>
    auto getFullCFFn(const Number& xMin, const Number& xMax, const GetLiquidity& getLiquidity, const GetLogLPMCF& logLPMCF){
        auto du=fangoost::computeDU(xMin, xMax);
        return [xMax,xMin, &logLPMCF, &getLiquidity, du](auto&& cf, const auto& loans){ 
            /**Note that val+!!!  This is so the cf can be recursively built from multiple runs over loans*/
            return futilities::for_each_parallel(cf, [&](const auto& val, const auto& iterateU){
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
    auto logLPMCF(int m, const LGDCF& lgdCF, const GetL& getL, const GetPD& getPD, const GetW& getW){
        return [&lgdCF, &getL, &getPD, &getW, m](const auto &u, const auto& loans){
            return futilities::for_each_parallel(0, m, [&](const auto& indexM){
                return futilities::sum(loans, [&](const auto& loan, const auto& index){
                    return (lgdCF(u, getL(loan))-1.0)*getPD(loan)*getW(loan, indexM);
                });
            });
        };
    }

    /**Characteristic function for LGD.  Follows CIR process.  U is typically complex.  Pass by value to enable multiple lambdas*/
    template<typename Lambda, typename Theta, typename Sigma, typename T, typename X0>
    auto getLGDCFFn(const Lambda& lambda, const Theta& theta, const Sigma& sigma,  const T& t, const X0& x0){
        auto expt=exp(-lambda*t);
        auto sigL=-(sigma*sigma)/(2*lambda);
        return [expt, sigL,&theta, &x0](const auto& u, const auto& l){
            auto uu=u*l;
            auto uP=uu*(1-expt)*sigL+1.0;
            return exp((uu*expt*x0)/uP)*pow(uP, theta/sigL);
        };
    }
}

#endif
