
#ifndef FIFO_H
#define FIFO_H



#pragma once
#include "JuceHeader.h"

template <typename T>
class FIFO {
    
public:
    FIFO() :
    fifo (96000) {
        // set number of channels
        buffer.setSize(2,fifo.getTotalSize());
        
    }
    ~FIFO() {
             
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
            
    }
    
    float read()
    {
        //read audio
        auto scopedRead = fifo.read(fifo.getNumReady());
        
        if(scopedRead.blockSize1 > 0)
        {
            for (int i = scopedRead.startIndex1; i < scopedRead.startIndex1 + scopedRead.blockSize1; i++)
            {
                return (buffer.getSample(0,i));
                
            }
            
        }
        if(scopedRead.blockSize2 > 0)
        {
            for (int i = scopedRead.startIndex2; i < scopedRead.startIndex2 + scopedRead.blockSize2; i++)
            {
                return(buffer.getSample(0,i));
                
            }
            
        }
    }
    juce::AbstractFifo fifo;
private:
       
        juce::AudioBuffer<T> buffer;
    
};


#endif //FIFO_H
