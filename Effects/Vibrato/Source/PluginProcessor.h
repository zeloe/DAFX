/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "../../../DelayLine/DelayLine.h"
#include "../../../LFO/WaveShapes/SineWave.h"
#include "../../../Utility/gain_block.h"
//==============================================================================
/**
*/
class VibratoAudioProcessor  : public juce::AudioProcessor
                            #if JucePlugin_Enable_ARA
                             , public juce::AudioProcessorARAExtension
                            #endif
{
public:
    //==============================================================================
    VibratoAudioProcessor();
    ~VibratoAudioProcessor() override;

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

    std::unique_ptr<SineWave> sine_LFO;
    std::unique_ptr<DelayLine> delayL;
    std::unique_ptr<DelayLine> delayR;
    
    std::unique_ptr<Gain_Block> wetL;
    std::unique_ptr<Gain_Block> wetR;
    std::unique_ptr<Gain_Block> dryL;
    std::unique_ptr<Gain_Block> dryR;
    
    juce::AudioBuffer<float> leftDry;
    juce::AudioBuffer<float> rightDry;
    
    juce::AudioBuffer<float> leftWet;
    juce::AudioBuffer<float> rightWet;
    
    std::atomic<float>* mix = nullptr;
    std::atomic<float>* lfo_freq = nullptr;
    std::atomic<float>* width = nullptr;
    
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (VibratoAudioProcessor)
};
