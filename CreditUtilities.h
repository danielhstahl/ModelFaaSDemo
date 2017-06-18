#ifndef __CREDITUTILITIES_H_INCLUDED__
#define __CREDITUTILITIES_H_INCLUDED__

#include <cmath>
#include <vector>
#include <complex>
#include "FunctionalUtilities.h"
#include "FangOost.h"

/**This is not in its own repo because it is choices that must be made per implementation of each credit model*/
namespace creditutilities {
    
    template<typename Number, typename Lambda, typename Q>
    auto getLiquidityRiskFn(const Lambda& lambda, const Q& q){
        return [&lambda, &q](const auto& u){
            return -(exp(-u*lambda)-1.0)*q-u;
        };
    }

    template<typename Number, typename GetVasicekMGF, typename GetLiquidity, typename GetLogLPMCF>
    auto getFullCFFn(const Number& xMin, const Number& xMax, const GetVasicekMGF& getVasicekMGF, const GetLiquidity& getLiquidity, const GetLogLPMCF& logLPMCF){
        auto du=fangoost::computeDU(xMin, xMax);
        auto cp=fangoost::computeCP(du);
        return [&du, &cp, &xMin, &getVasicekMGF, &logLPMCF, &getLiquidity](auto&& cf, const auto& loans){
            return futilities::for_each_parallel(cf, [&](const auto& val, const auto& index){
                return fangoost::formatCF(fangoost::getComplexU(fangoost::getU(du, index)), xMin, cp, [&](const auto u){
                    return getVasicekMGF(
                        logLPMCF(
                            getLiquidity(u)
                        )
                    );
                });
            });
        };
    }

    /** This function returns the exponent for the credit risk characteristic function: namely sum_j p_j Y (phi_j(u)-1).  Note that to do liquidity risk, pass "getLiquidityRisk"'s results to "u". */
    template<typename Loan, typename Incr, typename GetL, typename GetW, typename GetPD, typename LGDCF>
    auto logLPMCF(const LGDCF& lgdCF, const GetL& getL, const GetPD& getPD, const GetW& getW){
        return [&lgdCF, &getL, &getPD, &getW](const auto &u, const auto& loans, const auto& m){//
            return futilities::for_each_parallel(0, m, [&](const auto& indexM){
                return futilities::sum(loans, [&](const auto& loan, const auto& index){
                    return (lgdCF(u, getL(loan))-1.0)*getPD(loan)*getW(loan, indexM);
                });
            });
        };
    }

    /**Characteristic function for LGD.  Follows CIR process.  U is typically complex*/
    template<typename Number, typename LoanExposure, typename Lambda, typename Theta, typename Sigma, typename T, typename X0>
    auto getLGDCFFn(const Lambda &lambda,const Theta &theta, const Sigma &sigma, const T &t, const X0 &x0){
        auto expt=exp(-lambda*t);
        auto sigL=-sigma*sigma/(2*lambda);
        return [&expt, &sigL, &theta, &x0](const auto& u, const auto& l){
            auto uu=u*l;
            auto uP=uu*(1-expt)*sigL+1.0;
            exp((uu*expt*x0)/uP)*pow(uP, theta/sigL);
        };
    }

    template<typename Alpha, typename Tau>
    auto helpComputeMoments(const Alpha& alpha, const Tau& tau){ //hleper function since called so much
        return (1-exp(-alpha*tau))/alpha;
    }
    template<typename Rho, typename Sigma, typename Alpha>
    auto crossMultiply(const Rho& rho, const Sigma& sigma1, const Sigma& sigma2, const Alpha& alpha1, const Alpha& alpha2){
        return (rho*sigma1*sigma2)/(alpha1*alpha2);
    }

    /**
    Computes the expectation of the integral of a multivariate vasicek process with mean 1
    */
    template<typename Number, typename Alpha, typename Tau>
    auto computeExpectationVasicek(const std::vector<Number>& y0, const std::vector<Alpha>& alpha, const Tau& tau){
        return futilities::for_each_parallel_copy(y0, [&](const auto& val, const auto& index){
            return (val-1)*helpComputeMoments(alpha[index], tau);
        });
    }
    /**
    Computes the variance/covariance of the integral of a multivariate vasicek process
    */
    template<typename Alpha, typename Tau, typename Sigma, typename Rho>
    auto computeVarianceVasicek(const std::vector<Alpha>& alpha, const std::vector<Sigma>& sigma, const std::vector<std::vector<Rho> >& rho,  const Tau& tau){
        int rowLength=alpha.size();
        return futilities::for_each_parallel(0, rowLength, [&](const auto& indexI){
            auto ai=helpComputeMoments(alpha[indexI], tau);
            return futilities::for_each_parallel(0, rowLength, [&](const auto& indexJ){
                auto aj=helpComputeMoments(alpha[indexJ], tau);
                return crossMultiply(rho[indexI][indexJ], sigma[indexI], sigma[indexJ], alpha[indexI], alpha[indexJ])*(tau-ai-aj+helpComputeMoments(alpha[indexI]+alpha[indexJ], tau));
            });
        });
    }
    /**Computes the expectation of a the exponential of a weighted combination of the multidemensional integrated vasicek process*/
    template<typename Expectation, typename Variance, typename Number>
    auto getVasicekMFGFn(const std::vector<Expectation>& expectation , const std::vector< std::vector<Variance> >& variance){
        int m=expectation.size();
        return [m, &expectation, &variance](const auto& v){ //ocnst std::vector<std::complex<Number> > &v, 
            return exp(futilities::sum(0, m, [&](const auto& index){
                return v[index]*expectation[index];
            })+futilities::sum(0, m, [&](const auto& indexI){
                return futilities::sum(0, m, [&](const auto& indexJ){
                    return v[indexI]*v[indexJ]*variance[indexI][indexJ];
                });
            })*.5);
        };
    }
}

#endif
