/*
  ==============================================================================

    WaveTableOscillator.h
    Created: 22 Apr 2023 3:15:11pm
    Author:  Onez

  ==============================================================================
*/

#pragma once
#include "JuceHeader.h"
#include "../Utility/Interpolation.h"
template<typename WaveShape>
class WaveTableOscillator
{
public:
    WaveTableOscillator(std::shared_ptr<WaveShape> waveShape) :
    waveShape(waveShape),
    //pointer ownership
    tableSize(waveShape->waveTable.getNumSamples() - 1),
    wavetable(&waveShape->waveTable)
    {};
    
    ~WaveTableOscillator(){} ;
    
    void prepare(float sampleRate, int blocksize)
    {
        //prepare sampleRate
        m_sampleRate = sampleRate;
        bs = blocksize;
        lastTableDelta = 0.0f;
    }
    
    
    void setFrequency (float frequency)
    {
        //get the frequency for OSC
        auto tableSizeOverSampleRate = (float) tableSize / m_sampleRate;
        tableDelta = frequency * tableSizeOverSampleRate;
    }
    
    float getNextSample() noexcept
    {
        if(tableDelta != lastTableDelta)
        {
            increment = (tableDelta - lastTableDelta) / bs;
            for(int i = 0; i < bs; i++)
            {
                tableDelta += increment;
                auto index0 = (unsigned int) currentIndex;
                auto index1 = index0 + 1;
                auto index2 = index0 + 2;
                auto index3 = index0 + 3;
                auto frac = currentIndex - (float) index0;
                
                auto* table = wavetable->getReadPointer (0);
                auto value0 = table[index0 % tableSize];
                auto value1 = table[index1 % tableSize];
                auto value2 = table[index2 % tableSize];
                auto value3 = table[index3 % tableSize];
                currentSample = cubicInterpolation(value0, value1, value2, value3, frac);
                if ((currentIndex += tableDelta) > (float) tableSize)
                    currentIndex -= (float) tableSize;
            }
            lastTableDelta = tableDelta;
        }
        else
        {
            for(int i = 0; i < bs; i++)
            {
                auto index0 = (unsigned int) currentIndex;
                auto index1 = index0 + 1;
                auto index2 = index0 + 2;
                auto index3 = index0 + 3;
                auto frac = currentIndex - (float) index0;
                auto* table = wavetable->getReadPointer (0);
                auto value0 = table[index0 % tableSize];
                auto value1 = table[index1 % tableSize];
                auto value2 = table[index2 % tableSize];
                auto value3 = table[index3 % tableSize];
                currentSample = cubicInterpolation(value0, value1, value2, value3, frac);
                if ((currentIndex += tableDelta) > (float) tableSize)
                    currentIndex -= (float) tableSize;
            }
        }
        
        return currentSample;
    }
    
    
    
    
private:
    //private Variablen
    float m_sampleRate = 0;
    float currentIndex = 0.0, tableDelta = 0.0;
    float lastTableDelta = 0.0;
    float increment = 0.0;
    float bs = 0;
    std::shared_ptr<WaveShape> waveShape;
    const int tableSize;
    juce::AudioSampleBuffer* wavetable;
    float currentSample = 0.0;
    // check last index for fade in and out
    unsigned int lastIndex = 0;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (WaveTableOscillator);
};
