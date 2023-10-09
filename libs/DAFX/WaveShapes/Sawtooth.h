/*
  ==============================================================================

    Sawtooth.h
    Created: 22 Apr 2023 3:03:00pm
    Author:  Onez

  ==============================================================================
*/

#pragma once
#include "JuceHeader.h"
class Sawtooth
{
public:
    Sawtooth()
    {
        // wird nur einmal in Constructor ausgeführt
        createWavetable();
    };
    ~Sawtooth(){};
    
    void createWavetable()
    {
        //code für wavetable
        waveTable.setSize (1, (int) tableSize + 1);
        waveTable.clear();
        auto* samples = waveTable.getWritePointer (0);
        for(int i = 0; i < tableSize; ++i)
        {
            samples[i] = (float(i) / float (tableSize));
        }
    }
    
    juce::AudioSampleBuffer waveTable;
private:
    const unsigned int tableSize = 512;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Sawtooth);
};
