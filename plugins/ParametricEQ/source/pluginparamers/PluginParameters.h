
#include <JuceHeader.h>


#ifndef PLUGINPARAMETER_H
#define PLUGINPARAMETER_H


class PluginParameter
{
public:
    PluginParameter();
    ~PluginParameter();
    
    inline static const juce::String
        LOW_GAIN = "param_lowGain",
        LOW_CUTOFF_FREQUENCY = "param_lowCutoff",
        HIGH_GAIN = "param_highGain",
        MID_GAIN = "param_midGain",
        HIGH_CUTOFF_FREQUENCY = "param_highCutoff";
    
    inline static const juce::String
        LOW_GAIN_NAME = "Low Gain",
        LOW_CUTOFF_FREQUENCY_NAME = "Low Frequency Cutoff",
        HIGH_GAIN_NAME = "High Gain",
        MID_GAIN_NAME = "MID Gain",
        HIGH_CUTOFF_FREQUENCY_NAME = "High Frequency Cutoff";
    
    
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    static juce::StringArray getPluginParameterList();
    inline static juce::StringArray parameterList;
};

#endif 
