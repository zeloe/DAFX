/*
  ==============================================================================

    Hanning.h
    Created: 29 May 2023 11:36:42am
    Author:  Onez

  ==============================================================================
*/

#pragma once
#include "JuceHeader.h"
class UniSine
{
public:
    UniSine()
    {
        // wird nur einmal in Constructor ausgeführt
        createWavetable();
    };
    ~UniSine(){};
    
    void createWavetable()
    {
        //code für wavetable
        waveTable.setSize (1, (int) tableSize + 1);
        waveTable.clear();
        auto* samples = waveTable.getWritePointer (0);
        for(int i = 0; i < tableSize; ++i)
        {
            samples[i] = std::sin(M_PI * float(i) / float(tableSize));
        }
    }
    
    juce::AudioSampleBuffer waveTable;
private:
    const unsigned int tableSize = 512;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (UniSine);
};
