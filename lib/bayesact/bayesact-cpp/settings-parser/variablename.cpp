#include <string>
#include <cctype>

#include "variablename.h"

VariableName::VariableName(std::string name)
{
  this->_constName = "";
  this->_humanReadableName = name;

  for (size_t i = 0; i < name.length(); i++) {
    char newValue = _convertToConstChar(name[i]);

    if (newValue != UNKNOWN_CHAR)
      this->_constName += newValue;
  }
}

std::string
VariableName::getConstName()
{
  return this->_constName;
}

std::string
VariableName::getVariableName()
{
  return this->_variableName;
}

std::string 
VariableName::getHumanReadableName()
{
  return this->_humanReadableName;
}

char 
VariableName::_convertToConstChar(char c)
{
  if (c == ' ' || c == '_' || c == '.')
    return '_';
  else if (isdigit(c))
    return c;
  else if (isalpha(c))
    return toupper(c);
  else
    return UNKNOWN_CHAR;
}
