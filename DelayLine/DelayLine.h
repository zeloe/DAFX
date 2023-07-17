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
class DelayLine
{
public:
    DelayLine()
    {
        DelayLine::delayBuf.resize(512);
    };
    ~DelayLine(){};
    
    void prepare(int delayLineSize, int blocksize)
    {
        delayBuf.resize(delayLineSize);
        std::vector<float>::iterator ptr;
        
        for(ptr = delayBuf.begin(); ptr < delayBuf.end(); ptr++)
        {
            *ptr  = 0;
        }
        bs = blocksize;
        current_delay = 0;
        size = delayLineSize;
        output = 0;
    }
    
    void setParams(float del)
    {
        float temp = floor(del);
        fract = del - temp;
        delay = int(del);
    }
    
    
    void process(float* input)
    {
        if (current_delay != delay)
        {
            inc_delay =(delay - current_delay) / bs;
            inc_fract = (fract - current_fract) / bs;
            
            for(int i = 0; i < bs; i++)
            {
                current_delay += inc_delay;
                current_fract += inc_fract;
                readPointer = (writePointer - current_delay);
                delayBuf[writePointer] = input[i];
                if (readPointer - 3 < 0)
                {
                    readPointer += size;
                }
                const float y0 = delayBuf[(readPointer - 3 + size) % size];
                const float y1 = delayBuf[(readPointer - 2 + size) % size];
                const float y2 = delayBuf[(readPointer - 1 + size) % size];
                const float y3 = delayBuf[(readPointer + size) % size];
                
                output = cubicInterpolation(y0, y1, y2, y3, current_fract);
                input[i] = output;
                writePointer = (writePointer + 1) % size;
               
            }
            current_delay = delay;
            current_fract = fract;
        }
        else
        {
            for(int i = 0; i < bs; i++)
            {
                delayBuf[writePointer] = input[i];
                readPointer = (writePointer - int(delay) + size) % size;
                const float y0 = delayBuf[readPointer];
                input[i] = y0;
                writePointer = (writePointer + 1) % size;
            }
        }
    }
    
private:
    std::vector<float> delayBuf;
    size_t bs = 0;
    float fract = 0;
    float delay = 0;
    float current_fract = 0;
    float current_delay = 0;
    float inc_fract = 0;
    float inc_delay = 0;
    unsigned int readPointer = 0;
    unsigned int writePointer = 0;
    float output = 0;
    int size = 0;
};
