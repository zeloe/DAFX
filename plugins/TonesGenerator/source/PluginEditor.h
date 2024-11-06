/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "GUI.h"
//==============================================================================
/**
*/

class PluginAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    PluginAudioProcessorEditor (PluginAudioProcessor&);
    ~PluginAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;
   
private:
    PluginAudioProcessor& audioProcessor;
    std::unique_ptr<GUI> gui;
    
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PluginAudioProcessorEditor)
};
