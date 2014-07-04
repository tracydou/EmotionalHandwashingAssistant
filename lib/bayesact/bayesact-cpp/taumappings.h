#ifndef _TAU_MAPPING_H
#define _TAU_MAPPING_H

#ifdef _PYTHON
#include <Python.h>

#include <boost/python/tuple.hpp>
#endif

#include <iostream>

#include <Eigen/Dense>

#include "transientdynamics.h"
#include "values.h"

namespace bayesact
{

  #define TAU   0
  #define BEH_E 1
  #define BEH_P 2
  #define BEH_A 3

  class TauMappings
  {
    public:
      typedef Eigen::Matrix<double,Y_SIZE, EPA_DIMENSIONS> H;
      typedef Eigen::Matrix<double,Y_SIZE, 1> C;

      TauMappings(TransientDynamics*);
      TauMappings(std::string);

      void computeHC(YVector tau, H* const h, C* const c);
      #ifdef _PYTHON
      boost::python::tuple computeHC_python(PyObject* tau);
      #endif

      static TauMappings* parseFile(std::string);

      ~TauMappings();

      std::vector<TransientVariable>* gTemplate_;
      Eigen::Matrix<double, Y_SIZE, Eigen::Dynamic>* mTemplate_;

    private:
      std::vector<double>* computeG_(YVector tau);

      void init_(TransientDynamics*);
  };
}
#endif //_TAU_MAPPING_H
