
#include "settingdefaults.h"

#include "settings.h"

using namespace bayesact;

Settings::Settings()
{
  this->_values = new double[NUM_SETTINGS];
  for (unsigned i = 0; i < NUM_SETTINGS; i++)
    this->_values[i] = CONF_DEFAULTS[i];
}

double
Settings::getValue(ConfigVariable index)
{
  return this->_values[index];
}


void
Settings::setValue(ConfigVariable index, double value)
{
  this->_values[index] = value;
}
