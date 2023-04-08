/*
  ==============================================================================

    IIRCombFilter.h
    Created: 8 Apr 2023 11:11:09pm
    Author:  Onez

  ==============================================================================
*/

#pragma once
#pragma once
class IIRComb {
public:
    IIRComb()
    {
        IIRComb::delayLine.resize(512);
    };
    ~IIRComb(){};
    // A low Size is better like delayLineSize = 10
    void prepare(int delayLineSize,int blocksize)
    {
        delayLine.resize(delayLineSize + 1);
        int index = 0;
        std::vector<float>::iterator ptr;
        for(ptr = delayLine.begin(); ptr < delayLine.end(); ptr++)
        {
            *ptr = 0;
            index++;
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
        if(current_g != gain || current_delay != delay)
        {
            inc_g = (gain - current_g) / bs;
            inc_delay = (delay - current_delay) / bs;
            inc_fract = (fract - current_fract) / bs;
            for(int i = 0; i < bs; i++)
            {
                current_g += inc_g;
                current_delay += inc_delay;
                current_fract += inc_fract;
                readPointer = writePointer - current_delay;
                delayLine[writePointer] = output;
                if (readPointer - 1 < 0)
                {
                    readPointer += size;
                }
                const float y0 = delayLine[(readPointer - 1) % size];
                const float y1 = delayLine[(readPointer) % size];
                
                const float x_est = (1.0 - current_fract) * y0 + current_fract * y1;
                writePointer++;
                if (writePointer >= size)
                {
                    writePointer = 0;
                }
                output = input[i] + x_est * current_g;
                input[i] = output;
            }
            current_g = gain;
            current_delay = delay;
            current_fract = fract;
        } else {
            for(int i = 0; i < bs; i++)
            {
                delayLine[writePointer] = output;
                readPointer = (writePointer - int(delay) - 1 + size) % size;
                const float y0 = delayLine[readPointer];
                output = input[i] + y0 * gain;
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
};

