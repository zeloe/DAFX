/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "../../../Filters/SecondOrder-Filters.h"
#include "../../../Filters/SecondOrder-butterworth-bandstop.h"
#include "../../../Utility/gain_block.h"
#include "../../../LFO/WaveTableOscillator.h"
#include "../../../LFO/WaveShapes/Hanning.h"
#include "../../../LFO/WaveShapes/Triangle.h"
//==============================================================================
/**
*/
class PhasingAudioProcessor  : public juce::AudioProcessor
                            #if JucePlugin_Enable_ARA
                             , public juce::AudioProcessorARAExtension
                            #endif
{
public:
    //==============================================================================
    PhasingAudioProcessor();
    ~PhasingAudioProcessor() override;

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
    
    std::unique_ptr<Gain_Block> dryL;
    std::unique_ptr<Gain_Block> wetL;
    
    
    std::unique_ptr<SecondOrder_Butterworth_bandstop> filterL1;
    std::unique_ptr<SecondOrder_Butterworth_bandstop> filterL2;
    std::unique_ptr<SecondOrder_Butterworth_bandstop> filterL3;
    
   
   
    std::unique_ptr<Gain_Block> dryR;
    std::unique_ptr<Gain_Block> wetR;
    
    std::unique_ptr<SecondOrder_Butterworth_bandstop> filterR1;
    std::unique_ptr<SecondOrder_Butterworth_bandstop> filterR2;
    std::unique_ptr<SecondOrder_Butterworth_bandstop> filterR3;
    std::shared_ptr<Hanning> hannData;
    std::shared_ptr<WaveTableOscillator<Hanning>> possine_LFO;
    std::shared_ptr<Triangle> triData;
    std::shared_ptr<WaveTableOscillator<Triangle>> tri_LFO;
    
    
   
    juce::AudioBuffer<float> LeftWet;
    juce::AudioBuffer<float> LeftWet2;
    juce::AudioBuffer<float> LeftWet3;
    juce::AudioBuffer<float> RightWet;
    juce::AudioBuffer<float> RightWet2;
    juce::AudioBuffer<float> RightWet3;
   
    std::atomic<float>* lfo_freq = nullptr;
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PhasingAudioProcessor)
};
