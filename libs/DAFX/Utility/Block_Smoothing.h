



#include "JuceHeader.h"


class block_Smoothing : public juce::Thread
{
public:
    block_Smoothing() : Thread("UpdateParam") {}
    ~block_Smoothing() 
    {
        waitForThreadToExit(1000);
        stopThread(1000);
    }
    
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

    void calcCoeffThreaded(float& newParam, float& currentParam)
    {
        inc = (newParam - currentParam) / maxBs;
        isSmoothing = true;
        this->currentParam = &currentParam;
        this->newParam = &newParam;
        startThread(Priority::normal);
    }
    
    float smoothing ()
    {
        return *this->currentParam += inc;
    }
    
    void smoothThread()
    {
        for(int i = 0; i < maxBs; i++){ 
         *this->currentParam += inc;
        }
        *this->currentParam = *this->newParam;
    }


    void resetSmoother()
    {
        isSmoothing = false;
        *this->currentParam = *this->newParam;
    }

    void run() override
    {
            this->smoothThread();

    }

    
    bool isSmoothing = false;
    
private:
    float inc = 0;
    float maxBs = 0;
    float* currentParam = nullptr;
    float* newParam = nullptr;
};
