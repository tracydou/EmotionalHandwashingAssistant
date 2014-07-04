#ifndef _TRANSIENT_DYNAMICS_H
#define _TRANSIENT_DYNAMICS_H

#include <istream>
#include <vector>
#include "transientvariable.h"

#include <Eigen/Core>

#include "values.h"

namespace bayesact
{
  class TransientDynamics
  {
    public:
      TransientDynamics(std::string);

      static TransientDynamics* parseStream(std::istream& stream);

      Eigen::Matrix<double,Y_SIZE,Eigen::Dynamic> getMatrix();
      std::vector<TransientVariable> getFactors();

      // ~TransientDynamics();

    private:
      Eigen::Matrix<double,Y_SIZE,Eigen::Dynamic> matrix_;
      std::vector<TransientVariable> factors_;

      TransientDynamics();
  };
}

#endif //_TRANSIENT_DYNAMICS_H
