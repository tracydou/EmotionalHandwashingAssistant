#ifndef __XVAR_H
#define __XVAR_H

#include "values.h"

namespace bayesact
{

  class XVar{ 
    public:
      virtual XVar& operator+= (const XVar &rhs) {}
      virtual XVar& operator-= (const XVar &rhs) {}
      virtual XVar& operator*= (const XVar &rhs) {}
      virtual XVar& operator/= (const XVar &rhs) {}
      virtual XVar& operator*= (const double &rhs) {}
      virtual XVar& operator/= (const double &rhs) {}
      virtual XVar& operator+  (const XVar &rhs) {}
      virtual XVar& operator-  (const XVar &rhs) {}
      virtual XVar& operator*  (const XVar &rhs) {}
      virtual XVar& operator/  (const XVar &rhs) {}
      virtual XVar& operator*  (const double &rhs) {}
      virtual XVar& operator/  (const double &rhs) {}

      virtual bool operator== (const XVar &rhs) 
              { return this->turn == rhs.turn; }

      bool turn;
      inline virtual Actor getTurn()
      {
        // Placeholder code
        turn = !turn;
        if (turn)
          return AGENT;
        else
          return CLIENT;
      }
  };
}

#endif //__XVAR_H
