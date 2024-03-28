



#include "JuceHeader.h"


class block_Smoothing
{
public:
    block_Smoothing() {}
    ~block_Smoothing() {}
    
    void calcCoeff(float& newParam,float& currentParam,int bs)
    {
        inc = (newParam - currentParam) / bs;
        isSmoothing = true;
    }
    void prepare(int maxBlockSize)
    {
        maxBs = maxBlockSize;
    }
    
    void calcCoeff(float& newParam, float& currentParam)
    {
        inc = (newParam - currentParam) / maxBs;
        isSmoothing = true;
        this->currentParam = &currentParam;
        this->newParam = &newParam;
    }
    
    float smoothing ()
    {
        return *this->currentParam += inc;
    }
    
    void resetSmoother()
    {
        isSmoothing = false;
        *this->currentParam = *this->newParam;
    }
    
    bool isSmoothing = false;
    
private:
    float inc = 0;
    float maxBs = 0;
    float* currentParam = nullptr;
    float* newParam = nullptr;
};
