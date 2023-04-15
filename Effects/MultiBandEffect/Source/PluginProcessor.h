/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "../../../CombFilters/IIRCombFilter.h"
#include "../../../LFO/WaveShapes/SineWave.h"
#include "../../../Filters/FirstOrder-Filters.h"
#include "../../../Utility/gain_block.h"
#include "../../../CombFilters/IIRCombFilter.h"
#include "../../../LFO/WaveShapes/SineWave.h"
//==============================================================================
/**
*/
class MultiBandEffectAudioProcessor  : public juce::AudioProcessor
                            #if JucePlugin_Enable_ARA
                             , public juce::AudioProcessorARAExtension
                            #endif
{
public:
    //==============================================================================
    MultiBandEffectAudioProcessor();
    ~MultiBandEffectAudioProcessor() override;

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
    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    std::unique_ptr<FirstOrderLowpass>  LPF1L;
    std::unique_ptr<FirstOrderLowpass>  LPF1_L;
    std::unique_ptr<FirstOrderLowpass>  LPF2L;
    std::unique_ptr<FirstOrderLowpass>  LPF2_L;
    std::unique_ptr<FirstOrderLowpass>  LPF1R;
    std::unique_ptr<FirstOrderLowpass>  LPF1_R;
    std::unique_ptr<FirstOrderLowpass>  LPF2R;
    std::unique_ptr<FirstOrderLowpass>  LPF2_R;
    std::unique_ptr<Gain_Block> Gain_High;
    std::unique_ptr<Gain_Block> Gain_Mid;
    std::unique_ptr<Gain_Block> Gain_Low;
    
    
    std::atomic<float>* gain_high = nullptr;
    std::atomic<float>* gain_mid = nullptr;
    std::atomic<float>* gain_low = nullptr;
    std::atomic<float>* cross_high = nullptr;
    std::atomic<float>* cross_low = nullptr;
    
    juce::AudioBuffer<float> highL;
    juce::AudioBuffer<float> highR;
    juce::AudioBuffer<float> midL;
    juce::AudioBuffer<float> midR;
    juce::AudioBuffer<float> lowL;
    juce::AudioBuffer<float> lowR;
    
   
    
    
    
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MultiBandEffectAudioProcessor)
};
