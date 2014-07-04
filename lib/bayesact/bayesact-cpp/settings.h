//---------------------------------------------------------------------------
// NOTICE:
// This file has been generated from settings.csv
// To change the contents of this file, please adjust settings.csv
//  and make.
//---------------------------------------------------------------------------
#ifndef _SETTINGS_H
#define _SETTINGS_H

#include <math.h>
#include <cfloat>

namespace bayesact
{
  const unsigned NUM_SETTINGS 13

  enum ConfigVariable { AFFECT_CONTROL_PRINCIPLE_STRENGTH = 0,
                        AGENT_IDENTITY_INERTIA,
                        CLIENT_IDENTITY_INERTIA,
                        AGENT_INITIAL_IDENTITY_CERTAINTY,
                        CLIENT_INITIAL_IDENTITY_CERTAINTY,
                        ENVIRONMENT_NOISE,
                        DISCOUNT_FACTOR,
                        NUMBER_OF_SAMPLES,
                        BETA_VALUE_B_CLIENT,
                        BETA_VALUE_B_CLIENT_INIT,
                        BETA_VALUE_B_AGENT,
                        AGENT_ROUGH,
                        CLIENT_ROUGH};

  const double CONF_DEFAULTS[] = { 1, 0.01, 0.01, 0.01, 0.01, 1, 0.9, 300, inf, 1, 0, 0, 0 };

}

#endif //_SETTINGS_H
