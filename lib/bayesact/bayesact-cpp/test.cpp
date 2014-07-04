#include <cstdlib>

#include <iostream>

#include <Eigen/Core>

#include "values.h"

#include "taumappings.h"
#include "agent.h"
#include "state.h"

using namespace bayesact;

EpaVector
randomEpa()
{
  EpaVector retval;

  for (unsigned i = 0; i < EPA_DIMENSIONS; i++) {
    const double randomDouble = (double)(rand()) / (double)(RAND_MAX);

    retval(i) = randomDouble * 8.6 - 4.3;
  }

  return retval;
}

int main()
{
  TauMappings* tauMapping = TauMappings::parseFile("tdynamics-male.dat");
  Agent<XVar> agent (tauMapping, NULL);

  for (int i = 0; i < 100; i++) {
    EpaVector aab = randomEpa(); 
    EpaVector observ = randomEpa();

    std::cout << "AAB\n" << aab << std::endl;
    std::cout << "OBSERV\n" << observ << std::endl;
     
    agent.propagateForward (&aab, &observ);

    State<XVar> sum;
    for (int j = 0; j < agent.samples.size(); j++) {
      sum += *agent.samples[j];
    }
    sum /= agent.samples.size();

    std::cout << "~F\n" << sum.getF()   << std::endl;
    std::cout << "~TAU\n" << sum.getTau() << std::endl;
  }

  return 0;
}
