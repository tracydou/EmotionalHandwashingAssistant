/*
 *  File:				prompt_selecter.cpp
 *  Created by:			Luyuan Lin
 * 
 *  Define a class that selects proper prompts from <proposition, EPA>
 *  inputs. Selection is based on survey result link-here.
 */

#include "prompt_selecter.hpp"

namespace EHwA {
	
PromptSelecter::PromptSelecter(string filename) {
	this -> filename_ = filename;
	// open file, read contents of filename into memory for later use
	// then close file
}

PromptSelecter::~PromptSelecter() {
	// do nothing
}
	
string PromptSelecter::Select(const vector<double>& EPA, int prompt) {
	string prompt_filename = "/home/l39lin/Videos/DELTA.MPG";
	// TO be implemented
	return prompt_filename;
}
 
}  // namespace EHwA
