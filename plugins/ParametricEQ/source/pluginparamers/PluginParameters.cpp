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
    

    
    params.push_back (std::make_unique<juce::AudioParameterFloat> (LOW_GAIN,
                                                                    LOW_GAIN_NAME,
                                                                       0.f,
                                                                       2.f,
                                                                       0.5));
    
   
    
    params.push_back (std::make_unique<juce::AudioParameterInt> (LOW_CUTOFF_FREQUENCY,
                                                                 LOW_CUTOFF_FREQUENCY_NAME,
                                                                    20,
                                                                    5000,
                                                                    10));
    params.push_back(std::make_unique<juce::AudioParameterInt>(HIGH_CUTOFF_FREQUENCY,
        HIGH_CUTOFF_FREQUENCY_NAME,
        5000,
        16000,
        8000));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(HIGH_GAIN,
        HIGH_GAIN_NAME,
        0.f,
        2.f,
        0.5));



    params.push_back(std::make_unique<juce::AudioParameterFloat>(MID_GAIN,
        MID_GAIN_NAME,
        0.f,
        2.f,
        0.5));
    
    for (const auto & param : params) {
            parameterList.add(param->getParameterID());
        }
    
    
    return { params.begin(), params.end() };
}


juce::StringArray PluginParameter::getPluginParameterList() {
    return parameterList;
}
