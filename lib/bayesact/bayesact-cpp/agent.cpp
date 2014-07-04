#ifdef _PYTHON
#include <Python.h>
#endif

#include <cmath>

#include <vector>
#include <map>
#include <string>

#include <Eigen/Core>
#include <Eigen/LU>

#include "values.h"

#include "state.h"

#include "agent.h"

#include "multivariatenormal.h"

using namespace bayesact;

//this function builds the instances of \mathscr{H} and \mathsrc{C} from tau for a particular turn
//essentially, this constructs \mathscr{G} but without any "b" terms (no behaviours - replacing any elements of \mathscr{G} 
//that have only behaviours with 1), replicates it row-wise 9 times to give a 9x29 matrix, and mutliplies 
//this point-wise by the transpose of the transition matrix, M^T (another 9x29 matrix).  
//It then divides the resulting matrix into four blocks corresponding to the elements of the original \mathsrc{G} 
//that contain no, 'e', 'p' and 'a' behaviours (none,Tbe,Tbp,Tba).  
//It then sums each of these blocks to give a 9x4 matrix, the first column of which is \mathsrc{C} and
//the last three columns of which are \mathsrc{H}
//inputs are 
// - tau (the current transient sentiment vector)
// - H the rows of the transition matrix M that correspond to values of \mathscr{G} with Tb<i> values i=e,p, or a
//     H[turn]['Tbe'] gives those rows that correspond to values of \mathsrc{G} that contain 'Tbe'
// - C the rows of the transition matrix M that correspond to values of \mathscr{G} with no Tb values at all
//H[turn]['Tbi'] and C[turn] are both lists of tuples, each tuple giving: first, a list of strings like 'Tbi' 
template <class X_t>
YVector*
Agent<X_t>::sampleFVar_(YVector fundamentals, YVector transients,
                        double* weight, TauMappings::H* h, TauMappings::C* c, // OUTPUT
                        Actor turn, EpaVector* aab, EpaVector* observ, bool reset)
{
  if (reset)
    turn = CLIENT;

  //now we will insert aab into f here
  //this seems like a hack, but in fact it is only a small one
  //we are "using" the "b" slot in f to hold what the agent action is
  //and when turn=="agent", the variance on b is tiny, so forces 
  //the samples to have a Fb-value which is the same as aab
  //here, we could also replace f[0:3] with aab if aab is 6-D :
  //f=NP.concatenate((aab,f[6:9]))
  //this would happen if the agent can also set its own identity with an action
  //the isigf would also need to be modified to reflect this
  //see in the constructor the comment about this - search for AAB6D
  if (turn == AGENT && aab != NULL) {
    for (unsigned i = 0; i < EPA_DIMENSIONS; i++) {
      const unsigned fIndex = EPA_DIMENSIONS * BEHAVIOUR_INDEX + i;
      fundamentals[fIndex] = (*aab)[i];
    }
  }

  //special case - observation is used as f_b directly if there was a reset
  if (reset && observ != NULL) {
    for (unsigned i = 0; i < EPA_DIMENSIONS; i++) {
      const unsigned fIndex = EPA_DIMENSIONS * BEHAVIOUR_INDEX + i;
      fundamentals[fIndex] = (*observ)[i];
    }
  }

  this->tauMappings_->computeHC(transients, h, c);

  // k = Identity Matrix - Transpose(Verticle Stack ( Zeros, h, Zeros ))
  // Note that Zeros and h are all 3x9 so k is 9x9
  Eigen::Matrix<double, Y_SIZE, Y_SIZE> k = Eigen::Matrix<double,9,9>::Identity();
  for (unsigned i = 0; i < Y_SIZE; i++) { //TODO: Verify correctness
    YVector y = YVector::Zero();
    for (unsigned j = 0; j < EPA_DIMENSIONS; j++) {
      y[EPA_DIMENSIONS + j] = (*h)(i, j);
    }
    k.row(i) -= y;
  }

  Eigen::Matrix<double, 9, 9> ka, meanPrediction, sigN;

  ka = k * this->iSigA_;
  meanPrediction = ka * k;
  sigN = (meanPrediction + this->iSigF_).inverse();

  YVector tmp1 = ka * (*c);
  YVector tmp2 = this->iSigF_ * fundamentals;

  YVector meanValue = sigN * (tmp1 + tmp2);

  *weight = 1.0;
  YVector fSample = computeMultivariateNormal<Y_SIZE> (meanValue, sigN);

  return new YVector(fSample);
}

template<class X_t>
double
Agent<X_t>::computeLogProbDistFunc_(EpaVector x, double mean, double iVar, double lDenom)
{
  double num = 0;

  for (unsigned i = 0; i < EPA_DIMENSIONS; i++) {
    num -= iVar * pow(x[i] - mean,2);
  }

  return num - lDenom;
}

template<class X_t>
YVector*
Agent<X_t>::sampleTVar_(TauMappings::H h, TauMappings::C c, YVector* fSample)
{
  EpaVector fBehaviour = getEpa(*fSample, BEHAVIOUR_INDEX);

  YVector* retval = new YVector();
  *retval = h * fBehaviour + c; //TODO: Verify correctness

  return retval;
}

template<class X_t>
double 
Agent<X_t>::computeSampleFVarWeight_(State<X_t>* state, double iVar, double lDenom, 
                                Actor turn, EpaVector* observ)
{
  double weight = 1.0;

  if (observ != NULL && turn == CLIENT) {
    EpaVector fundBehaviour = getEpa(state->getF(), BEHAVIOUR_INDEX);
    EpaVector dvo = fundBehaviour - (*observ);
    weight = computeLogProbDistFunc_(dvo, 0.0, iVar, lDenom);
  }

  return weight;
}

