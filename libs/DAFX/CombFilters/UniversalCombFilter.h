/*
  ==============================================================================

    UniversalCombFilter.h
    Created: 10 Apr 2023 11:03:36am
    Author:  Onez

  ==============================================================================
*/

#pragma once
#include "../DelayLine/DelayLine.h"
class UniversalComb {
public:
    UniversalComb()
    {
        fbdelayLine  = std::make_unique<DelayLine>();
        ffdelayLine = std::make_unique<DelayLine>();
        smoother = std::make_unique<block_Smoothing>();
        
        
    }
    ~UniversalComb(){}

    void prepare(int delayLineSize, int maxBlockSize, double cur_sampleRate, int channels) noexcept
    {
        ffdelayLine->prepare(delayLineSize, maxBlockSize, channels);
        fbdelayLine->prepare(delayLineSize,maxBlockSize, channels);
        fbBuffer.setSize(2,maxBlockSize);
        fbBuffer.clear();
        
        this->sampleRate = cur_sampleRate;
        tempFBL = 0;
        tempFBR = 0;
    }
    void setFrequency(float hz, float lingain)
    {
        const float period  = 1.f / hz;
        float delayTimeSamples = period * sampleRate;
        ffdelayLine->setParams(delayTimeSamples);
        fbdelayLine->setParams(delayTimeSamples);
        a =  lingain;
        current_a = lingain;
    }
    
   
    
    void process(juce::AudioBuffer<float>& buffer,  int bs)
    {
        current_bs = bs;
        const float* leftIn = buffer.getReadPointer(0);
        const float* rightIn = buffer.getReadPointer(1);
        
        float* leftOut = buffer.getWritePointer(0);
        float* rightOut= buffer.getWritePointer(1);
       
        
         float inputL;
         float inputR;
         float tempFFL;
         float tempFFR;

        if(smoother->isSmoothing == true)
        {
            current_a = a;
            ffdelayLine->resetSmoother();
            fbdelayLine->resetSmoother();
            smoother->resetSmoother();
            
        }
        else
        {
            for(int i = 0; i < current_bs; i++)
            {
                inputL = leftIn[i];
                inputR = rightIn[i];
                tempFFL = ffdelayLine->processBlockInter(inputL   * current_a, current_bs);
                tempFFR =  ffdelayLine->processBlockInter(inputR  * current_a, current_bs);
                tempFBL =  (inputL) +   tempFFL   +    fbdelayLine->processBlockInter(tempFBL * current_a,current_bs);;
                tempFBR =  (inputR) +   tempFFR   +   fbdelayLine->processBlockInter(tempFBR * current_a,current_bs);
                leftOut[i]   = 0.15f* (tempFBL);
                rightOut[i]  = 0.15f* (tempFBR);
                
                
            }
        }
    }
    
    
private:
    std::unique_ptr<DelayLine> ffdelayLine;
    std::unique_ptr<DelayLine> fbdelayLine;
    std::unique_ptr<block_Smoothing> smoother;
    juce::AudioBuffer<float> fbBuffer;
    float sampleRate = 0;
    float a = 0;
    float current_a = 0;
    int current_bs = 0;
    float tempFBL = 0;
    float tempFBR = 0;
};

