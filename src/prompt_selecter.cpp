/*
 *  File:				prompt_selecter.cpp
 *  Created by:			Luyuan Lin
 * 
 *  Define a class that selects proper prompts from <proposition, EPA>
 *  inputs. Selection is based on survey result link-here.
 */

#include "prompt_selecter.hpp"
#include <stdlib.h> // qsort()
#include "../lib/csv_parser/csv_v3.h"
#include <iostream>

using std::cout;
using std::endl;

namespace EHwA {

// ==== This class will be used only inside class PromptSelecter ====

Item::Item(): proposition_(-1) {
  filename_ = "";
  epa_.resize(3);
  for (int i = 0; i < 3; ++i) {
    epa_[i] = 0;
  }
}

Item::Item(string filename, int proposition, const vector<double> epa) :
  proposition_(proposition) {
  filename_ = filename;
  epa_.resize(3);
  for (int i = 0; i < 3; ++i) {
    epa_[i] = epa[i];
  }
}

Item::Item(string filename, int proposition, double epa[]) {
  proposition_ = proposition;
  filename_ = filename;
  epa_.resize(3);
  for (int i = 0; i < 3; ++i) {
    epa_[i] = epa[i];
  }
}
  
Item::Item(const Item& item) {
  filename_ = item.getFilename();
  proposition_ = item.getProposition();
  epa_ = item.getEPA();
}

Item::~Item() {
  // nothing needed
}

void Item::SwapWith(Item& item) {
  if (this == &item) {
    return;
  }
  Item tmp(item);
  (*this) = item;
  item = tmp;  
}

Item& Item::operator = (const Item& item) {
  if (this == &item) {
    return *this;
  }
  filename_ = item.getFilename();
  proposition_ = item.getProposition();
  epa_ = item.getEPA();
  return *this;
}

string Item::getFilename() const {return filename_;}
int Item::getProposition() const {return proposition_;}
vector<double> Item::getEPA() const { return epa_;}

string Item::DebugString() const {
  string result = "";
  result += "filename = ";
  result += filename_;
  result += ", prompt = ";
  result += proposition_;
  result += "epa = <";
  for (int i = 0; i < 3; ++i) {
    result += epa_[i];
    if (i != 2) {
      result += ", ";
    }
  }
  result += ">\n";
  return result;
}

// ================= End of definition of local class ================

const char* PromptSelecter::HEADER_FILENAME = "filename";
const char* PromptSelecter::HEADER_PROMPT = "prompt";
const char* PromptSelecter::HEADER_EVALUATION = "evaluation";
const char* PromptSelecter::HEADER_POTENCY = "potency";
const char* PromptSelecter::HEADER_ACTIVITY = "activity";

int compare (const void* a, const void* b) {
  int result = ((Item*)a) -> getProposition() - ((Item*)b) -> getProposition();
  for (int i = 0; i <3 && result == 0; ++i) {
    result = ((Item*)a) -> getEPA()[i] - ((Item*)b) -> getEPA()[i];
  }
  return result;
}
	
PromptSelecter::PromptSelecter(string items_filename, string default_prompt_filename) {
  default_prompt_ = default_prompt_filename;
  // read contents into items_ from "filename"
  io::CSVReader<FIELD_NUMBER_OF_EACH_ITEM> in(items_filename);
  in.read_header(io::ignore_extra_column, HEADER_FILENAME, HEADER_PROMPT,
                 HEADER_EVALUATION, HEADER_POTENCY, HEADER_ACTIVITY);
  string filename;
  int prompt = -1;
  double evaluation = 0.0, potency = 0.0, activity = 0.0;
  while(in.read_row(filename, prompt, evaluation, potency, activity)){
	double epa_array[] = {evaluation, potency, activity};
	Item item(filename, prompt, epa_array);
	cout << "=============== tracy: pushed in item: " << item.DebugString();
    items_.push_back(item);
  }
  // sort items in order of proposition, e, p, & a
  qsort(&items_, items_.size(), sizeof(Item), compare);
  cout << "=========== sorted!" << endl;
  // update index_
  int current_prompt = INVALID_PROMPT;
  for (unsigned int i = 0; i < items_.size(); ++i) {
	cout << "============= tracy: item = " << items_[i].DebugString();
    if (items_[i].getProposition() != current_prompt) {
      current_prompt = items_[i].getProposition();
      index_.push_back(pair<int, unsigned int> (current_prompt, i));
    }
  }  
}

PromptSelecter::~PromptSelecter() {
	// do nothing
}

int PromptSelecter::FindStartingIndexOfProposition(int proposition) {
  for (unsigned int i = 0; i < index_.size(); ++i) {
    if (index_[i].first == proposition) {
      return index_[i].second;
    }
  }
  return NOT_FOUND_PROPOSITION_PROMPT;
}

double distance(const vector<double>& real_epa, const vector<double>& desired_epa) {
  double dist = MAX_DIST;
  if (real_epa.size() != 3 || desired_epa.size() != 3) {
    return dist;
  }
  dist = 0;
  for (int i = 0; i < 3; ++i) {
    dist += (real_epa[i] - desired_epa[i]) * (real_epa[i] - desired_epa[i]);
  }
  return dist;
} 

// If no proper prompt is found, then a default video will be played    
string PromptSelecter::Select(const vector<double>& EPA, int prompt) {
	string prompt_filename = default_prompt_;
	int starting_index = FindStartingIndexOfProposition(prompt);
  if (starting_index != NOT_FOUND_PROPOSITION_PROMPT) {
    double current_dist = MAX_DIST;
    for (unsigned int i = starting_index; i < items_.size(); ++i) {
      if (items_[i].getProposition() != prompt) {
        break;
      }
      double dist = EHwA::distance(items_[i].getEPA(), EPA);
      if (dist < current_dist) {
        current_dist = dist;
        prompt_filename = items_[i].getFilename();
      }
    }
  }
	return prompt_filename;
}
 
}  // namespace EHwA
