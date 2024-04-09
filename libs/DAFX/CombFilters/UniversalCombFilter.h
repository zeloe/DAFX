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
        smoother->prepare(maxBlockSize);
        this->sampleRate = cur_sampleRate;
        tempFBL = 0;
        tempFBR = 0;
    }
    void setFrequency(float hz)
    {
        const float period  = 1.f / hz;
        float delayTimeSamples = period * sampleRate;
        ffdelayLine->setParams(delayTimeSamples);
        fbdelayLine->setParams(delayTimeSamples);
         
    }
    void setLinGain(float g)
    {
        a = g;
        smoother->smooth(a, current_a);
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

        if (ffdelayLine->smoother_DelayTime->isSmoothing == true)
        {

            for (int i = 0; i < current_bs; i++)
            {
                inputL = leftIn[i];
                inputR = rightIn[i];
                tempFFL = ffdelayLine->processBlockInter(inputL * current_a);
                tempFFR = ffdelayLine->processBlockInter(inputR * current_a);
                tempFBL = (inputL)+tempFFL + fbdelayLine->processBlockInter(tempFBL * current_a);;
                tempFBR = (inputR)+tempFFR + fbdelayLine->processBlockInter(tempFBR * current_a);
                leftOut[i] = scale * (tempFBL);
                rightOut[i] = scale * (tempFBR);

            }
            ffdelayLine->resetSmoother();
            fbdelayLine->resetSmoother();
        }
        else
        {
            for(int i = 0; i < current_bs; i++)
            {
                inputL = leftIn[i];
                inputR = rightIn[i];
                tempFFL = ffdelayLine->processBlockInter(inputL   * current_a);
                tempFFR =  ffdelayLine->processBlockInter(inputR  * current_a);
                tempFBL =  (inputL) +   tempFFL   +    fbdelayLine->processBlockInter(tempFBL * current_a);
                tempFBR =  (inputR) +   tempFFR   +   fbdelayLine->processBlockInter(tempFBR * current_a);
                leftOut[i]   = scale * (tempFBL);
                rightOut[i]  = scale * (tempFBR);
                
                
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
    const float scale = 0.1;
};
class SIMDUniversalComb {
public:
    SIMDUniversalComb()
    {
        ffdelayLine = std::make_unique<SIMDDelayLine>();
        
        fbdelayLine = std::make_unique<SIMDDelayLine>();
         
        smoother = std::make_unique<block_Smoothing>();
    }

    ~SIMDUniversalComb() {}

    void prepare(int delayLineSize, int maxBlockSize, double cur_sampleRate, int channels) noexcept
    {
        ffdelayLine->prepare(delayLineSize, maxBlockSize, channels);
      
        fbdelayLine->prepare(delayLineSize, maxBlockSize, channels);
      
        smoother->prepare(maxBlockSize);
        this->sampleRate = cur_sampleRate;
    }

    void setFrequency(float hz)
    {
        const float period = 1.f / hz;
        float delayTimeSamples = period * sampleRate;
        ffdelayLine->setParams(delayTimeSamples);
      
        fbdelayLine->setParams(delayTimeSamples);
       
    }

    void setLinGain(float g)
    {
        a = g;
        current_a = g;
        smoother->smooth(a, current_a);
    }

    void process(juce::AudioBuffer<float>& buffer, int bs)
    {
        current_bs = bs;

        // Get pointers to input and output channels
        const float* leftIn = buffer.getReadPointer(0);
        const float* rightIn = buffer.getReadPointer(1);
        float* leftOut = buffer.getWritePointer(0);
        float* rightOut = buffer.getWritePointer(1);

        // Process samples in groups of 4
        for (int i = 0; i < current_bs; i += 4)
        {
            // Load 4 input samples for left and right channels into SIMD registers
            __m128 leftInput = _mm_loadu_ps(&leftIn[i]);
            __m128 rightInput = _mm_loadu_ps(&rightIn[i]);

            // Process left channel samples
            __m128 scaledLeft = _mm_mul_ps(leftInput, _mm_set1_ps(current_a));
            ffdelayLine->write(scaledLeft);
            __m128 tempFBL = _mm_add_ps(leftInput, ffdelayLine->read());
            tempFBL = _mm_add_ps(tempFBL, fbdelayLine->read());
            tempFBL = _mm_mul_ps(tempFBL, _mm_set1_ps(current_a));
            fbdelayLine->write(tempFBL);
            
           
            //__m128 scaledLeftFB = _mm_add_ps(tempFBL, leftInput);
            __m128 scaledLeftOutput = _mm_mul_ps(tempFBL, _mm_set1_ps(scale));
            _mm_storeu_ps(&leftOut[i], scaledLeftOutput);

            // Process right channel samples
            __m128 scaledRight = _mm_mul_ps(rightInput, _mm_set1_ps(current_a));
             ffdelayLine->write(scaledRight);
            __m128 tempFBR = _mm_add_ps(rightInput, ffdelayLine->read());
            tempFBR = _mm_add_ps(tempFBR, fbdelayLine->read());
            tempFBR = _mm_mul_ps(tempFBR, _mm_set1_ps(current_a));
            fbdelayLine->write(tempFBR);
            //__m128 scaledRightFB = _mm_add_ps(scaledRightFB, rightInput);
            __m128 scaledRightOutput = _mm_mul_ps(tempFBR, _mm_set1_ps(scale));
            _mm_storeu_ps(&rightOut[i], scaledRightOutput);
        }
    }

private:
    std::unique_ptr<SIMDDelayLine> ffdelayLine;
    std::unique_ptr<SIMDDelayLine> fbdelayLine;
    std::unique_ptr<block_Smoothing> smoother;

    float sampleRate = 0;
    float a = 0;
    float current_a = 0;
    int current_bs = 0;

    const float scale = 0.1;
};
