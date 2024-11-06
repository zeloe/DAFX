/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#if JUCE_USE_SIMD
#ifndef SIMDPROCESSOR_H
#define SIMDPROCESSOR_H
#include <JuceHeader.h>
#include "../../../libs/DAFX/FFT/FFT_Convolution.h"
#include "../../../libs/DAFX/Utility/UtilFunctions.h"
#include "pluginparamers/PluginParameters.h"
//==============================================================================

//==============================================================================
template <typename T>
static T* toBasePointer(juce::dsp::SIMDRegister<T>* r) noexcept
{
    return reinterpret_cast<T*> (r);
}

constexpr auto registerSize = juce::dsp::SIMDRegister<float>::size();

class SIMDProcessor
{
public:
    SIMDProcessor()
    {
        convEngine = std::make_unique<FFT_Convolution<juce::dsp::SIMDRegister<float>>>();
        overlapBuffer.setSize(1, FFTSIZE);  // Buffer for storing overlapping output segments
     
        outputFifo = std::make_unique<juce::AbstractFifo>(FFTSIZE * 10);  // FIFO for output
        outputFifoBuffer.setSize(2,outputFifo->getTotalSize());   // Buffer for the output FIFO
        outputFifoBuffer.clear();
    }
    ~SIMDProcessor() {}
    void prepare(const juce::dsp::ProcessSpec& spec)
    {
        interleaved =juce::dsp::AudioBlock<juce::dsp::SIMDRegister<float>>(interleavedBlockData, 1, spec.maximumBlockSize);
        zero = juce::dsp::AudioBlock<float>(zeroData, juce::dsp::SIMDRegister<float>::size(), spec.maximumBlockSize); // [6]

        zero.clear();
        sampleRate = spec.sampleRate;   // [4]
        samplesPerBlock = spec.maximumBlockSize;
        tempBuffer.setSize(1,spec.maximumBlockSize);
        tempBuffer.clear();
        std::unique_ptr<juce::MemoryInputStream> IRStream = std::make_unique<juce::MemoryInputStream>(BinaryData::IR_aif, BinaryData::IR_aifSize,true);
        juce::AudioFormatManager formatManager;
        formatManager.registerBasicFormats();
       
        // Create an AudioFormatReader from the memory stream
        auto reader = formatManager.createReaderFor(std::move(IRStream));
        
        
        
        juce::AudioBuffer<float> audioBuffer(static_cast<int>(reader->numChannels), static_cast<int>(reader->lengthInSamples));
        reader->read(&audioBuffer, 0, static_cast<int>(reader->lengthInSamples), 0, true, true);
         
        convEngine->prepare(audioBuffer, spec.maximumBlockSize);
        
        
        delete(reader);
        outputFifoBuffer.clear();
    }

    template <typename SampleType>
    auto prepareChannelPointers(const juce::dsp::AudioBlock<SampleType>& block)
    {
        std::array<SampleType*, registerSize> result{};

        for (size_t ch = 0; ch < result.size(); ++ch)
            result[ch] = (ch < block.getNumChannels() ? block.getChannelPointer(ch) : zero.getChannelPointer(ch));

        return result;
    }

    void process(const juce::dsp::ProcessContextReplacing<float>& context)
    {
        jassert(context.getInputBlock().getNumSamples() == context.getOutputBlock().getNumSamples());
        jassert(context.getInputBlock().getNumChannels() == context.getOutputBlock().getNumChannels());

        const auto& input = context.getInputBlock();
        const auto numSamples = (int)input.getNumSamples();

        auto inChannels = prepareChannelPointers(input);

        using Format = juce::AudioData::Format<juce::AudioData::Float32, juce::AudioData::NativeEndian>;

        juce::AudioData::interleaveSamples(juce::AudioData::NonInterleavedSource<Format> { inChannels.data(), registerSize, },
            juce::AudioData::InterleavedDest<Format>      { toBasePointer(interleaved.getChannelPointer(0)), registerSize },
            numSamples);

        convEngine->process(juce::dsp::ProcessContextReplacing<juce::dsp::SIMDRegister<float>>(interleaved));

        
        
        
        
        
        
        auto outChannels = prepareChannelPointers(context.getOutputBlock());

        juce::AudioData::deinterleaveSamples(juce::AudioData::InterleavedSource<Format>  { toBasePointer(interleaved.getChannelPointer(0)), registerSize },
            juce::AudioData::NonInterleavedDest<Format> { outChannels.data(), registerSize },
            numSamples);
    
        
        
        // Write the final OLA result to output FIFO
        auto writeScoped = outputFifo->write(FFTSIZE);  // FFTSIZE samples being written

        if (writeScoped.blockSize1 > 0)
        {
            // Manually copy each sample from outChannels to outputFifoBuffer
            for (int chan = 0; chan < 1; ++chan) // Assuming 1 channel for simplicity, adjust as needed
            {
                for (int i = 0; i < writeScoped.blockSize1; ++i)
                {
                    outputFifoBuffer.setSample(chan, writeScoped.startIndex1 + i, outChannels[chan][i]); // Copy sample by sample
                }
            }
        }

        if (writeScoped.blockSize2 > 0)
        {
            for (int chan = 0; chan < 1; ++chan) // Adjust channel count as needed
            {
                for (int i = 0; i < writeScoped.blockSize2; ++i)
                {
                    outputFifoBuffer.setSample(chan, writeScoped.startIndex2 + i, outChannels[chan][writeScoped.blockSize1 + i]); // Copy remaining samples if buffer wraps
                }
            }
        }
        
        
    }

   
     

    //==============================================================================
    juce::AudioBuffer<float> overlapBuffer;      // For storing overlapping samples
    std::unique_ptr<juce::AbstractFifo> outputFifo;  // FIFO for output storage
    juce::AudioBuffer<float> outputFifoBuffer;   // Buffer for the output FIFO
    std::unique_ptr<FFT_Convolution<juce::dsp::SIMDRegister<float>>> convEngine;
    juce::dsp::AudioBlock<juce::dsp::SIMDRegister<float>> interleaved;              // [2]
    juce::dsp::AudioBlock<float> zero;

    juce::HeapBlock<char> interleavedBlockData, zeroData;               // [3]

    juce::AudioBuffer<float> tempBuffer;
    double sampleRate = 0.0;
    size_t samplesPerBlock = 0;
    const size_t FFTSIZE = 2048;
};
#endif //JUCE_SIMD
#endif //SIMDPROCESSOR_H
