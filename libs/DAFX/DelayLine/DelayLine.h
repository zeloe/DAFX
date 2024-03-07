/*
  ==============================================================================

    DelayLine.h
    Created: 13 Apr 2023 11:08:39am
    Author:  Onez

  ==============================================================================
*/

#pragma once
#include "JuceHeader.h"
#include "../Utility/Interpolation.h"
#include "../Utility/Block_Smoothing.h"
class DelayLine
{
public:
    DelayLine()
    {
        smoother_DelayTime = std::make_unique<block_Smoothing>();
        smoother_Fraction = std::make_unique<block_Smoothing>();
    };
    ~DelayLine(){};
    
    void prepare(int delayLineSize, int maxBlocksize)
    {
        delBuffer.setSize(1,delayLineSize);
        size = delayLineSize;
        current_bs = maxBlocksize;
    }
    
    void setParams(float del)
    {
        float temp = floor(del);
        fract = del - temp;
        delay = int(del);
        smoother_DelayTime->calcCoeff(delay,current_delay,current_bs);
        smoother_Fraction->calcCoeff(fract,current_fract,current_bs);
    }
    
    void incrementDelayLine()
    {
        writePointer = (writePointer + 1) % size;
    }
    
    void resetDLine()
    {
        writePointer = 0;
        readPointer = 0;
    }
    
    
    void process(juce::AudioBuffer<float>& buffer, int bs)
    {
        const float* input = buffer.getReadPointer(0);
        float* output = buffer.getWritePointer(0);
        const float* delRead = delBuffer.getReadPointer(0);
        float* delWrite = delBuffer.getWritePointer(0);
        current_bs = bs;
        if (smoother_DelayTime->isSmoothing == true)
        {
            for(int i = 0; i < bs; i++)
            {
                 
                readPointer = (writePointer - smoother_DelayTime->smoothing(current_delay));
                delWrite[writePointer] = input[i];
                if (readPointer - 3 < 0)
                {
                    readPointer += size;
                }
                const float y0 = delRead[(readPointer - 3 + size) % size];
                const float y1 = delRead[(readPointer - 2 + size) % size];
                const float y2 = delRead[(readPointer - 1 + size) % size];
                const float y3 = delRead[(readPointer + size) % size];
                
                const float t_output = cubicInterpolation(y0, y1, y2, y3,smoother_Fraction->smoothing(current_fract));
                output[i] = t_output;
                this->incrementDelayLine();
               
            }
            smoother_DelayTime->resetSmoother();
            smoother_Fraction->resetSmoother();
        }
        else
        {
            for(int i = 0; i < bs; i++)
            {
                delWrite[writePointer] = input[i];
                readPointer = (writePointer - int(current_delay) + size) % size;
                const float y0 = delRead[readPointer];
                output[i] = y0;
                this->incrementDelayLine();
            }
        }
    }
    
    
    
    float process(float input, float bs)
    {
        
        const float* delRead = delBuffer.getReadPointer(0);
        float* delWrite = delBuffer.getWritePointer(0);
        current_bs = bs;
        if (smoother_DelayTime->isSmoothing == true)
        {
            
            readPointer = (writePointer - smoother_DelayTime->smoothing(current_delay));
            delWrite[writePointer] = input;
            if (readPointer - 3 < 0)
            {
                readPointer += size;
            }
            const float y0 = delRead[(readPointer - 3 + size) % size];
            const float y1 = delRead[(readPointer - 2 + size) % size];
            const float y2 = delRead[(readPointer - 1 + size) % size];
            const float y3 = delRead[(readPointer + size) % size];
                
            const float t_output = cubicInterpolation(y0, y1, y2, y3, smoother_Fraction->smoothing(current_fract));
                
            this->incrementDelayLine();
            return t_output;
               
        }
        else
        {
            delWrite[writePointer] = input;
            readPointer = (writePointer - int(delay) + size) % size;
            const float y0 = delRead[readPointer];
            this->incrementDelayLine();
            return y0;
        }
    }
    std::unique_ptr<block_Smoothing> smoother_DelayTime;
    std::unique_ptr<block_Smoothing> smoother_Fraction;
private:
    juce::AudioBuffer<float> delBuffer;
    
    int current_bs = 0;
    float fract = 0;
    float delay = 0;
    float current_fract = 0;
    float current_delay = 0;
    
    unsigned int readPointer = 0;
    unsigned int writePointer = 0;
    
    unsigned int size = 0;
};
