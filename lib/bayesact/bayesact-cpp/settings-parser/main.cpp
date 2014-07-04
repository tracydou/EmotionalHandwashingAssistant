#include <cstring>

#include <iostream>
#include <fstream>
#include <string>

#include "settingsheader.h"

void printHelpstring(char* execName)
{
  std::cout << "Usage is: " << execName << " [-o <outfile>] <input>";
  std::cout << std::endl;
}

int main(int argc, char* argv[])
{
  char* inName = NULL;
  char* outName = NULL;

  if (argc == 1) {
    printHelpstring(argv[0]);
    return 1;
  }

  for (int i = 1; i < argc; i++) {
    if (strcmp(argv[i],"-o") == 0) {
      if (outName != NULL || i+1 == argc) {
        printHelpstring(argv[0]);
        return 1;
      }
      outName = argv[++i];
    } else {
      if (inName != NULL) {
        printHelpstring(argv[0]);
        return 1;
      }
      inName = argv[i];

    }
  }

  SettingsHeader* parser = SettingsHeader::parseFile(inName);

  std::streambuf * buf;
  std::ofstream of;

  if (outName != NULL) {
    std::cout << "Writing to " << outName << std::endl; // DEBUG
    of.open(outName);
    buf = of.rdbuf();
  } else {
    std::cout << "No outfile named. Using stdout." << std::endl; // DEBUG
    buf = std::cout.rdbuf();
  }

  std::ostream out(buf);

  parser->writeHeader(out, outName);

  std::cout << "Write completed without error" << std::endl; //DEBUG

  //Note: Assuming that out.close() is called on the deconstructor

  return 0;
}
