#ifndef _VARIABLE_NAME_H
#define _VARIABLE_NAME_H

#include <string>

#define UNKNOWN_CHAR '?'

class VariableName
{
  public:
    VariableName(std::string);

    std::string getConstName();
    std::string getVariableName();
    std::string getHumanReadableName();

  private:
    std::string _constName;
    std::string _variableName;
    std::string _humanReadableName;

    static char _convertToConstChar(char);
    static char _convertToVariableChar(char);
};

#endif // _VARIABLE_NAME_H
