/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "../../../Filters/SecondOrder-Filters.h"
#include "../../../Utility/gain_block.h"
#include "../../../LFO/WaveShapes/SineWave.h"
//==============================================================================
/**
*/
class WahWahFilterAudioProcessor  : public juce::AudioProcessor
                            #if JucePlugin_Enable_ARA
                             , public juce::AudioProcessorARAExtension
                            #endif
{
public:
    //==============================================================================
    WahWahFilterAudioProcessor();
    ~WahWahFilterAudioProcessor() override;

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
    std::unique_ptr<Gain_Block> gainBlockDryL;
    std::unique_ptr<Gain_Block> gainBlockWetL;
    std::unique_ptr<Gain_Block> gainBlockDryR;
    std::unique_ptr<Gain_Block> gainBlockWetR;
    std::unique_ptr<BandpassFilter> bandpassL;
    std::unique_ptr<BandpassFilter> bandpassR;
    std::unique_ptr<SineWave> waveTable;
    std::atomic<float>* mix = nullptr;
    std::atomic<float>* lfo_freq = nullptr;
    std::atomic<float>* bw = nullptr;
    std::atomic<float>* cf = nullptr;
    std::atomic<float>* depth = nullptr;
    juce::AudioBuffer<float> LeftDry;
    juce::AudioBuffer<float> LeftWet;
    juce::AudioBuffer<float> RightDry;
    juce::AudioBuffer<float> RightWet;
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (WahWahFilterAudioProcessor)
};
