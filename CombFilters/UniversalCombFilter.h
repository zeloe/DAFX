/*
  ==============================================================================

    UniversalCombFilter.h
    Created: 10 Apr 2023 11:03:36am
    Author:  Onez

  ==============================================================================
*/

#pragma once
#include "../Utility/Interpolation.h"
class UniversalComb {
public:
    UniversalComb()
    {
        UniversalComb::delayLine.resize(512);
        UniversalComb::delayLine2.resize(512);
    };
    ~UniversalComb(){};
    // A low Size is better like delayLineSize = 10
    void prepare(int delayLineSize,int blocksize)
    {
        delayLine.resize(delayLineSize + 1);
        delayLine2.resize(delayLineSize + 1);
        std::vector<float>::iterator ptr;
        for(ptr = delayLine.begin(); ptr < delayLine.end(); ptr++)
        {
            *ptr = 0;
        }
        for(ptr = delayLine2.begin(); ptr < delayLine2.end(); ptr++)
        {
            *ptr = 0;
        }
        bs = blocksize;
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
    
    
    void process(float* input)
    {
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
                delayLine[writePointer] = output;
                delayLine2[writePointer] = input[i];
                const float y0 = delayLine[(readPointer) % size];
                const float y1 = delayLine[(readPointer + 1) % size];
                const float y2 = delayLine[(readPointer + 2) % size];
                const float y3 = delayLine[(readPointer + 3) % size];
                //less artifacts with higher interpolation methods
                const float x_est = splineInterpolation(y0, y1, y2, y3, current_fract);
                const float y02 = delayLine2[(readPointer) % size];
                const float y12 = delayLine2[(readPointer + 1) % size];
                const float y22 = delayLine2[(readPointer + 2) % size];
                const float y32 = delayLine2[(readPointer + 3) % size];
                //less artifacts with higher interpolation methods
                const float x_est2 = splineInterpolation(y02, y12, y22, y32, current_fract);
                writePointer++;
                if (writePointer >= size)
                {
                    writePointer = 0;
                }
                //  fb with *-1 different results
                output = input[i] + x_est * gain * 0.5 + x_est2 * gain;
                input[i] = output;
            }
            
            current_delay = delay;
            current_fract = fract;
        } else if (current_g != gain) {
            inc_g = (gain - current_g) / bs;
            for(int i = 0; i < bs; i++)
            {
                current_g += inc_g;
                delayLine[writePointer] = output;
                delayLine2[writePointer] = input[i];
                readPointer = (writePointer - int(delay) + size) % size;
                const float y0 = delayLine[readPointer];
                const float y02 = delayLine2[readPointer];
                //  fb with *-1 different results
                output = input[i] + y0 * current_g  * 0.5 + y02 * current_g;
                input[i] = output;
                writePointer = (writePointer + 1) % size;
                
            }
            current_g = gain;
        } else {
                for(int i = 0; i < bs; i++)
                {
                    delayLine[writePointer] = output;
                    delayLine2[writePointer] = input[i];
                    readPointer = (writePointer - int(delay) + size) % size;
                    const float y0 = delayLine[readPointer];
                    const float y02 = delayLine2[readPointer];
                    //  fb with *-1 different results
                    output = input[i] + y0 * gain * 0.5 + y02 * gain;
                    input[i] = output;
                    writePointer = (writePointer + 1) % size;

                }
            }
    }
    
    
    
    
    
private:
    size_t bs = 0;
    float current_g = 0;
    float  inc_g = 0;
    float gain = 0;
    float current_fract = 0;
    float current_delay = 0;
    float fract = 0;
    float delay = 0;
    float inc_fract = 0;
    float inc_delay = 0;
    unsigned int readPointer = 0;
    unsigned int writePointer = 0;
    int size = 0;
    float output = 0;
    std::vector<float> delayLine;
    std::vector<float> delayLine2;
};

