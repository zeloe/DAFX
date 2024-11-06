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
    gui = std::make_unique<GUI>(p);
    setSize (400, 400);
    setResizable(false, true);
    addAndMakeVisible(gui.get());
    setResizeLimits(guiwidth,guiheight,guiwidth * 1.2,guiheight * 1.2);
}

PluginAudioProcessorEditor::~PluginAudioProcessorEditor()
{
}

//==============================================================================
void PluginAudioProcessorEditor::paint (juce::Graphics& g)
{
   
}

void PluginAudioProcessorEditor::resized()
{
  
    auto area = getLocalBounds();
        if (area.isEmpty ())
        {
            return;
        }
    else
    {
       
       
     
        float scaleX = (float)area.getWidth() / (float)guiwidth;
        float scaleY = (float)area.getHeight() / (float)guiheight;
        
       
        gui->setBounds(area);
        gui->setTransform(juce::AffineTransform::scale(scaleX,scaleY));
        
    }
}
 
