/*
  ==============================================================================

    Hanning.h
    Created: 29 May 2023 11:36:42am
    Author:  Onez

  ==============================================================================
*/

#pragma once
#ifndef WaveTableProcessor_H
#define WaveTableProcessor_H
#include "JuceHeader.h"

template <typename T>
class WaveTableProcessor
{
public:
    WaveTableProcessor()
    {
        currentIndex = 0.0f;
        createWavetable();
    }

    ~WaveTableProcessor()
    {


    }
    
    void createWavetable()
    {
        waveTable.setSize(1, (int)tableSize + 1);
        waveTable.clear();
        auto* samples = waveTable.getWritePointer(0);
        for (unsigned int i = 0; i < tableSize; ++i)
        {
            samples[i] = std::sin(juce::MathConstants<float>::twoPi * float(i) / float(tableSize));
        }
    }
     
    void setFrequency(std::atomic<float>& freq) {
        frequency = freq.load();
        return;
    }
    
    void getWaveTable(juce::AudioBuffer<float>& newWaveTable) {
        waveTable = newWaveTable;
        return;
    }
    
    T cubic(T y0, T y1, T y2, T y3, T x) {
            const T a0 = y3 - y2 - y0 + y1;
            const T a1 = y0 - y1 - a0;
            const T a2 = y2 - y0;
            const T a3 = y1;
            return (a0 * x * x * x + a1 * x * x + a2 * x + a3);
    }
    
    T process() {

        auto index0 = (unsigned int)currentIndex;
        auto index1 = index0 + 1;
        auto index2 = index0 + 2;
        auto index3 = index0 + 3;
        auto frac = currentIndex - (float)index0;
        
        const float * table = waveTable.getReadPointer(0);
        
        T value0 = juce::dsp::SIMDRegister<float>::fromNative({ table[index0 % tableSize], table[(index0 + 1) % tableSize],table[(index0 + 2) % tableSize],table[(index0 + 3) % tableSize] });
        T value1 = juce::dsp::SIMDRegister<float>::fromNative({ table[index1 % tableSize], table[(index1 + 1) % tableSize],table[(index1 + 2) % tableSize],table[(index1 + 3) % tableSize] });
        T value2 = juce::dsp::SIMDRegister<float>::fromNative({ table[index2 % tableSize], table[(index2 + 1) % tableSize],table[(index2 + 2) % tableSize],table[(index2 + 3) % tableSize] });
        T value3 = juce::dsp::SIMDRegister<float>::fromNative({ table[index3 % tableSize], table[(index3 + 1) % tableSize],table[(index3 + 2) % tableSize],table[(index3 + 3) % tableSize] });
        T currentSample = cubic(value0, value1, value2, value3, frac);
        currentIndex += frequency;

        if ((currentIndex) > (float)tableSize)
            currentIndex -= (float)tableSize;

        return currentSample;
        
       
       
    }
private:
    juce::AudioSampleBuffer waveTable;
    float tempRate = 44100;
    float currentIndex = 0.0;
    unsigned const int tableSize = 4096;
    float phase = 0;
    float temp = 0;
    float temp2 = 0;
    float frequency = 0;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(WaveTableProcessor)
};

class SineTable
{
public:
    SineTable(const SineTable&) = delete;
    SineTable& operator=(const SineTable&) = delete;
    SineTable(SineTable&&) = default;
   
    SineTable()
    {
        createWavetable();
    }
   
    ~SineTable()
    {


    }

     
    
    void createWavetable()
    {
        const int tableSize = 4096;
        waveTable.setSize(1, (int)tableSize + 1);
        waveTable.clear();
        auto* samples = waveTable.getWritePointer(0);
        for (int i = 0; i < tableSize; ++i)
        {
            samples[i] = std::sin(juce::MathConstants<float>::twoPi * float(i) / float(tableSize));
        }
        return;
    }

   
    juce::AudioSampleBuffer waveTable;
};
#endif // WaveTableProcessor
