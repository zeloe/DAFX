/*
  ==============================================================================

    IIRCombFilter.h
    Created: 8 Apr 2023 11:11:09pm
    Author:  Onez

  ==============================================================================
*/

#pragma once
#include "../Utility/Interpolation.h"
class IIRComb {
public:
    IIRComb(){}
    ~IIRComb(){}
    // A low Size is better like delayLineSize = 10
    void prepare(unsigned int delayLineSize)
    {
        delayLine.setSize(1, delayLineSize + 1);
        
        delayLine.clear();
        current_g  = 0;
        current_delay = 0;
        current_fract = 0;
        size = delayLineSize;
    }
    //delay should be smaller than size
    //g should be from -0.90 to 0.90 if you want higher values you should scale the output after the comb filter
    
    void setParams(float d,float g)
    {
        float temp = (int)floor(d);
        fract = d - temp;
        delay = int(d);
        gain = g;
    }
    
    
    void process(float* input, const int bs)
    {
        float* delayWtr = delayLine.getWritePointer(0);
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
                if(readPointer - 3 < 0 )
                {
                    readPointer  = readPointer + size;
                }
                delayWtr[writePointer] = output;
                const float y0 = delayWtr[(readPointer - 3) % size];
                const float y1 = delayWtr[(readPointer - 2) % size];
                const float y2 = delayWtr[(readPointer - 1) % size];
                const float y3 = delayWtr[(readPointer) % size];
                //less artifacts with higher interpolation methods
                const float x_est = splineInterpolation(y0, y1, y2, y3, current_fract);
                writePointer++;
                if (writePointer >= size)
                {
                    writePointer = 0;
                }
                output = input[i] + x_est * gain;
                input[i] = output;
            }
            
            current_delay = delay;
            current_fract = fract;
        } else if (current_g != gain) {
            inc_g = (gain - current_g) / bs;
            for(int i = 0; i < bs; i++)
            {
                current_g += inc_g;
                delayWtr[writePointer] = output;
                readPointer = (writePointer - int(delay) + size) % size;
                const float y0 = delayWtr[readPointer];
                output = input[i] + y0 * current_g;
                input[i] = output;
                writePointer = (writePointer + 1) % size;
                
            }
            current_g = gain;
        } else {
                for(int i = 0; i < bs; i++)
                {
                    delayWtr[writePointer] = output;
                    readPointer = (writePointer - int(delay) + size) % size;
                    const float y0 = delayWtr[readPointer];
                    output = input[i] + y0 * gain;
                    input[i] = output;
                    writePointer = (writePointer + 1) % size;

                }
            }
    }
    
    
    
    
    
private:
    float current_g = 0;
    float  inc_g = 0;
    float gain = 0;
    float current_fract = 0;
    float current_delay = 0;
    float fract = 0;
    unsigned int delay = 0;
    float inc_fract = 0;
    float inc_delay = 0;
    unsigned int readPointer = 0;
    unsigned int writePointer = 0;
    unsigned int size = 0;
    juce::AudioBuffer<float> delayLine;
    float output = 0;
};

