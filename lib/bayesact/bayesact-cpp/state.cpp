#ifdef _PYTHON
#include <Python.h>
#endif

#include <cstdlib>
#include <vector>
#include <algorithm>

#include <Eigen/Dense>

#include "values.h"

#include "state.h"

#define SINGLE_RANDOM false

using namespace bayesact;

template <class X_t>
State<X_t>::State()
{
  this->f_ = YVector::Zero();
  this->tau_ = YVector::Zero();
  this->x_ = X_t();
  this->weight_ = 1.0;
}

template <class X_t>
State<X_t>::State(YVector f, YVector tau, X_t x, double weight)
{
  (void)static_cast<XVar*>((X_t*)0); // Hacky solution to make sure X_t
                                     //   is a child of XVar
  this->f_ = f;
  this->tau_ = tau;
  this->x_ = x;
  this->weight_ = weight; 
}

template <class X_t>
State<X_t>::State(const State<X_t>& other)
{
  this->f_ = other.f_;
  this->tau_ = other.tau_;
  this->x_ = other.x_;
  this->weight_ = other.weight_; 
}

template <class X_t>
State<X_t>&
State<X_t>::operator=(const State<X_t> &rhs)
{
  this->f_ = rhs.f_;
  this->tau_ = rhs.tau_;
  this->x_ = rhs.x_;
  this->weight_ = rhs.weight_; 

  return *this;
}

template <class X_t>
State<X_t>&
State<X_t>::operator+=(const State<X_t> &rhs)
{
  for (int i = 0; i < Y_SIZE; i++) {
    this->f_[i]   += rhs.f_[i];
    this->tau_[i] += rhs.tau_[i];
  }
  this->x_ += rhs.x_;
  this->weight_ += rhs.weight_;

  return *this;
}

template <class X_t>
State<X_t>&
State<X_t>::operator-=(const State<X_t> &rhs)
{
  for (int i = 0; i < Y_SIZE; i++) {
    this->f_[i]   -= rhs.f_[i];
    this->tau_[i] -= rhs.tau_[i];
  }
  this->x_      -= rhs.x_;
  this->weight_ -= rhs.weight_;

  return *this;
}

template <class X_t>
State<X_t>&
State<X_t>::operator*=(const State<X_t> &rhs)
{
  for (int i = 0; i < Y_SIZE; i++) {
    this->f_[i]   *= rhs.f_[i];
    this->tau_[i] *= rhs.tau_[i];
  }
  this->x_      *= rhs.x_;
  this->weight_ *= rhs.weight_;

  return *this;
}

template <class X_t>
State<X_t>&
State<X_t>::operator/=(const State<X_t> &rhs)
{
  for (int i = 0; i < Y_SIZE; i++) {
    this->f_[i]   /= rhs.f_[i];
    this->tau_[i] /= rhs.tau_[i];
  }
  this->x_      /= rhs.x_;
  this->weight_ /= rhs.weight_;

  return *this;
}

template <class X_t>
State<X_t>&
State<X_t>::operator*=(const double &rhs)
{
  this->f_      *= rhs;
  this->tau_    *= rhs;
  this->x_      *= rhs;
  this->weight_ *= rhs;
}

template <class X_t>
State<X_t>&
State<X_t>::operator/=(const double &rhs)
{
  *this *= (1.0 / rhs);
}

template <class X_t>
const State<X_t>
State<X_t>::operator+ (const State<X_t> &other)
{
  return State<X_t>(*this) += other;
}

template <class X_t>
const State<X_t>
State<X_t>::operator- (const State<X_t> &other)
{
  return State<X_t>(*this) -= other;
}


template <class X_t>
const State<X_t> 
State<X_t>::operator* (const State<X_t> &other)
{
  return State<X_t>(*this) *= other;
}

template <class X_t>
const State<X_t> 
State<X_t>::operator/ (const State<X_t> &other)
{
  return State<X_t>(*this) /= other;
}

template <class X_t>
const State<X_t> 
State<X_t>::operator* (const double &other)
{
  return State<X_t>(*this) *= other;
}

