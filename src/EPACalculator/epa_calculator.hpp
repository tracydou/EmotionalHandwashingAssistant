/*
 *  File:				epa_calculator.hpp
 *  Created by:			Luyuan Lin
 * 
 *  Defines an EPA-Calc that converts user behaviours into epa values.
 *  Currently it takes handpositions as input and is implemented using
 *  threshold techniques as a prototype, one can improve it by taking
 *  more features as input and using more sophisticated ML models to 
 *  get the convertion function.
 *  
 */

#ifndef EPA_CALCULATOR_
#define EPA_CALCULATOR_

#include <utility>
#include <list>
#include <vector>
#include "../defines.hpp"

using std::list;
using std::pair;
using std::vector;

namespace EHwA {

class EPACalculator {
  public:   
    EPACalculator();
    ~EPACalculator();
    
    // Input positions of hands frames over a set of frames,
    // update & return currentEPA
    // Prerequest: most recent positions are at the beginning of hand_pos
    static vector<double> Calculate(
      const list<pair<Position, Position> >& hand_pos);
    
    // define here, instantiate in .cpp
    const static double THRESHOLD_DIST_FOR_POTENCY;
    const static double THRESHOLD_DIFF_FOR_ACTIVITY;
    const static double max_epa;
    const static double min_epa;
   
  protected:
    static double ConvertDistToPotency(
      const list<pair<Position, Position> >& hand_pos);
    static double ConvertDiffToActivity(
      const list<pair<Position, Position> >& hand_pos);
};

}    // namespace EHwA

#endif  // EPA_CALCULATOR_
