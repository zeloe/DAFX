/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "../../../libs/DAFX/CombFilters/UniversalCombFilter.h"
#include "pluginparamers/PluginParameters.h"

#if JUCE_USE_SIMD

//==============================================================================
template <typename T>
static T* toBasePointer(juce::dsp::SIMDRegister<T>* r) noexcept
{
    return reinterpret_cast<T*> (r);
}

constexpr auto registerSize = juce::dsp::SIMDRegister<float>::size();

class SIMDCOMB
{
public:
    SIMDCOMB() 
    {
        uniComb = std::make_unique<UniversalComb<juce::dsp::SIMDRegister<float>>>();

    };
    ~SIMDCOMB() {};
    void prepare(const juce::dsp::ProcessSpec& spec)
    {
        interleaved =juce::dsp::AudioBlock<juce::dsp::SIMDRegister<float>>(interleavedBlockData, 1, spec.maximumBlockSize);
        zero = juce::dsp::AudioBlock<float>(zeroData, juce::dsp::SIMDRegister<float>::size(), spec.maximumBlockSize); // [6]

        zero.clear();
        sampleRate = spec.sampleRate;   // [4]
        samplesPerBlock = spec.maximumBlockSize;
        uniComb->prepare(samplesPerBlock * 50, samplesPerBlock, sampleRate, 2);
         
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

        uniComb->process(juce::dsp::ProcessContextReplacing<juce::dsp::SIMDRegister<float>>(interleaved)); // [12]

        auto outChannels = prepareChannelPointers(context.getOutputBlock()); // [13]

        juce::AudioData::deinterleaveSamples(juce::AudioData::InterleavedSource<Format>  { toBasePointer(interleaved.getChannelPointer(0)), registerSize },
            juce::AudioData::NonInterleavedDest<Format> { outChannels.data(), registerSize },
            numSamples); // [14]
    }

   
     

    //==============================================================================
    
    std::unique_ptr<UniversalComb<juce::dsp::SIMDRegister<float>>> uniComb;
    juce::dsp::AudioBlock<juce::dsp::SIMDRegister<float>> interleaved;              // [2]
    juce::dsp::AudioBlock<float> zero;

    juce::HeapBlock<char> interleavedBlockData, zeroData;               // [3]

    
    double sampleRate = 0.0;
    size_t samplesPerBlock = 0;
};
#endif
//=============================================================================
/**
*/
class PluginAudioProcessor  : public juce::AudioProcessor, public juce::AudioProcessorValueTreeState::Listener
                            #if JucePlugin_Enable_ARA
                             , public juce::AudioProcessorARAExtension
                            #endif
  
{
public:
    //==============================================================================
    PluginAudioProcessor();
    ~PluginAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;
    
    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;
    juce::AudioProcessorValueTreeState treeState;
private:

    void parameterChanged(const juce::String& parameterID, float newValue) override;
    void initParams();
    // Declare std::unique_ptr member variable for simdComb
    std::unique_ptr<SIMDCOMB> simdComb;
    std::atomic<float>* freq = nullptr;
    std::atomic<float>* gain = nullptr;
    juce::CriticalSection audioCallbackLock;
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PluginAudioProcessor)
};
