/*
 *  File:				prompt_selecter.cpp
 *  Created by:			Luyuan Lin
 *  Created:			March 2014
 *  Last Modified:		March 2014
 *  Last Modified by:	Luyuan Lin
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
	
int PromptSelecter::Select(const vector<double>& EPA, int prompt) {
	// 
	return 0;
}
 
}  // namespace EHwA
