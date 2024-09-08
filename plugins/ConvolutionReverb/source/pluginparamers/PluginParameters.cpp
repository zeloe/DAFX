#include "PluginParameters.h"


PluginParameter::PluginParameter()
{
    
}

PluginParameter::~PluginParameter()
{
    
}


juce::AudioProcessorValueTreeState::ParameterLayout PluginParameter::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;
    

    
    params.push_back (std::make_unique<juce::AudioParameterFloat> (GAIN,
                                                                   GAIN_NAME,
                                                                       -0.95f,
                                                                       0.95f,
                                                                       0.5));
    
   
    
    params.push_back (std::make_unique<juce::AudioParameterInt> (FREQUENCY,
                                                                   FREQUENCY_NAME,
                                                                    20,
                                                                    2000,
                                                                    10));
    
    for (const auto & param : params) {
            parameterList.add(param->getParameterID());
        }
    
    
    return { params.begin(), params.end() };
}


juce::StringArray PluginParameter::getPluginParameterList() {
    return parameterList;
}
