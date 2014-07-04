#include <Python.h>

#include <cstdlib>

#include <Eigen/Core>

#include <fstream>

#include "values.h"

#include <iostream>

#include "transientdynamics.h"
#include "taumappings.h"

#include "xvar.h"
#include "agent.h"

#define LOOPS 50

bayesact::EpaVector* createRandomEpa()
{
  bayesact::EpaVector* retval = new bayesact::EpaVector();

  for (unsigned i = 0; i < EPA_DIMENSIONS; i++) {
    const double randomDouble = (double)(rand()) / (double)(RAND_MAX);

    (*retval)(i) = randomDouble * 8.6 - 4.3;
  }

  return retval;
}

int main()
{
  const char* filename = "tdynamics-male.dat";

  std::cout << "Begining to read " << filename << std::endl;

  std::fstream fs;
  fs.open(filename, std::fstream::in);

  bayesact::TransientDynamics* tdyn = bayesact::TransientDynamics::parseStream(fs);

  fs.close();

  std::cout << "File read" << std::endl;

  bayesact::TauMappings tauMappings (tdyn);

  bayesact::Agent<bayesact::XVar>* agent;
  agent = new bayesact::Agent<bayesact::XVar>(&tauMappings);

  for (unsigned i = 0; i < LOOPS; i++) {
    std::cout << "Trial " << i << " beginning" << std::endl;

    bayesact::EpaVector* aab = createRandomEpa();
    bayesact::EpaVector* observ = createRandomEpa();

    //std::cout << "  Random EPA values generated" << std::endl;

    agent->propagateForward(aab, observ);

    //std::cout << "  Trial " << i << " complete" << std::endl;
  }

  delete tdyn;
}
