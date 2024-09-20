/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
PluginAudioProcessorEditor::PluginAudioProcessorEditor (PluginAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    spectrum = p.fifo->spectrum.get();
    
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    addAndMakeVisible(spectrum);
    setSize (400, 400);
}

PluginAudioProcessorEditor::~PluginAudioProcessorEditor()
{
}

//==============================================================================
void PluginAudioProcessorEditor::paint (juce::Graphics& g)
{
    spectrum->paint(g);
}

void PluginAudioProcessorEditor::resized()
{
    spectrum->setBounds(getLocalBounds());
}
