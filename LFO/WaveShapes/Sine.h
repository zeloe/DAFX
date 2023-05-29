/*
  ==============================================================================

    Sine.h
    Created: 22 Apr 2023 3:02:31pm
    Author:  Onez

  ==============================================================================
*/

#pragma once
#include "JuceHeader.h"
class Sine
{
public:
    Sine()
    {
        // wird nur einmal in Constructor ausgeführt
        createWavetable();
    };
    ~Sine(){};
    
    void createWavetable()
    {
        //code für wavetable
        waveTable.setSize (1, (int) tableSize + 1);
        waveTable.clear();
        auto* samples = waveTable.getWritePointer (0);
        for(int i = 0; i < tableSize; ++i)
        {
            samples[i] = std::sin(2 * M_PI * float(i) / float(tableSize));
        }
    }
    
    juce::AudioSampleBuffer waveTable;
private:
    const unsigned int tableSize = 512;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Sine);
};
