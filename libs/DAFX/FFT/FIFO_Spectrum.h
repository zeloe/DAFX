
#ifndef FIFOSPECTRUM_H
#define FIFOSPECTRUM_H



#pragma once
#include "JuceHeader.h"
#include "Spectrum_GUI.h"
template <typename T>
class FIFOSpectrum {
    
public:
    FIFOSpectrum() :
    fifo (2048 * 10) {
        buffer.setSize(1,fifo.getTotalSize());
        buffer.clear();
        spectrum = std::make_unique<Spectrum_GUI>();
    }
    ~FIFOSpectrum() {
             
    }
    
    
    void push(juce::AudioBuffer<T> block)
    {
            // write
            auto scopedWrite = fifo.write(block.getNumSamples());
            
            if (scopedWrite.blockSize1 > 0)
            {
                for(int chan = juce::jmin(block.getNumChannels(), buffer.getNumChannels()); --chan >=0;)
                buffer.copyFrom(0, scopedWrite.startIndex1,block,0,0,scopedWrite.blockSize1);
            }
            if (scopedWrite.blockSize2 > 0)
            {
                for(int chan = juce::jmin(block.getNumChannels(), buffer.getNumChannels()); --chan >=0;)
                {
                    buffer.copyFrom(chan,scopedWrite.startIndex2,block,chan,scopedWrite.blockSize1,scopedWrite.blockSize2);
                }
                
            }
        read();
            
    }
    
    void read()
    {
        //read audio
        auto scopedRead = fifo.read(fifo.getNumReady());
        
        if(scopedRead.blockSize1 > 0)
        {
            for (int i = scopedRead.startIndex1; i < scopedRead.startIndex1 + scopedRead.blockSize1; i++)
            {
                spectrum->write(buffer.getSample(0,i));
                
            }
            
        }
        if(scopedRead.blockSize2 > 0)
        {
            for (int i = scopedRead.startIndex2; i < scopedRead.startIndex2 + scopedRead.blockSize2; i++)
            {
                spectrum->write(buffer.getSample(0,i));
                
            }
            
        }
    }
    
    
    
    
    
    
    juce::AbstractFifo fifo;
    std::unique_ptr<Spectrum_GUI> spectrum;
private:
  
    juce::AudioBuffer<T> buffer;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(FIFOSpectrum)
};


#endif //FIFOSPECTRUM_H
