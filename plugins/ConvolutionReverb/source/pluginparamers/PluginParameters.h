
#include <JuceHeader.h>


#ifndef PLUGINPARAMETER_H
#define PLUGINPARAMETER_H


class PluginParameter
{
public:
    PluginParameter();
    ~PluginParameter();
    
    inline static const juce::String
        GAIN = "param_gain",
        FREQUENCY = "param_frequency";
    
    inline static const juce::String
        GAIN_NAME = "Gain",
        FREQUENCY_NAME = "Frequency";
    
    
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    static juce::StringArray getPluginParameterList();
    inline static juce::StringArray parameterList;
};

#endif 
