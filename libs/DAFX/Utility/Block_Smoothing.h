



#include "JuceHeader.h"

#pragma once

template<typename FloatType>
class BlockSmoothing 
{
public:
    BlockSmoothing() {}
    ~BlockSmoothing()
    {
      
    }

    void calcCoeff(FloatType& newParam, FloatType& currentParam, int bs)
    {
        inc = (newParam - currentParam) / bs;
        isSmoothing = true;
    }

    void prepare(int maxBlockSize)
    {
        maxBs = maxBlockSize;
        fac = 1.0f / (maxBs);
    }

    void calcCoeff(FloatType& newParam, FloatType& currentParam)
    {
        inc = (newParam - currentParam) * fac;
        isSmoothing = true;
        this->currentParam = &currentParam;
        this->newParam = &newParam;
    }
     

    FloatType smoothing()
    {
        return *this->currentParam += inc;
    }

    void smooth(FloatType& newParam, FloatType& currentParam)
    {
        inc = (newParam - currentParam) * fac;
        isSmoothing = true;
        this->currentParam = &currentParam;
        this->newParam = &newParam;
        *this->currentParam += inc * maxBs;
        *this->currentParam = *this->newParam;
    }

    void resetSmoother()
    {
        isSmoothing = false;
        *this->currentParam = *this->newParam;
    }
     

    bool isSmoothing = false;

private:
    FloatType inc = 0;
    FloatType fac = 0;
    size_t maxBs = 0;
    FloatType* currentParam = nullptr;
    FloatType* newParam = nullptr;
};
