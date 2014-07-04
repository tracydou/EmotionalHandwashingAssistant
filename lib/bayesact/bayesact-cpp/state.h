#ifndef _STATE_H
#define _STATE_H

#include <vector>
#include <Eigen/Dense>

#include "xvar.h"

#include "values.h"

namespace bayesact
{
  template <class X_t> // Note X_t should be a child of XVar
  class State{
    public:
      State();
      State(YVector f, YVector tau, X_t x, double weight);
      State(const State& other);
      //~State();
 
      State& operator=(const State &rhs);
 
      State& operator+=(const State &rhs);
      State& operator-=(const State &rhs);
      State& operator*=(const State &rhs);
      State& operator/=(const State &rhs);

      State& operator*=(const double &rhs);
      State& operator/=(const double &rhs);

      const State operator+(const State &other);
      const State operator-(const State &other);
      const State operator*(const State &other);
      const State operator/(const State &other);

      const State operator*(const double &other);
      const State operator/(const double &other);

      void roughen(YVector noiseVector);
      State<X_t> multiplyByWeight();

      static std::vector<State*> computeUnweightedSamples (std::vector<State*> samples, int nNewSamples = 0);

      double computeDeflection();

      YVector getF() {return this->f_;}
      YVector getTau() {return this->tau_;}
      X_t getX() {return this->x_;}
      double getWeight() {return this->weight_;}

      Actor getTurn();

      void setWeight(double value) {this->weight_ = value;}

      static State<X_t>* random();

    private:
      YVector f_;
      YVector tau_;
      X_t x_;
      double weight_;

      static YVector computeRandom_();
  };
}
#endif // _STATE_H
