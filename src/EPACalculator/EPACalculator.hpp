/*
 *  File:				EPACalculator.hpp
 *  Created by:			Luyuan Lin
 *  Created:			March 2014
 *  Last Modified:		March 2014
 *  Last Modified by:	Luyuan Lin
 *  
 */

#includes <vector>

#ifndef EPA_CALCULATOR_
#define EPA_CALCULATOR_

class EPACalculator {
  public:   
    EPACalculator();
    ~EPACalculator();
    
    // Input positions of hands and facial expressions over a set of
    // frames, update and return EPA
    vector<double> Calculate(const vector<FacialExpression>& facialExp,
                              const vector<vector<Point3_>>& handPos);
    vector<double> getCurrentEPA();
   
  protected:
    double ConvertFacialExpressionToEvaluation(
      const vector<FacialExpression>& facialExp);
    double ConvertDistToPotency(const vector<vector<Point3_>>& handPos);
	double ConvertDiffToActivity(const vector<vector<Point3_>>& handPos);
  
    vector<double> currentEPA;
};


#endif  // EPA_CALCULATOR_
