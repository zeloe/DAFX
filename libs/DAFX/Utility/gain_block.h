/*
  ==============================================================================

    gain_block.h
    Created: 5 Feb 2023 11:39:34pm
    Author:  Onez

  ==============================================================================
*/


#pragma once
#include "JuceHeader.h"
#include "math.h"
class Gain_Block
{
public:
    Gain_Block()  {};
    ~Gain_Block() {};
    
    void prepare(int blocksize)
    {
        temp_gain = 0;
        bs = blocksize;
    }
    
    void setGain(float gain)
    {
        current_gain = gain;
    }
    
    void process(float* input) noexcept
    {
        if(temp_gain != current_gain)
        {
            // this works for block based processing
            gain_inc = (current_gain - temp_gain) / bs;
            for (size_t i = 0; i < bs; ++i)
            {
                temp_gain += gain_inc;
                const float gain = input[i] * temp_gain;
                input[i] = gain;
            }
            temp_gain = current_gain;
        } else {
            for (size_t i = 0; i < bs; ++i)
            {
                const float gain = input[i] * current_gain;
                input[i] = gain;
            }
        }
             
    }
private:
    float temp_gain;
    float current_gain;
    float gain_inc = 0;
    size_t bs = 0;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Gain_Block);
};
