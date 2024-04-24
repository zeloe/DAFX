/*
  ==============================================================================

    NaturalSoundingCombFilter.h
    Created: 1 Jun 2023 2:48:53pm
    Author:  Onez

  ==============================================================================
*/

#pragma once
#include "../Utility/Interpolation.h"


class NatComb {
public:
    NatComb(){}
    ~NatComb(){}
    // A low Size is better like delayLineSize = 10
    void prepare(int delayLineSize)
    {
        delayLine.setSize(1,delayLineSize + 1);
       
        current_g  = 0;
        size = delayLineSize;
        current_delay = 0;
        current_fract = 0;
    }
    //delay should be smaller than size
    //g should be from -0.99 to 0.99
    void setParams(float d,float g)
    {
        float temp = (int)floor(d);
        fract = d - temp;
        delay = int(d);
        gain = g;
    }
    
    
    void process(float* input, const int bs)
    {
        const float* delRead = delayLine.getReadPointer(0);
        float* delWrite = delayLine.getWritePointer(0);
        if(current_delay != delay)
        {
            
            inc_delay = (delay - current_delay) / bs;
            inc_fract = (fract - current_fract) / bs;
            
            for(int i = 0; i < bs; i++)
            {
                current_delay += inc_delay;
                current_fract += inc_fract;
                readPointer = (writePointer - current_delay + size);
                readPointer = (readPointer) % size;
                if(readPointer - 3 < 0)
                {
                    readPointer = readPointer + size;
                }
                const float y0 = delRead[(readPointer - 3) % size];
                const float y1 = delRead[(readPointer - 2) % size];
                const float y2 = delRead[(readPointer - 1) % size];
                const float y3 = delRead[(readPointer) % size];
                //less artifacts with higher interpolation methods
                const float x_est = cubicInterpolation(y0, y1, y2, y3, current_fract);
                writePointer++;
                if (writePointer >= size)
                {
                    writePointer = 0;
                }
                yn  = b_0 * x_est + b_1 * xhold - a_1 * yhold;
                yhold = yn;
                xhold = x_est;
                const float output = input[i] + gain * yn;
                delWrite[writePointer] = output;
                input[i] = output;
            }
            current_delay = delay;
            current_fract = fract;
            
        }
        else if(gain != current_g)
        {
            inc_g = (gain - current_g) / bs;
            
            for(int i = 0; i < bs; i++)
            {
                current_g += inc_g;
                readPointer = (writePointer - int(delay) + size) % size;
                const float y0 = delRead[readPointer];
                writePointer = (writePointer + 1) % size;
                yn  = b_0 * y0 + b_1 * xhold - a_1 * yhold;
                yhold = yn;
                xhold = y0;
                const float output = input[i] + current_g * yn;
                delWrite[writePointer] = output;
                input[i] = output;
            }
            current_g = gain;
        }
        else
        {
            for(int i = 0; i < bs; i++)
            {
                readPointer = (writePointer - int(delay) + size) % size;
                const float y0 = delRead[readPointer];
                writePointer = (writePointer + 1) % size;
                yn = b_0 * y0 + b_1 * xhold - a_1 * yhold;
                yhold = yn;
                xhold = y0;
                const float output = input[i] + gain * yn;
                delWrite[writePointer] = output;
                input[i] = output;
                }
            }
    }
    
    
    
    
    
private:
    float current_g = 0;
    float  inc_g = 0;
    float b_0 = 0.5;
    float b_1 = 0.5;
    float a_1 = 0.7;
    float g = 0.5;
    float xhold = 0;
    float yhold = 0;
    float gain = 0;
    float current_fract = 0;
    float current_delay = 0;
    float fract = 0;
    float delay = 0;
    float inc_fract = 0;
    float inc_delay = 0;
    unsigned int readPointer = 0;
    unsigned int read2 = 0;
    unsigned int writePointer = 0;
    unsigned int write2 = 0;
    int size = 0;
    juce::AudioBuffer<float> delayLine;
    float yn;
    unsigned int finalsize;
};

