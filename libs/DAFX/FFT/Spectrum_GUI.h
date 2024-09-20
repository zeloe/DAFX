

#ifndef SPECTRUMGUI_H
#define SPECTRUMGUI_H
#include <JuceHeader.h>
#include "../Utility/FIFO.h"
#include "FFT.h"
class Spectrum_GUI : public juce::Component,  public juce::Timer
{
public:
    
    Spectrum_GUI() {
       
        fft = std::make_unique<FFT<float>>(FFTSIZE);
        
        counter  = 0;
        TimeDomainBuffer.resize(FFTSIZE);
        auto* data = TimeDomainBuffer.data();
        for(int i = 0; i < FFTSIZE; i++) {
            data[i].real = (0.f);
            data[i].imag = (0.f);
        }
        nextFFTBlockReady = false;
        hamming.setSize(1,FFTSIZE);
        auto write = hamming.getWritePointer(0);
        for(int i = 0; i < FFTSIZE; i++) {
            write[i] = 0.54f - 0.46f * cos(juce::MathConstants<float>::twoPi * i / FFTSIZE);
        }
        
        
        
        scopeData.setSize(1,FFTSIZE);
        scopeData.clear();
        startTimerHz(30);
    }
    
    
   
    
    void paint(juce::Graphics &g) override {
        g.setColour(juce::Colours::white);
      
        drawFrame(g);
    }
    
    
    void resized() override {
        
    }
    void computeScopeData()
    {
        auto mindB = -60.0f;
        auto maxdB = 0.0f;
        int scopeSize = FFTSIZE / 2;// Define the scope size, likely half the FFT size with
        auto* fftData = TimeDomainBuffer.data();  // FFT data pointer

        for (int i = 0; i < scopeSize; ++i)                         // [3]
        {
            // Skewing the X axis to distribute frequency bins logarithmically
            auto skewedProportionX = 1.0f - std::exp (std::log (1.0f - (float) i / (float) scopeSize) * 0.2f);

            // Map the skewed proportion to the FFT data index
            auto fftDataIndex = juce::jlimit (0, FFTSIZE / 2, (int) (skewedProportionX * (float) FFTSIZE * 0.5f));

            // Get the magnitude at this FFT index, convert it to decibels
            auto mag = fftData[fftDataIndex].real * fftData[fftDataIndex].real
                     + fftData[fftDataIndex].imag * fftData[fftDataIndex].imag;

            // Convert magnitude to decibels
            auto dB = juce::Decibels::gainToDecibels(mag)
                    - juce::Decibels::gainToDecibels((float)FFTSIZE);

            // Clamp the decibel value between mindB and maxdB, then map to 0.0 - 1.0 range
            auto level = juce::jmap(juce::jlimit(mindB, maxdB, dB), mindB, maxdB, 0.0f, 1.0f);

            
            scopeData.setSample(0, i, level);
        }
    }
    
    void Spectrogram() {
        fft->perform(TimeDomainBuffer.data(),FFTSIZE,fft->twiddle.data());
       
        computeScopeData();
  
    }
    
    void drawFrame (juce::Graphics& g)
    {
        auto* magDat = scopeData.getWritePointer(0);
        auto width  = getLocalBounds().getWidth();
        auto height = getLocalBounds().getHeight();

        // Create a path object
        juce::Path spectrumPath;

        // Move to the first point of the spectrum path
        spectrumPath.startNewSubPath(0, juce::jmap(magDat[0], 0.0f, 1.0f, (float) height, 0.0f));

        // Iterate through the FFT data and add points to the path
        for (int i = 1; i < FFTSIZE / 2; ++i)
        {
            float x = (float) juce::jmap(i, 0, FFTSIZE / 2 - 1, 0, width);
            float y = juce::jmap(magDat[i], 0.0f, 1.0f, (float) height, 0.0f);

            // Add the current point to the path
            spectrumPath.lineTo(x, y);
        }

        // Optional: Set the color and stroke thickness for the path
        g.setColour(juce::Colours::white);
        g.strokePath(spectrumPath, juce::PathStrokeType(2.0f));  // Stroke width of 2.0 pixels
    }

    void timerCallback() override
    {
        if(nextFFTBlockReady) {
            Spectrogram();
            nextFFTBlockReady = false;
            repaint();
        }
    }

        void write(float dataFromAudioThread)
        {
            if (counter >= FFTSIZE)
            {
                nextFFTBlockReady = true;
                counter = 0;
            }

            // Read from FIFO and store data in TimeDomainBuffer
            auto* data = TimeDomainBuffer.data();
            auto* window = hamming.getWritePointer(0);
            data[counter].real = dataFromAudioThread * window[counter];
            data[counter].imag = 0.f;
            ++counter;
        }
    
    
    std::vector<typename FFT<float>::ComplexT> TimeDomainBuffer;
    
private:
    const int FFTSIZE = 2048;
    int counter = 0;
    
    std::unique_ptr<FFT<float>> fft;
    juce::AudioBuffer<float> hamming;
    bool nextFFTBlockReady = false;
    juce::AudioBuffer<float> scopeData;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Spectrum_GUI)
};
#endif //SPECTRUMGUI_H
