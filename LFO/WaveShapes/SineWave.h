/*
  ==============================================================================

    Waveshapes.h
    Created: 8 Feb 2023 12:10:19am
    Author:  Onez

  ==============================================================================
*/

#pragma once
#include "JuceHeader.h"
#include "math.h"
class SineWave
{
public:
    SineWave(int size)
    {
        size = size;
        waveshape.resize(size);
        std::vector<float>::iterator ptr;
        int index = 0;
        for(ptr = waveshape.begin(); ptr < waveshape.end(); ptr++)
        {
            *ptr =(std::sin(2.0 * M_PI * (float(index) / float(size))));
            index++;
        }
        
    };
    ~SineWave() {};
    
    void prepare(float sampleRate, int blocksize) noexcept
    {
        m_sampleRate = sampleRate;
        phase = 0;
        temp_freq = 0;
        bs = blocksize;
        result = 0;
    }
    void setFreq(float freq)
    {
        current_freq = freq;
    }
    
    float process_waveTable(float freq)
    {
        
        phase += size / (m_sampleRate / freq);

        if ( phase >= size - 1.0)
        {
            phase -=size;
        }
        remainder = phase - floor(phase);
        const float output = (float) ((1.0-remainder) * waveshape[1+ (long) phase] + remainder * waveshape[2+(long) phase]);
        return output;
    }
    
    float process() noexcept
    {
         
        if(temp_freq != current_freq)
        {
            // this works for block based processing
            freq_inc = (current_freq - temp_freq) / bs;
            for (size_t i = 0; i < bs; ++i)
            {
                temp_freq += freq_inc;
                result = process_waveTable(temp_freq);
            }
            temp_freq = current_freq;
        } else {
            for (size_t i = 0; i < bs; ++i)
            {
                result = process_waveTable(temp_freq);
               
            }
        }
    return result;
    }
    
private:
    float phase = 0;
    float frequency;
    float temp_freq;
    float current_freq;
    float freq_inc = 0;
    float increment = 0;
    float m_sampleRate = -1;
    std::vector<float> waveshape;
    float remainder = 0;
    float result = 0;
    float size = 512;
    size_t bs = 0;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SineWave);
};
