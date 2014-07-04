#include <string>
#include <vector>
#include <istream>
#include <cctype> // isspace

#include "transientvariable.h"

#include "transientdynamics.h"

using namespace bayesact;

TransientDynamics::TransientDynamics()
{
  this->matrix_ = Eigen::Matrix<double,Y_SIZE,Eigen::Dynamic>(Y_SIZE,0);
  this->factors_ = std::vector<TransientVariable>();
}

TransientDynamics*
TransientDynamics::parseStream(std::istream& stream)
{
  TransientDynamics* retval = new TransientDynamics();

  while (!stream.eof()) {
    std::string tempFlags;
    unsigned flags = 0;

    stream.ignore();

    stream >> tempFlags;

    // Translating flags (string) into tempFlags (unsigned)
    for (int i = 0; i < tempFlags.length(); i++) {
      if (tempFlags[i] == '1')
        flags = (flags << 1) + 1;
      else if (tempFlags[i] == '0')
        flags = flags << 1;
    }

    retval->factors_.push_back (TransientVariable(flags));

    const unsigned oldSize = retval->matrix_.cols();
    retval->matrix_.conservativeResize(Eigen::NoChange, oldSize + 1);

    for (unsigned i = 0; i < Y_SIZE; i++)
      stream >> retval->matrix_(i, oldSize);
    
    // Eat rest of line.
    while (!stream.eof() && isspace(stream.peek())) stream.get();
  }

  return retval;
}

Eigen::Matrix<double,Y_SIZE,Eigen::Dynamic>
TransientDynamics::getMatrix()
{
  return this->matrix_;
}

std::vector<TransientVariable>
TransientDynamics::getFactors()
{
  return this->factors_;
}
