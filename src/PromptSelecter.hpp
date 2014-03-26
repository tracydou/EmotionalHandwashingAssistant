/*
 *  File:				PromptSelecter.hpp
 *  Created by:			Luyuan Lin
 *  Created:			March 2014
 *  Last Modified:		March 2014
 *  Last Modified by:	Luyuan Lin
 * 
 *  Define a class that selects proper prompts from <proposition, EPA>
 *  inputs. Selection is based on survey result link-here.
 */

#include <string>
#include <vector>
using std::string;
using std::vector;

namespace EHwA {
	
class PromptSelecter {
public:
    PromptSelecter(string filename);
	~PromptSelecter();
	
    // select proper prompt basing on EPA and propositional prompt
    // return id of the proper prompt
    int Select(const vector<double>& EPA, int prompt);

protected:
    string filename;
};
    
}  // namespace EHwA
