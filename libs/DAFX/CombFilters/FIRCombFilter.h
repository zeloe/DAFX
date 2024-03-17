/*
  ==============================================================================

    FIRCombFilter.h
    Created: 8 Apr 2023 7:47:46pm
    Author:  Onez

  ==============================================================================
*/

#pragma once
#include "../DelayLine/DelayLine.h"
class FIRComb {
public:
    FIRComb()
    {
        ffdelayLine = std::make_unique<DelayLine>();
        smoother = std::make_unique<block_Smoothing>();
    }
    ~FIRComb(){}
    
    void prepare(int delayLineSize, int& maxBlockSize, double sampleRate) noexcept
    {
        ffdelayLine->prepare(delayLineSize, maxBlockSize);
        ms = delayLineSize;
        ffdelayLine->setParams(delayLineSize);
        
        //
        this->sampleRate = sampleRate;
    }
    
    void setFrequency(float hz, float t60)
    {
        D = 1000.f / hz;
        ffdelayLine->setParams(D * sampleRate * 0.001f);
        //with other values of gain(0.5) it might become instable
        // beware of 0 gain != 0 never
        float temp = 20.f * log10(0.5);
        float temp2 = exp(log(0.001) * (-60.f * D / temp));
        a = pow(10.f, temp2 / 20.f);
        //();
        smoother->calcCoeff(a, current_a, current_bs);
    }
    
   
    
    
    void process(juce::AudioBuffer<float>& buffer,  int bs)
    {
        current_bs = bs;
        const float* leftInRPtr = buffer.getReadPointer(0);
        const float* rightInRPtr = buffer.getReadPointer(1);
        
        float* leftOutRPtr = buffer.getWritePointer(0);
        float* rightOutRPtr = buffer.getWritePointer(1);
        if(smoother->isSmoothing == true)
        {
            while(bs > 0)
            {
                
                
                const float leftIn = *leftInRPtr++;
                const float rightIn = *rightInRPtr;
                
                
                
                *leftOutRPtr++ = leftIn + ffdelayLine->processBlock(leftIn,current_bs) * smoother->smoothing(current_a);
                float rightOut = *rightOutRPtr++;
                bs--;
            }
            current_a = a;
            smoother->resetSmoother();
        }
        else
        {
            while(bs > 0)
            {
                const float leftIn = *leftInRPtr++;
                const float rightIn = *rightInRPtr;
                
                *leftOutRPtr++ = leftIn + ffdelayLine->processBlock(leftIn,current_bs) * current_a;
                float rightOut = *rightOutRPtr++;
                bs--;
            }
        }
    }
    
    
    
    
    
private:
    std::unique_ptr<DelayLine> ffdelayLine;
    std::unique_ptr<block_Smoothing> smoother;
    float sampleRate = 0;
    float D = 0;
    float a = 0;
    float current_a = 0;
    int current_bs = 0;
    float ms = 0;
    juce::AudioBuffer<float> delayLine;
};

