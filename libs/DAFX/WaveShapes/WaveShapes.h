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
        
        auto index0 = (unsigned int)currentIndex;        // Base index (integer part of currentIndex)
        auto frac = currentIndex - (float)index0;        // Fractional part of currentIndex
        
        // Use SIMD to handle the 4-point interpolation
        juce::dsp::SIMDRegister<float> fracSIMD = juce::dsp::SIMDRegister<float>::fromNative({
            frac, frac, frac, frac
        });
        
        // Handle wrapping at the end of the wavetable
        auto wrapIndex = [this](unsigned int index) {
            return index % this->tableSize;  // Capture 'this' if 'tableSize' is a member variable
        };
        
        // Load the table values for 4 consecutive points for cubic interpolation
        const float* table = waveTable.getReadPointer(0);
        T value0 = juce::dsp::SIMDRegister<float>::fromNative({
            table[wrapIndex(index0)],
            table[wrapIndex(index0 + 1)],
            table[wrapIndex(index0 + 2)],
            table[wrapIndex(index0 + 3)]
        });
        
        T value1 = juce::dsp::SIMDRegister<float>::fromNative({
            table[wrapIndex(index0 + 1)],
            table[wrapIndex(index0 + 2)],
            table[wrapIndex(index0 + 3)],
            table[wrapIndex(index0 + 4)]
        });
        
        T value2 = juce::dsp::SIMDRegister<float>::fromNative({
            table[wrapIndex(index0 + 2)],
            table[wrapIndex(index0 + 3)],
            table[wrapIndex(index0 + 4)],
            table[wrapIndex(index0 + 5)]
        });
        
        T value3 = juce::dsp::SIMDRegister<float>::fromNative({
            table[wrapIndex(index0 + 3)],
            table[wrapIndex(index0 + 4)],
            table[wrapIndex(index0 + 5)],
            table[wrapIndex(index0 + 6)]
        });
        
        // Perform SIMD-aware cubic interpolation
        T currentSample = cubic(value0, value1, value2, value3, fracSIMD);
        
        // Update the current index, and ensure it wraps around the table size
        currentIndex += frequency;
        if (currentIndex >= tableSize) {
            currentIndex -= tableSize;
        }
        
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

class WaveTables
{
public:
   
    WaveTables(int index)
    {
        createWavetable(index);
    }
   
    ~WaveTables()
    {


    }

     
    
    void createWavetable(int index)
    {
        const int tableSize = 4096;
        waveTable.setSize(1, (int)tableSize + 1);
        waveTable.clear();
        auto* samples = waveTable.getWritePointer(0);
        switch(index)
        {
            case 0: // sine
            {
                for (int i = 0; i < tableSize; ++i)
                {
                    samples[i] = std::sin(juce::MathConstants<float>::twoPi * float(i) / float(tableSize));
                }
                break;
            }
            case 1: // some other waveform
            {
                bool over = false;
                unsigned int count = 0;
                unsigned int halfsize = tableSize / 2;
                for (int i = 0; i < tableSize; ++i)
                {
                    if (!over)
                    {
                        samples[i] = (float(count) / float(tableSize / 2));
                        if (count >= halfsize) // Changed to >=
                        {
                            over = true;
                        }
                        count++;
                    }
                    else
                    {
                        samples[i] = (float(count) / float(tableSize / 2));
                        count--;
                    }
                }
                break;
            }
            case 2: // Sawtooth
            {
                for (int i = 0; i < tableSize; ++i)
                {
                    samples[i] = (float(i) / float(tableSize));
                }
                break;
            }
            case 3: // Square
            {
                bool over = false; // Reinitialize over
                unsigned int count = 0; // Reinitialize count
                unsigned int halfsize = tableSize / 2;
                for (int i = 0; i < tableSize; ++i)
                {
                    if (!over)
                    {
                        samples[i] = 1;
                        count++;
                        if (count >= halfsize) // Changed to >=
                        {
                            over = true;
                        }
                    }
                    else
                    {
                        samples[i] = -1;
                    }
                }
                break;
            }
            case 4: // Noise
            {
                juce::Random rand;
                for (int i = 0; i < tableSize; ++i)
                {
                    samples[i] = rand.nextFloat();
                }
                break;
            }
            case 5: // PinkNoise
            {
                // Implement pink noise generation here
                break;
            }
        }

    }
   
    juce::AudioSampleBuffer waveTable;
};
#endif // WaveTableProcessor
