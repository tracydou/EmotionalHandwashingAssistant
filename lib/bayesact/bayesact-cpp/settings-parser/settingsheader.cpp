#include <vector>

#include <stdlib.h>
#include <fstream>

#include "variablename.h"

#include "settingsheader.h"

SettingsHeader*
SettingsHeader::parseFile (std::string filename, bool hasHeader)
{
  SettingsHeader* retval = new SettingsHeader(filename);

  retval->_inFilename = filename;

  std::string name;
  std::string value;

  std::fstream input(filename.c_str(), std::fstream::in);

  if (hasHeader)
    getline(input, name, ROW_SPLIT);

  while (true) {
    getline(input, name, COL_SPLIT);
    getline(input, value, ROW_SPLIT);

    if (input.eof()) break;

    DefaultValue pair(VariableName(name), atof(value.c_str()));

    retval->_defaultValues->push_back(pair);
  }

  input.close();

  return retval;
}

void
SettingsHeader::writeHeader(std::string filename)
{
  std::fstream out (filename.c_str(), std::fstream::out);

  this->writeHeader(out, filename);

  out.close();
}

void
SettingsHeader::writeHeader(std::ostream& out, std::string name)
{
  VariableName headername(name);

  this->_writeAutogenMessage(out);

  out << "#ifndef _" << headername.getConstName() << std::endl;
  out << "#define _" << headername.getConstName() << std::endl;
  out << std::endl;

  out << "#include <math.h>" << std::endl;
  out << "#include <cfloat>" << std::endl;
  out << std::endl;

  out << "namespace bayesact" << std::endl << "{" << std::endl;

  out << "  const unsigned NUM_SETTINGS ";
  out << this->_defaultValues->size() << std::endl;
  out << std::endl;

  out << "  enum ConfigVariable { ";
  out << this->_defaultValues->at(0).first.getConstName() << " = 0";
  for (size_t i = 1; i < this->_defaultValues->size(); i++) {
    out << ',' << std::endl << "                        ";
    out << this->_defaultValues->at(i).first.getConstName();
  }
  out << "};" << std::endl;
  out << std::endl;

  out << "  const double CONF_DEFAULTS[] = { ";
  out << this->_defaultValues->at(0).second;
  for (size_t i = 1; i < this->_defaultValues->size(); i++) {
    out << ", ";
    out << this->_defaultValues->at(i).second;
  }
  out << " };" << std::endl;
  out << std::endl;

  out << "}" << std::endl;
  out << std::endl;

  out << "#endif //_" << headername.getConstName() << std::endl;

}

void SettingsHeader::_writeAutogenMessage(std::ostream& out)
{
  out << "//";
  for (int i = 0; i < 75; i++)
    out << '-';
  out << std::endl;
  out << "// NOTICE:" << std::endl;
  out << "// This file has been generated from ";
  out <<   this->_inFilename << std::endl;
  out << "// To change the contents of this file, please adjust ";
  out <<   this->_inFilename << std::endl;
  out << "//  and make." << std::endl;
  out << "//";
  for (int i = 0; i < 75; i++)
    out << '-';
  out << std::endl;
}

SettingsHeader::SettingsHeader(std::string inFilename)
{
  this->_defaultValues = new std::vector<DefaultValue>();
  this->_inFilename = inFilename;
}

SettingsHeader::~SettingsHeader()
{
  delete this->_defaultValues;
}
