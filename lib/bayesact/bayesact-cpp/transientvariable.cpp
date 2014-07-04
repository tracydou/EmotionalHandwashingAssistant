#include "transientvariable.h"

#include "values.h"

using namespace bayesact;

unsigned *TransientVariable::flags = NULL;

TransientVariable::TransientVariable(unsigned value)
{
  this->value_ = value;

  if (TransientVariable::flags == NULL) {
    TransientVariable::flags  = new unsigned[Y_SIZE];

    TransientVariable::flags[TAE_INDEX] = TAE;
    TransientVariable::flags[TAP_INDEX] = TAP;
    TransientVariable::flags[TAA_INDEX] = TAA;
    TransientVariable::flags[TBE_INDEX] = TBE;
    TransientVariable::flags[TBP_INDEX] = TBP;
    TransientVariable::flags[TBA_INDEX] = TBA;
    TransientVariable::flags[TCE_INDEX] = TCE;
    TransientVariable::flags[TCP_INDEX] = TCP;
    TransientVariable::flags[TCA_INDEX] = TCA;
  }
}

bool
TransientVariable::isSet(unsigned flag)
{
  return this->value_ & flag;
}

bool 
TransientVariable::isSetIndex(unsigned index)
{
  return this->isSet(this->flags[index]);
}

bool
TransientVariable::flip(unsigned flag)
{
  return this->value_ ^= flag;
}

bool
TransientVariable::set(unsigned flag, bool value)
{
  if (value)
    return this->value_ |= flag;
  else
    return this->value_ &= ~flag;
}

double
TransientVariable::computeFactor(YVector tau)
{
  double retval = 1.0;

  for (int i = 0; i < Y_SIZE; i++) {
    if (this->isSet(TransientVariable::flags[i]))
      retval *= tau[i];
  }

  return retval;
}
