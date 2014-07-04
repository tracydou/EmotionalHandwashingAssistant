#ifndef _AGENT_H
#define _AGENT_H

#include <vector>
#include <Eigen/Dense>
#include <map>
#include <string>

#ifdef _PYTHON
#include <Python.h>
#endif

#include "values.h"

#include "settings.h"
#include "taumappings.h"

#include "state.h"

namespace bayesact
{
  #define EPSILON 0.000000001 

  template <class X_t>
  class Agent 
  {
    public:
      Agent(TauMappings* tauMappings, Settings* settings = NULL);

      // Parameters will be given proper types through development of this function
      void propagateForward (EpaVector* aab, EpaVector* observ, 
                             X_t* xobserv = NULL, EpaVector* paab = NULL,
                             bool verb = false);

      State<X_t>* sampleNext(State<X_t>* state, EpaVector* aab, EpaVector* observ, 
                             EpaVector* paab = NULL, bool reset = false);

/*
      State<X_t>* sampleNext(PyObject* state, PyObject* aab, PyObject* observ, 
                             PyObject* paab = NULL, bool reset = false);
*/

      #ifdef _PYTHON
      Agent(std::string filename, PyObject* settings = NULL);
      void propagateForward_python (PyObject* aab_p, PyObject* observ_p, 
                            PyObject* xobserv_p = NULL, PyObject* paab_p = NULL,
                            bool verb = false);
      #endif
      std::vector<State<X_t>*> samples;

    protected:
      Settings* settings_;

      
      TauMappings* tauMappings_;

      Eigen::Matrix<double,9,9> sigA_;
      Eigen::Matrix<double,9,9> iSigA_;
      Eigen::Matrix<double,9,9> sigF_;
      Eigen::Matrix<double,9,9> iSigF_;

      double lDenom_;
      double iVar_;

      double clientRougheningFactor_;
      double agentRougheningFactor_;

      double averageDeflection_;
      X_t averageX_;

      YVector* sampleFVar_(YVector fundamentals, YVector transients,
                  double* weight, TauMappings::H* h, TauMappings::C* c, // OUTPUT
                  Actor turn = AGENT, EpaVector* aab = NULL, EpaVector* observ = NULL,
                  bool reset = false);

      YVector* sampleTVar_(TauMappings::H, TauMappings::C, YVector*);

      void roughenSamples_ (std::vector<State<X_t>*> samples);

      double computeSampleFVarWeight_(State<X_t>* state, double ivar, double lDenom, 
                                Actor turn, EpaVector* observ = NULL);
      double computeSampleXVarWeightExp_(State<X_t>* sample, X_t* xObserved);

      State<X_t>* computeSampleMean_(std::vector<State<X_t>*> samples);

      double computeLogProbDistFunc_(EpaVector x, double mean,
                                     double iVar, double lDenom);

      YVector computeNoiseVector_();

      private:
        void init_(TauMappings* t, Settings* s);
  };

}
#endif // _AGENT_H
