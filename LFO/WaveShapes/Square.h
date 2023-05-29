/*
  ==============================================================================

    Square.h
    Created: 22 Apr 2023 3:02:40pm
    Author:  Onez

  ==============================================================================
*/

#pragma once
#include "JuceHeader.h"
class Square
{
public:
    Square()
    {
        // wird nur einmal in Constructor ausgeführt
        createWavetable();
    };
    ~Square(){};
    
    void createWavetable()
    {
        //code für wavetable
        waveTable.setSize (1, (int) tableSize + 1);
        waveTable.clear();
        auto* samples = waveTable.getWritePointer (0);
        bool over = false;
        unsigned int count = 0;
        unsigned int halfsize = tableSize / 2;
        for(int i = 0; i <  tableSize; ++i)
        {
            if(over == false)
            {
                samples[i] = 1;
                count++;
                if (count > halfsize)
                {
                    over = true;
                }
            } else
            {
                samples[i] = - 1;
            }
        }
    }
    
    juce::AudioSampleBuffer waveTable;
private:
    const unsigned int tableSize = 512;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Square);
};
