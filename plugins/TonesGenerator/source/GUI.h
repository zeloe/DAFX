/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#ifndef GUI_H
#define GUI_H
#pragma once
#include <JuceHeader.h>
#include "pluginparamers/PluginParameters.h"
#include "PluginProcessor.h"
#include "GUIDefines.h"
class GUI : public juce::Component
{
public:
    GUI(PluginAudioProcessor& pRef) :
    processorRef(pRef),
    tonesParameter(processorRef.treeState.getParameter(PluginParameter::parameterListInt[0].name))
    {
        int counter = 1;
      
        for(auto waveName : PluginParameter::waveShapesNames) {
            tonesBox.addItem(waveName,counter);
            counter++;
        }
        addAndMakeVisible(tonesBox);
        // Create a ComboBoxAttachment to link tonesBox with tonesParameter
        tonesAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
            processorRef.treeState,
            PluginParameter::parameterListInt[0].name,
            tonesBox
        );
        initializeSliders();
        initializeAttachments();
       
        // Set an initial selected index based on the parameter's value
        tonesBox.setSelectedItemIndex(static_cast<int>(tonesParameter->getValue() * 5));

        // Set the onChange lambda to capture changes
        tonesBox.onChange = [this]() {
            if (tonesParameter) {
                int selectedIndex = tonesBox.getSelectedItemIndex();
                //set change on audio processor
                processorRef.simdProcessor->switchWaveTable(selectedIndex);
                //tell the host something has changed
            
                // Convert index to normalized value (0.0 to 1.0 range)
                float normalizedValue = selectedIndex / static_cast<float>(tonesBox.getNumItems() - 1);
                
                // Begin a gesture to let the host know the parameter is being changed
               // tonesParameter->beginChangeGesture();

                // Update the parameter's value based on the selected index
                tonesParameter->setValueNotifyingHost(normalizedValue);
                // End the gesture
                //tonesParameter->endChangeGesture();
                processorRef.updateHostDisplay();
            }
        };
        
    }
    ~GUI() override {}
    
    void resized() override
    {
        if(!getLocalBounds().isEmpty())
        {
            for (int i = 0; i < PluginParameter::ParameterIndexFloat::TOTAL_NUM_PARAMETERS; i++) {
                auto& slider = sliderArray[i];
                auto& spos = sliderPos[i];
                slider->setBounds(spos.x, spos.y, spos.width, spos.height);
            }
            tonesBox.setBounds(tonesx,10,tonesWidth,tonesHeight);
        }
        
        
    }
    
    
    
    
    void paint(juce::Graphics& g) override
    {
        
    }
    
    void initializeSliders()
    {
        {
            for (size_t i = 0; i < PluginParameter::ParameterIndexFloat::TOTAL_NUM_PARAMETERS; ++i)
            {
                sliderArray[i] = std::make_unique<juce::Slider>();
                auto& slider = sliderArray[i];
                slider->setSliderStyle(juce::Slider::RotaryVerticalDrag);
                slider->setTextBoxStyle(juce::Slider::TextBoxBelow, true, 100,20);
                addAndMakeVisible(*slider);
            }
        }
        
    }
    void initializeAttachments()
    {
        for (size_t i = 0; i < PluginParameter::ParameterIndexFloat::TOTAL_NUM_PARAMETERS; ++i)
            {
                    attachArray[i] = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
                    processorRef.treeState, PluginParameter::parameterListFloat[i].name, *sliderArray[i]);
            }
        
    }
    
    
  
private:
    PluginAudioProcessor& processorRef;
    juce::ComboBox tonesBox;
    std::array< std::unique_ptr<juce::Slider>, PluginParameter::ParameterIndexFloat::TOTAL_NUM_PARAMETERS> sliderArray;
    std::array< std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>, PluginParameter::ParameterIndexFloat::TOTAL_NUM_PARAMETERS> attachArray;
    juce::RangedAudioParameter* tonesParameter;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> tonesAttachment;
};

#endif //GUI_H
