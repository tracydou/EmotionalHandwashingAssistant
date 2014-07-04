#ifndef _TRANSIENT_VARIABLES_H
#define _TRANSIENT_VARIABLES_H

#include <string>

#include "values.h"

namespace bayesact
{

  // Note that these constants are based on the binary representation
  // in the tdyn.dat files. Don't change these without considering the
  // implications.
  #define TAE 0x100
  #define TAP 0x080
  #define TAA 0x040
  #define TBE 0x020
  #define TBP 0x010
  #define TBA 0x008
  #define TCE 0x004
  #define TCP 0x002
  #define TCA 0x001

  class TransientVariable
  {
    public:
      TransientVariable(unsigned);

      bool isSet(unsigned flag); // This is probably the prefered method.
      bool isSetIndex(unsigned index);

      bool flip(unsigned flag);
      bool set(unsigned flag, bool val);

      static unsigned* flags;

      double computeFactor(YVector tau);

      unsigned value_;
    private:
  };
}

#endif // _TRANSIENT_VARIABLES_H
