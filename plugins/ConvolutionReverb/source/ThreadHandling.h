/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/


#ifndef THREADHANDLING_H
#define THREADHANDLING_H
#pragma once
#include <JuceHeader.h>
#include "SIMDProcessor.h"

//==============================================================================
class FilterPartitionThread : public juce::Thread {
public:
    FilterPartitionThread() :
    juce::Thread("Partitions"),
    fifo(2048 * 20)
    {
        buffer.setSize(2,fifo.getTotalSize());
        buffer2.setSize(2,FFTSIZE);
        buffer.clear();
        buffer2.clear();
        conv = std::make_unique<SIMDProcessor>();
        
    }
    ~FilterPartitionThread() override
    {
        stopThread(2000);
        
        
    }
    
    void prepare(const juce::dsp::ProcessSpec& spec) {
        
        conv->prepare(spec);
        buffer.clear();
        buffer2.clear();
        startThread(Priority::high);
        
    }
    
    
    
    
    
    
    void push(juce::AudioBuffer<float> block)
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
    
    
    
    
    void run() override
    {
        while (!threadShouldExit())
        {
            // Check how many samples are ready in the FIFO
            auto numReady = fifo.getNumReady();
            
            // Only process if we have enough samples to form a full FFT block
            if (numReady >= FFTSIZE)
            {
                // Read the available samples from the FIFO in blocks of FFTSIZE
                auto scopedRead = fifo.read(FFTSIZE);
                
               
                
                // Process the first block (blockSize1)
                if (scopedRead.blockSize1 > 0)
                {
                    for (int chan = 0; chan < buffer.getNumChannels(); ++chan)
                    {
                        // Copy blockSize1 from the buffer to tempBuffer
                        buffer2.copyFrom(chan, 0, buffer, chan, scopedRead.startIndex1, scopedRead.blockSize1);
                    }
                    
                    // Create AudioBlock and ProcessContext to process the copied data
                    juce::dsp::AudioBlock<float> audioBlock(buffer2.getArrayOfWritePointers(), buffer2.getNumChannels(), FFTSIZE);
                    juce::dsp::ProcessContextReplacing<float> context(audioBlock);
                    conv->process(context); // Process with convolution or other DSP
                }
                
                // Process the second block (blockSize2) if it exists
                if (scopedRead.blockSize2 > 0)
                {
                    for (int chan = 0; chan < buffer.getNumChannels(); ++chan)
                    {
                        // Copy blockSize2 from the buffer to tempBuffer
                        buffer2.copyFrom(chan, 0, buffer, chan, scopedRead.startIndex2, scopedRead.blockSize2);
                    }
                    
                    // Create AudioBlock and ProcessContext for the second block of data
                    juce::dsp::AudioBlock<float> audioBlock(buffer2.getArrayOfWritePointers(), buffer2.getNumChannels(), FFTSIZE);
                    juce::dsp::ProcessContextReplacing<float> context(audioBlock);
                    conv->process(context); // Process with convolution or other DSP
                }
            }
        }
    }

        
        
        
        
        
    std::unique_ptr<SIMDProcessor> conv;
    
private:
    const size_t FFTSIZE = 2048;
    juce::AbstractFifo fifo;
    juce::AudioBuffer<float> buffer;
    juce::AudioBuffer<float> buffer2;
   
};
#endif // THREADHANDLING_H
