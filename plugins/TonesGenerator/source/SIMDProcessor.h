/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#if JUCE_USE_SIMD
#ifndef SIMDPROCESSOR_H
#define SIMDPROCESSOR_H
#pragma once
#include <JuceHeader.h>
#include "../../../libs/DAFX/WaveShapes/WaveShapes.h"
#include "pluginparamers/PluginParameters.h"

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
    SIMDProcessor(juce::AudioProcessorValueTreeState& treeState) : parameters(treeState), stopListening(false)
    {
        //make  wavetableprocessor
        waveTableProcessor = std::make_unique<WaveTableProcessor<juce::dsp::SIMDRegister<float>>>();
        getParams();
        listeningThread = std::thread([this] {
                   while (!stopListening.load()) {
                       // Run the ListenToChanges() function on the message thread
                       juce::MessageManager::callAsync([this] {
                           ListenToChanges();
                      
                       });

                       // Sleep for a short time to avoid hammering the CPU
                       std::this_thread::sleep_for(std::chrono::milliseconds(10));
                   }
               });

    }
    
    // Delete copy constructor and copy assignment operator
       SIMDProcessor(const SIMDProcessor&) = delete;
       SIMDProcessor& operator=(const SIMDProcessor&) = delete;
    
    ~SIMDProcessor()
    {
        // Signal the listening thread to stop and join it
                stopListening.store(true);
                if (listeningThread.joinable())
                    listeningThread.join();
    }
    
    void getParams() {
        size_t counter = 0;
        for (const auto& param : PluginParameter::parameterListFloat) {
            
            params[counter] = parameters.getParameter(param.name);
            paramComp[counter] = -10000.f;
            counter++;
        }
        waveTableIDX = parameters.getParameter("p_Wavetables");
    }
    //not really sure if this is the best method
    void ListenToChanges() {
        
        if(std::abs(params[0]->getValue() - paramComp[0]) > epsilon) {
            float newValue = params[0]->getValue();
            float newParam = newValue * 0.0001f;
            gain.store(newParam);
            paramComp[0] = newValue;
        }
        if(std::abs(params[1]->getValue() - paramComp[1]) > epsilon) {
            float newValue = params[1]->getValue();
            float overSampleRate = 4096.f / sampleRate;
            float res = newValue * overSampleRate * 1980 + 20;
            frequency.store(res);
            waveTableProcessor->setFrequency(frequency);
            paramComp[1] = newValue;
        }
    }
        
    void switchWaveTable() {
        int idx = waveTableIDX->getValue() * 5;
        // if it changes aquire lock
        // allocate memory
        //add to release pool
    }
    
    
    
    void prepare(const juce::dsp::ProcessSpec& spec)
    {
        interleaved =juce::dsp::AudioBlock<juce::dsp::SIMDRegister<float>>(interleavedBlockData, 1, spec.maximumBlockSize);
        zero = juce::dsp::AudioBlock<float>(zeroData, juce::dsp::SIMDRegister<float>::size(), spec.maximumBlockSize); // [6]

        zero.clear();
        sampleRate = spec.sampleRate;   // [4]
        samplesPerBlock = spec.maximumBlockSize;
        pole = 0.f;
        new_Gain = -1.f;
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

        const auto& input = context.getInputBlock(); // [9]
        const auto numSamples = (int)input.getNumSamples();

        auto inChannels = prepareChannelPointers(input); // [10]

        using Format = juce::AudioData::Format<juce::AudioData::Float32, juce::AudioData::NativeEndian>;

        juce::AudioData::interleaveSamples(juce::AudioData::NonInterleavedSource<Format> { inChannels.data(), registerSize, },
            juce::AudioData::InterleavedDest<Format>      { toBasePointer(interleaved.getChannelPointer(0)), registerSize },
            numSamples); // [11]
        //check if wavetable has changed, if messagethread aquired spinlock ?? then lock
        
        //swap it if needed
        
        
        //fade out and in
        
        // get data
        const auto processContext = juce::dsp::ProcessContextReplacing<juce::dsp::SIMDRegister<float>>(interleaved);
        auto& outputBlock = processContext.getOutputBlock();
        const auto numChannels = outputBlock.getNumChannels();
        const auto numSamples2 = outputBlock.getNumSamples();
        
        //actual processing
        juce::dsp::SIMDRegister<float> fac = juce::dsp::SIMDRegister<float>::fromNative({gain.load(),gain.load(),gain.load(),gain.load()});
        for(size_t ch = 0; ch < numChannels; ch++) {
            auto* outputSamples = outputBlock.getChannelPointer(ch);
            for(size_t i = 0; i < numSamples2; i++) {
                new_Gain = fac + pole * juce::dsp::SIMDRegister<float>::fromNative({0.9999f, 0.9999f, 0.9999f, 0.9999f});
               
                outputSamples[i] = waveTableProcessor->process() * new_Gain;
                pole = new_Gain;
            }
        }
        

        auto outChannels = prepareChannelPointers(context.getOutputBlock());

        juce::AudioData::deinterleaveSamples(juce::AudioData::InterleavedSource<Format>  { toBasePointer(interleaved.getChannelPointer(0)), registerSize },
            juce::AudioData::NonInterleavedDest<Format> { outChannels.data(), registerSize },
            numSamples);
    }


    //==============================================================================
    
    std::unique_ptr<WaveTableProcessor<juce::dsp::SIMDRegister<float>>> waveTableProcessor;
    juce::dsp::AudioBlock<juce::dsp::SIMDRegister<float>> interleaved;
    juce::dsp::AudioBlock<float> zero;

    juce::HeapBlock<char> interleavedBlockData, zeroData;

    
    float sampleRate = 0.0;
    size_t samplesPerBlock = 0;
private:
    juce::AudioProcessorValueTreeState& parameters;
    std::atomic<float> gain;
    std::atomic<float> frequency;
    static_assert(std::atomic<float>::is_always_lock_free);
    static const int totalNumFloatParams = PluginParameter::ParameterIndexFloat::TOTAL_NUM_PARAMETERS;
    std::array<juce::RangedAudioParameter*,totalNumFloatParams> params;
    juce::RangedAudioParameter* waveTableIDX;
    std::array<float,totalNumFloatParams> paramComp;
    const float epsilon = 0.0001f;  // Define a small tolerance value
    std::atomic<bool> stopListening;  // Atomic flag to control the loop
    std::thread listeningThread;  // Thread to run the listen loop
    juce::dsp::SIMDRegister<float> pole;
    juce::dsp::SIMDRegister<float> new_Gain;
};
#endif // JUCE SIMD
#endif // SIMDPROCESSOR_H
