#ifdef _PYTHON
#include <Python.h>
#endif

#include <istream>
#include <fstream>
#include <vector>

#include <Eigen/Dense>

#include "values.h"

#include "transientvariable.h"
#include "transientdynamics.h"

#include "taumappings.h"

using namespace bayesact;

TauMappings::TauMappings(TransientDynamics* tdyn)
{
  this->init_(tdyn);
}

TauMappings::TauMappings(std::string filename)
{
  std::fstream f(filename.c_str(), std::fstream::in);
  if (!f.good()) {
    f.close();
    throw new std::exception();
  }

  TransientDynamics* tdyn = TransientDynamics::parseStream(f);
  f.close();

  this->init_(tdyn);
}

void
TauMappings::init_(TransientDynamics* tdyn)
{
  // Initialize gTemplate_ and mTemplate_
  this->gTemplate_ = new std::vector<TransientVariable>[EPA_DIMENSIONS + 1];
  this->mTemplate_ = new Eigen::Matrix<double, Y_SIZE, Eigen::Dynamic>[EPA_DIMENSIONS + 1];

  // Create local variables matrix and factors
  Eigen::Matrix<double,Y_SIZE,Eigen::Dynamic> matrix;
  std::vector<TransientVariable> factors;

  matrix = tdyn->getMatrix();
  factors = tdyn->getFactors();

  for (unsigned i = 0; i < factors.size(); i++) {
    unsigned j = TAU;

    if (factors[i].isSet(TBE))
      j = BEH_E;
    else if (factors[i].isSet(TBP))
      j = BEH_P;
    else if (factors[i].isSet(TBA))
      j = BEH_A;

    factors[i].set(TBE, false);
    factors[i].set(TBP, false);
    factors[i].set(TBA, false);

    const unsigned oldSize = this->mTemplate_[j].cols();

    this->gTemplate_[j].push_back(factors[i]);
    this->mTemplate_[j].conservativeResize(Eigen::NoChange, oldSize + 1);
    for (unsigned k = 0; k < Y_SIZE; k++)
      this->mTemplate_[j](k, oldSize) = matrix(k, i);
  }
}

TauMappings::~TauMappings()
{
  delete[] this->gTemplate_;
  delete[] this->mTemplate_;
}

TauMappings*
TauMappings::parseFile(std::string tdynFilename)
{
  std::fstream f(tdynFilename.c_str(), std::fstream::in);
  if (!f.good()) {
    f.close();
    throw new std::exception();
  }

  TransientDynamics* tdyn = TransientDynamics::parseStream(f);

  f.close();

  return new TauMappings(tdyn);
}

// This function may not be nessecary if we can "computeFactor" without
// it looking strange and unreadable.
std::vector<double>*
TauMappings::computeG_(YVector tau)
{
  std::vector<double>* retval = new std::vector<double>[EPA_DIMENSIONS + 1];

  for (unsigned i = 0; i < EPA_DIMENSIONS + 1; i++) {
    for (unsigned j = 0; j < this->gTemplate_[i].size(); j++) {
      retval[i].push_back(this->gTemplate_[i][j].computeFactor(tau));
    }
  }

  return retval;
}

void
TauMappings::computeHC(YVector tau, H* const h, C* const c)
{
  *h = H::Zero();
  *c = C::Zero();

  std::vector<double>* g = this->computeG_(tau);

  for (unsigned i = 0; i < EPA_DIMENSIONS + 1; i++) {
    for (unsigned j = 0; j < g[i].size(); j++) {
      for (unsigned k = 0; k < Y_SIZE; k++) {
        if (i == 0)
          (*c)(k) += g[i][j] * this->mTemplate_[i](k, j);
        else
          (*h)(k,i-1) += g[i][j] * this->mTemplate_[i](k, j);
      }
    }
  }
}
