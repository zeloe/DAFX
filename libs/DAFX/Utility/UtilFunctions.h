#ifndef UTILFUNCTIONS_H
#define UTILFUNCTIONS_H
#pragma once
#include "JuceHeader.h"

inline int   nextPower_2 ( unsigned  int  x)

{

double nextnum = std::ceil(std::log2(x));

double result = pow  (2.0  ,  nextnum) ;

return  int ( result ) ;

}


inline int nextPowerOfBlockSize ( unsigned  int  blockSize, unsigned int maxSize)

{

    int result = (((maxSize) / (blockSize)) + 1) * blockSize;

    return  result;

}

#endif //UTILFUNCTIONS_H
