
#pragma once
#include "JuceHeader.h"

inline int   nextPower_2 ( unsigned  int  x)

{

double nextnum = std::ceil(std::log2(x));

double result = pow  (2.0  ,  nextnum) ;

return  int ( result ) ;

}
