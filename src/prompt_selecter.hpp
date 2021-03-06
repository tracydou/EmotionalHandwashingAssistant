/*
 *  File:			prompt_selecter.hpp
 *  Created by:			Luyuan Lin
 * 
 *  Define a class that selects proper prompts from <proposition, EPA>
 *  inputs. Selection is based on survey result link-here.
 */
#ifndef PROMPT_SELECTER_
#define PROMPT_SELECTER_

#include "defines.hpp"
#include <string>
#include <utility>
#include <vector>

using std::string;
using std::pair;
using std::make_pair;
using std::vector;

namespace EHwA {

class Item {
public:
  Item();
  Item(string filename, int proposition, const vector<double> epa);
  Item(string filename, int proposition, double epa[]);
  Item(const Item& item);
  ~Item();
  
  void SwapWith(Item& item);
  Item& operator = (const Item& item);

  string getFilename() const;
  int getProposition() const;
  vector<double> getEPA() const;
  
  string DebugString() const;

private:
  string filename_;
  int proposition_; // this should be consistant with action types
  vector<double> epa_;
};

class PromptSelecter {
public:
    // "filename" must be a csv file cosisting of Items; i.e. each line is
    // <question_number, filename, prompt_purpose, prompt_number, e, p, a>
    // "default_prompt_filename" is the name of the default video prompt;
    // it can only be the filename, without path prefix
    PromptSelecter(string items_filename, string default_prompt_filename);
    ~PromptSelecter();
	
    // select proper prompt basing on EPA and propositional prompt
    // return filename of the proper prompt
    string Select(const vector<double>& EPA, int prompt);

private:
    int FindStartingIndexOfProposition(int proposition);

    vector<Item> items_; // sorted Items, originally read from "filename"
    vector<pair<int, unsigned int> > index_; // maps from "prositional prompt" to "starting index in items_" 
    string default_prompt_;
    
    // These constants' values should be consistant with OutputMappingResult in ../data
    static const int FIELD_NUMBER_OF_EACH_ITEM = 5; // consist with class Item
    static const char* HEADER_FILENAME; // declare here, define in .cpp
    static const char* HEADER_PROMPT;
    static const char* HEADER_EVALUATION;
    static const char* HEADER_POTENCY;
    static const char* HEADER_ACTIVITY;
};
    
}  // namespace EHwA

#endif  // PROMPT_SELECTER_
