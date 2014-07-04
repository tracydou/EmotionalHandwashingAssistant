#ifndef _SETTINGS_PARSER_H
#define _SETTINGS_PARSER_H

#include <istream>
#include <ostream>

#include <vector>

#include <utility>

#include "variablename.h"

#define ROW_SPLIT '\n'
#define COL_SPLIT ','

class SettingsHeader
{
  public:
    typedef std::pair<VariableName, double> DefaultValue;

    static SettingsHeader* parseFile (std::string filename,
                                     bool hasHeader = true);

    void writeHeader(std::string);
    void writeHeader(std::ostream& out, std::string name);

    ~SettingsHeader();
  private:
    SettingsHeader(std::string);

    std::string _inFilename;

    void _writeAutogenMessage(std::ostream&);

    std::vector<DefaultValue>* _defaultValues;
};

#endif //_SETTINGS_PARSER_H
