#ifndef TauAnalysis_ClassicSVfit_SVfitIntegratorMarkovChain_h
#define TauAnalysis_ClassicSVfit_SVfitIntegratorMarkovChain_h

/** \class SVfitIntegratorMarkovChain
 *
 * Generic class to perform Markov Chain type integration
 * in N-dimensional space.
 *
 * The code is implemented following the description in:
 *  [1] "Probabilistic Inference Using Markov Chain Monte Carlo Methods",
 *      R. Neal, http://www.cs.toronto.edu/pub/radford/review.pdf
 *  [2] "Bayesian Training of Backpropagation Networks by the Hybrid Monte Carlo Method",
 *      R. Neal, http://www.cs.toronto.edu/pub/radford/bbp.ps
 *
 * \author Christian Veelken, NICPB Tallinn
 *
 */

#include <Math/Functor.h>
#include <TRandom3.h>
#include <TFile.h>
#include <TTree.h>

#include <functional>
#include <vector>
#include <string>
#include <iostream>

namespace classic_svFit
{
  class SVfitIntegratorMarkovChain
  {
   public:
    SVfitIntegratorMarkovChain(const std::string&, unsigned, unsigned, unsigned, unsigned, double, double, unsigned, unsigned, double, double, const std::string&, int = 0);
    ~SVfitIntegratorMarkovChain();

    /// set initial position of Markov Chain in N-dimensional space to given values,
    /// in order to start path of chain transitions from non-random point
    void initializeStartPosition_and_Momentum(const double*);

    /// register "call-back" functions:
    /// A user may register any number of "call-back" functions,
    /// which are evaluated in every iteration of the Markov Chain.
    /// The mechanism allows to compute expectation values
    /// of any number of observables, according to eq. (12) in [2].
    /// The evaluation of "call-back" functions proceeds
    /// by calling ROOT::Math::Functor::operator(x).
    /// The argument x is a vector of dimensionality N,
    /// represent the current position q of the Markov Chain in the
    /// N-dimensional space in which the integration is performed.
    void registerCallBackFunction(const ROOT::Math::Functor&);

    /// compute integral of function g
    /// the points xl and xh represent the lower left and upper right corner of a Hypercube in d-dimensional integration space
    using gPtr_C = std::function<double(const double*, size_t, void*)>;
    //typedef double (*gPtr_C)(const double*, size_t, void*);
    void integrate(gPtr_C g, const double* xl, const double* xu, unsigned d, double& integral, double& integralErr);

    void print(std::ostream&) const;

  protected:
    void setIntegrand(gPtr_C, const double*, const double*, unsigned);

    void initializeStartPosition_and_Momentum();

    void makeStochasticMove(unsigned, bool&, bool&);

    void sampleSphericallyRandom();

    void updateX(const std::vector<double>&);

    double evalProb(const std::vector<double>&);

    gPtr_C integrand_;

    /// parameters defining integration region
    ///  numDimensions: dimensionality of integration region (Hypercube)
    ///  xMin:          lower boundaries of integration region
    ///  xMax:          upper boundaries of integration region
    ///  initMode:      flag indicating how initial position of Markov Chain is chosen (uniform/Gaus distribution)
    unsigned numDimensions_;
    double* x_;
    std::vector<double> xMin_; // index = dimension
    std::vector<double> xMax_; // index = dimension
    int initMode_;

    /// parameters defining number of "stochastic moves" performed per integration
    ///  numIterBurnin:   number of "stochastic moves" performed to reach "ergodic" state of Markov Chain
    ///                  ("burnin" iterations do not enter the integral computation;
    ///                   their purpose is to make the computed integral value ~independent
    ///                   of the initial position at which the Markov Chain is started)
    ///  numIterSampling: number of "stochastic moves" used to compute integral
    unsigned numIterBurnin_;
    unsigned numIterSampling_;

    // maximum number of attempts to find a valid starting-position for the Markov Chain
    // (i.e. an initial point of non-zero probability)
    unsigned maxCallsStartingPos_;

    /// parameters defining "simulated annealing" stage at beginning of integration
    ///  simAnnealingAlpha: number of "stochastic moves" performed at high temperature during "burnin" stage
    ///  T0:                initial annealing temperature
    ///  alpha:             speed parameter with which temperature decreases (~alpha^iteration)
    unsigned numIterSimAnnealingPhase1_;
    unsigned numIterSimAnnealingPhase2_;
    unsigned numIterSimAnnealingPhase1plus2_;
    double T0_;
    double sqrtT0_;
    double alpha_;
    double alpha2_;

    /// number of Markov Chains run in parallel
    unsigned numChains_;

    /// number of iterations per batch
    /// (used for estimation of uncertainty on computed integral value,
    ///  according to eqs. (6.39) and (6.40) in [1])
    unsigned numBatches_;

    /// parameters defining step-sizes of Metropolis moves:
    ///  epsilon0: average step-size
    ///  nu:       variation of step-size for individual moves
    typedef std::vector<double> vdouble;
    double epsilon0_;
    vdouble epsilon0s_;
    vdouble epsilon_;
    double nu_;

    /// random number generator
    TRandom3 rnd_;

    /// internal variables storing current state of Markov Chain
    vdouble p_;
    vdouble q_;
    vdouble gradE_;
    double prob_;

    /// temporary variables used for computations
    vdouble u_;
    vdouble pProposal_;
    vdouble qProposal_;

    vdouble probSum_; // index = chain*numBatches + batch
    vdouble integral_;

    long numMoves_accepted_;
    long numMoves_rejected_;

    unsigned numChainsRun_;

    long numIntegrationCalls_;
    long numMovesTotal_accepted_;
    long numMovesTotal_rejected_;

    int errorFlag_;

    std::vector<const ROOT::Math::Functor*> callBackFunctions_;

    std::string treeFileName_;
    TFile* treeFile_;
    TTree* tree_;
    int treeMove_;
    float treeIntegrand_;

    int verbosity_; // flag to enable/disable debug output
  };
}

#endif
