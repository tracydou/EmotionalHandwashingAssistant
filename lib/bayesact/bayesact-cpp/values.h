#ifndef __VALUES_H
#define __VALUES_H

#define EPA_DIMENSIONS 3

#define Y_SIZE EPA_DIMENSIONS*3

#define E_INDEX 0
#define P_INDEX 1
#define A_INDEX 2

#define ACTOR_INDEX 0
#define BEHAVIOUR_INDEX 1
#define CLIENT_INDEX 2

// These values must correspond with the index represented in tau.
#define TAE_INDEX EPA_DIMENSIONS * ACTOR_INDEX + E_INDEX
#define TAP_INDEX EPA_DIMENSIONS * ACTOR_INDEX + P_INDEX
#define TAA_INDEX EPA_DIMENSIONS * ACTOR_INDEX + A_INDEX
#define TBE_INDEX EPA_DIMENSIONS * BEHAVIOUR_INDEX + E_INDEX
#define TBP_INDEX EPA_DIMENSIONS * BEHAVIOUR_INDEX + P_INDEX
#define TBA_INDEX EPA_DIMENSIONS * BEHAVIOUR_INDEX + A_INDEX
#define TCE_INDEX EPA_DIMENSIONS * CLIENT_INDEX + E_INDEX
#define TCP_INDEX EPA_DIMENSIONS * CLIENT_INDEX + P_INDEX
#define TCA_INDEX EPA_DIMENSIONS * CLIENT_INDEX + A_INDEX

#include <Eigen/Core>

namespace bayesact
{
  enum Actor { AGENT=0, CLIENT };
  #define Actor_MAX Actor::CLIENT // This MUST be adjusted if Actor is extended

  typedef Eigen::Matrix<double,Y_SIZE,1> YVector;
  typedef Eigen::Matrix<double,EPA_DIMENSIONS,1> EpaVector;

  inline EpaVector getEpa(YVector yVector, unsigned index)
  {
    EpaVector retval = EpaVector::Zero();

    for (size_t i = 0; i < EPA_DIMENSIONS; i++) {
      retval(i) = yVector(EPA_DIMENSIONS * index + i);
    }

    return retval;
  }
}

#endif //__VALUES_H