template <class X_t>
const State<X_t> 
State<X_t>::operator/ (const double &other)
{
  return State<X_t>(*this) /= other;
}


template <class X_t>
void
State<X_t>::roughen(YVector noiseVector)
{
  for (int i = 0; i < Y_SIZE; i++) {
    // Gets a random double in the range [0,1)
    const double randomDouble = (double)(rand()) / (double)(RAND_MAX);
    const double randomRoughness = (2 * randomDouble) - 1;
    this->f_[i] += randomRoughness * noiseVector[i];
  }
}

template <class X_t>
std::vector<State<X_t>*> 
State<X_t>::computeUnweightedSamples (std::vector<State<X_t>*> samples, int nNewSamples)
{
  unsigned nSamples = samples.size();

  if (nNewSamples == 0)
    nNewSamples = nSamples;

  std::vector<double> cumulativeWeights (nSamples, 0.0);
  std::vector<double> relativeCumulativeWeights (nSamples, 0.0);

  cumulativeWeights[0] = samples[0]->getWeight();
  for (int i = 1; i < nSamples; i++) {
    cumulativeWeights[i] = cumulativeWeights[i-1] + samples[i]->getWeight();
  }

  double const weightSum = cumulativeWeights[nSamples-1];
  for (int i = 1; i < nSamples; i++) {
    relativeCumulativeWeights[i] = cumulativeWeights[i]/weightSum;
  }

  std::vector<double> randomSampleNumber (nNewSamples,0);

  #if SINGLE_RANDOM
  const double inverseNNewSamples = 1.0 / ((double)nNewSamples);
  const double randomDouble = (double)(rand()) / (double)(RAND_MAX);
  randomSampleNumber[0] = randomDouble * inverseNNewSamples;
  for (int i = 1; i < nNewSamples; i++) {
    randomSampleNumber[i] = randomSampleNumber[i-1] + inverseNNewSamples;

  }
  #else
  for (int i = 0; i < nNewSamples; i++) {
    const double randomDouble = (double)(rand()) / (double)(RAND_MAX);
    randomSampleNumber[i] = randomDouble;
  }
  std::sort(randomSampleNumber.begin(), randomSampleNumber.end());
  #endif

  unsigned j = 0;
  std::vector<State<X_t>*> newSamples (nNewSamples, NULL);

  for (int i = 0; i < nNewSamples; i++) {
    while (relativeCumulativeWeights[j] < randomSampleNumber[i]) 
      j++;

    newSamples[i] = new State<X_t> (*(samples[j]));

//    newSamples.at(i) = samples[j];
    newSamples[i]->weight_ = 1.0;
  }

  return newSamples;
}

template <class X_t>
Actor
State<X_t>::getTurn()
{
  return this->x_.getTurn();
}

template <class X_t>
double
State<X_t>::computeDeflection()
{
  YVector diff = this->tau_ - this->f_;

  return diff.dot(diff);
}

template <class X_t>
State<X_t>
State<X_t>::multiplyByWeight()
{
  State<X_t> retval (*this);

  for (unsigned i = 0; i < Y_SIZE; i++) {
    retval.f_[i] *= this->weight_;
    retval.tau_[i] *= this->weight_;
  }

  retval.x_ *= this->weight_;
  retval.weight_ = 1.0; // This has the advantage of not breaking things
                       // if this function were to accidentally be called
                       // repeatedly.
  return retval;
}

template <class X_t>
YVector
State<X_t>::computeRandom_()
{
  YVector retval;

  for (unsigned i = 0; i < Y_SIZE; i++) {
    const double randomDouble = (double)(rand()) / (double)(RAND_MAX);

    retval(i) = randomDouble * 8.6 - 4.3;
  }

  return retval;
}

template <class X_t>
State<X_t>*
State<X_t>::random()
{
  YVector f = computeRandom_();
  YVector tau = computeRandom_();
  X_t x = X_t();
  double weight = 1;

  return new State<X_t> (f, tau, x, weight);
}

#include "xvar.h"
template class State<XVar>; // This is done so that State<XVar> will be
                            //   accessible from a shared library.
