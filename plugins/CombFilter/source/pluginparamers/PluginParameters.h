


#ifndef PLUGINPARAMETER_H
#define PLUGINPARAMETER_H

#include <JuceHeader.h>

class PluginParameter
{
public:
    PluginParameter();
    ~PluginParameter();
    
    enum ParameterIndexFloat {
            GAIN,
            FREQUENCY,
            TOTAL_NUM_PARAMETERS
        
    };
    
    struct ParameterInfoFloat
        {
            juce::String name;
            juce::String label;
            float minValueFloat;
            float maxValueFloat;
            float defaultValueFloat;
            
        };
    
    inline static std::vector<ParameterInfoFloat> parameterListFloat = {
                { "param_Gain", "Gain", -.99f, .99f, 0.5f},
                { "param_Frequency", "Frequency", 20, 2000, 100}
        };
        
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
};

#endif 
