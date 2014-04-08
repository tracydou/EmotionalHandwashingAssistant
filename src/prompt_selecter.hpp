/*
 *  File:				prompt_selecter.hpp
 *  Created by:			Luyuan Lin
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
    // return filename of the proper prompt
    string Select(const vector<double>& EPA, int prompt);

protected:
    string filename_;
};
    
}  // namespace EHwA
