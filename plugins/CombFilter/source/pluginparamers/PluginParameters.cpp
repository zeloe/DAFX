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
    

    
    for (const auto& param : parameterListFloat)
        {
            
                    params.push_back(std::make_unique<juce::AudioParameterFloat>(param.name,
                    param.label,
                    param.minValueFloat,
                    param.maxValueFloat,
                    param.defaultValueFloat));
        }
    
    
    return { params.begin(), params.end() };
}

