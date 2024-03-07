



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
    
    
    float smoothing (float& currentParam)
    {
        return currentParam += inc;
    }
    
    void resetSmoother()
    {
        isSmoothing = false;
    }
    
    bool isSmoothing = false;
    
private:
    float inc = 0;
};
