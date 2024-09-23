
#include <JuceHeader.h>


#ifndef PLUGINPARAMETER_H
#define PLUGINPARAMETER_H


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
    
    struct ParameterInfoFloat {
        juce::String name;
        juce::String label;
        float minValueFloat;
        float maxValueFloat;
        float defaultValueFloat;
        bool isAutomatable;
    };
    
    inline static std::vector<ParameterInfoFloat> parameterListFloat = {
                { "param_Gain", "Gain", 0.f, 1.f, 0.5f, true},
                { "param_Frequency", "Frequency", 20, 2000, 100, true}
        };
    
    struct ParameterInfoChoice {
        juce::String name;
        juce::String label;
        juce::StringArray labels;
        int defaultValueInt;
        bool isAutomatable;
    };
    inline static juce::StringArray waveShapesNames = {"Sine","Triangle","Sawtooh","Square","Noise","PinkNoise"};
    
    inline static std::vector<ParameterInfoChoice> parameterListChoice {
        {"p_Wavetables", "Waveshapes",waveShapesNames,0,false}
    };
    
    
    
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
};

#endif //PLUGINPARAMETER_H
