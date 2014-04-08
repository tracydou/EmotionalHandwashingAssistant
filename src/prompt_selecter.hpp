/*
 *  File:			prompt_selecter.hpp
 *  Created by:			Luyuan Lin
 * 
 *  Define a class that selects proper prompts from <proposition, EPA>
 *  inputs. Selection is based on survey result link-here.
 */

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
  Item(int id, string filename, int proposition, const vector<double> epa);
  Item(const Item& item);
  ~Item();
  
  void SwapWith(Item& item);
  Item& operator = (const Item& item);

  int getId() const;
  string getFilename() const;
  int getProposition() const;
  vector<double> getEPA() const;

private:
  int id_;
  string filename_;
  int proposition_; // this should be consistant with action types
  vector<double> epa_;
};

class PromptSelecter {
public:
    // "filename" must be a csv file cosisting of Items; i.e. each line is
    // <id, filename, proposition, e, p, a>
    PromptSelecter(string items_filename, string default_prompt_filename);
    ~PromptSelecter();
	
    // select proper prompt basing on EPA and propositional prompt
    // return filename of the proper prompt
    string Select(const vector<double>& EPA, int prompt);

private:
    int FindStartingIndexOfProposition(int proposition);

    vector<Item> items_; // sorted Items, originally read from "filename"
    vector<pair<int, int> > index_; // maps from "prositional prompt" to "starting index in items_" 
    string default_prompt_;
};
    
}  // namespace EHwA
