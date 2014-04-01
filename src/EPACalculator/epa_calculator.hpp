/*
 *  File:				epa_calculator.hpp
 *  Created by:			Luyuan Lin
 *  Created:			March 2014
 *  Last Modified:		March 2014
 *  Last Modified by:	Luyuan Lin
 *  
 */

#ifndef EPA_CALCULATOR_
#define EPA_CALCULATOR_

#include <utility>
#include <vector>
#include "../defines.hpp"

using std::vector;
using std::pair;

namespace EHwA {

// TODO: add "facial expressions" as an influencer as well
class EPACalculator {
  public:   
    EPACalculator();
    ~EPACalculator();
    
    // Input positions of hands frames over a set of frames,
    // update & return currentEPA
    vector<double> Calculate(
      const vector<pair<Position, Position> >& hand_pos);
    vector<double> get_current_epa() const;
   
  protected:
    double ConvertDistToPotency(
      const vector<pair<Position, Position> >& hand_pos);
	double ConvertDiffToActivity(
	  const vector<pair<Position, Position> >& hand_pos);
  
    vector<double> current_epa_;
};

}    // namespace EHwA

#endif  // EPA_CALCULATOR_