template<class X_t>
double
Agent<X_t>::computeSampleXVarWeightExp_(State<X_t>* sample, X_t* xObserved)
{
  if (xObserved == NULL) return 1;

  if (sample->getX() == (*xObserved))
    return 1;
  else
    return 0;
}

template<class X_t>
Agent<X_t>::Agent(TauMappings* tauMappings, Settings* settings)
{
  this->init_(tauMappings, settings);
}

template <class X_t>
void
Agent<X_t>::init_(TauMappings* tauMappings, Settings* settings)
{
  if (settings != NULL)
    this->settings_ = settings;
  else
    this->settings_ = new Settings();

  this->tauMappings_ = tauMappings;
  
  const double gamma = abs(this->settings_->getValue(ENVIRONMENT_NOISE));
  double x = 1/(gamma*gamma);
  this->iVar_ = x * 0.5;

  const double c = 2.7568155996140180; // log( (2 * pi) ^ 1.5 ) 
  this->lDenom_ = 3 * log(gamma) + c; // Mathematically equivalent to:
                                      // log ((2pi)^1.5 * gamma ^ 3)

  const double ALPHA = this->settings_->getValue(AFFECT_CONTROL_PRINCIPLE_STRENGTH);
  const double BETA  = this->settings_->getValue(AGENT_IDENTITY_INERTIA);
  this->sigA_  = Eigen::Matrix<double,9,9>::Identity() * (ALPHA * ALPHA);
  this->iSigA_ = Eigen::Matrix<double,9,9>::Identity() / (ALPHA * ALPHA);
  this->sigF_  = Eigen::Matrix<double,9,9>::Identity() * (BETA * BETA);
  this->iSigF_ = Eigen::Matrix<double,9,9>::Identity() / (BETA * BETA);
  // TODO: Enchance sigF (and iSigF) to actually include setting values;

  const unsigned N = this->settings_->getValue(NUMBER_OF_SAMPLES);

  for (unsigned i = 0; i < N; i++) {
    this->samples.push_back(State<X_t>::random()); 
    //TODO: Stop using purely random starting point
  }
}

template <class X_t>
State<X_t>*
Agent<X_t>::sampleNext(State<X_t>* state, EpaVector* aab, EpaVector* observ, 
                  EpaVector* paab, bool reset)
{
  Actor turn = state->getTurn();

  YVector* fSample;
  YVector* tauSample;
  X_t xSample;
  double weight;
  TauMappings::H h;
  TauMappings::C c;

  fSample = this->sampleFVar_(state->getF(), state->getTau(), &weight, &h, &c,
                              state->getTurn(), NULL, NULL, reset); // OUTPUT
  tauSample = this->sampleTVar_(h, c, fSample);
  xSample = state->getX();

  State<X_t>* retval = new State<X_t> (*fSample, *tauSample, xSample, weight);

  delete fSample;
  delete tauSample;

  return retval;
}

template <class X_t>
void
Agent<X_t>::propagateForward (EpaVector* aab, EpaVector* observ, 
                              X_t* xObserved, EpaVector* paab,
                              bool verb)
{
  std::vector<State<X_t>*> newSamples, newUnweightedSamples;
  newUnweightedSamples = State<X_t>::computeUnweightedSamples (this->samples);

  this->roughenSamples_(newUnweightedSamples);

  double totalWeight = 0.0;

  while (totalWeight < EPSILON) {
    this->samples.clear();
    totalWeight = 0.0;
    bool reset = false;

    for (unsigned i = 0; i < newUnweightedSamples.size(); i++) {
      State<X_t>* newSample = this->sampleNext(newUnweightedSamples[i], 
                                               aab, observ, paab, reset);
      double weight;
      weight = this->computeSampleFVarWeight_(newSample, this->iVar_,
                                              this->lDenom_,
                                            newSample->getTurn(), observ);
      double exponent = newSample->getWeight() + weight;
      double newWeight = this->computeSampleXVarWeightExp_(newSample, xObserved);

      newSample->setWeight(exp(exponent) * newWeight);

      totalWeight += newSample->getWeight();

      this->samples.push_back(newSample);

      reset = true;
    }
  }

  State<X_t>* averageState = computeSampleMean_(this->samples);

  this->averageDeflection_ = averageState->computeDeflection();
  this->averageX_ = averageState->getX();
}

template <class X_t>
void
Agent<X_t>::roughenSamples_ (std::vector<State<X_t>*> samples)
{
  YVector noiseVector = this->computeNoiseVector_();

  for (int i = 0; i < Y_SIZE; i++) {
    samples[i]->roughen(noiseVector);
  }
}

template <class X_t>
State<X_t>*
Agent<X_t>::computeSampleMean_(std::vector<State<X_t>*> samples)
{
  State<X_t>* retval = new State<X_t>();

  for (unsigned i = 0; i < samples.size(); i++) {
    *retval += samples[i]->multiplyByWeight();
  }
  *retval /= retval->getWeight();

  return retval;
}

template <class X_t>
YVector
Agent<X_t>::computeNoiseVector_()
{
  YVector noiseVector = YVector::Zero();

  for (int i = 0; i < Y_SIZE; i++) {
    if (i < EPA_DIMENSIONS)
      noiseVector(i) = this->settings_->getValue(AGENT_ROUGH);
    else if (i < 2*EPA_DIMENSIONS)
      noiseVector(i) = 1.0;
    else if (i < 3*EPA_DIMENSIONS)
      noiseVector(i) = this->settings_->getValue(CLIENT_ROUGH);

    if (noiseVector(i) == 0.0)
      noiseVector(i) = 1.0;
  }

  return noiseVector;
}

#include "xvar.h"

template class Agent<XVar>;
