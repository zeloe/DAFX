



#include "JuceHeader.h"

#include "JuceHeader.h"
 
class BlockSmoothing 
{
public:
    BlockSmoothing() {}
    ~BlockSmoothing()
    {
      
    }

    void calcCoeff(float& newParam, float& currentParam, int bs)
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
     

    float smoothing()
    {
        return *this->currentParam += inc;
    }

    void smooth(float& newParam, float& currentParam)
    {
        inc = (newParam - currentParam) / maxBs;
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
    float inc = 0;
    size_t maxBs = 0;
    float* currentParam = nullptr;
    float* newParam = nullptr;
};
