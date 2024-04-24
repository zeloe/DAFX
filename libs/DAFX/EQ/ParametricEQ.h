/*
  ==============================================================================

    FirstOrder-Filters.h
    Created: 3 Apr 2023 5:39:05pm
    Author:  Onez

  ==============================================================================
*/
#include "JuceHeader.h"
#include "../Utility/Block_Smoothing.h"
#include "../Filters/FirstOrder-Filters.h"
#pragma once



template<typename FloatType>
class ParametricEQ
{
public:
    ParametricEQ()
    {
        lowPass = std::make_unique<FirstOrderAllPass<FloatType>>(1.0);
        lowPass2 = std::make_unique<FirstOrderAllPass<FloatType>>(1.0);
        smoother_highGain = std::make_unique<BlockSmoothing<FloatType>>();
        smoother_lowGain = std::make_unique<BlockSmoothing<FloatType>>();
        smoother_midGain = std::make_unique<BlockSmoothing<FloatType>>();
    }
    ~ParametricEQ() {};
    
    //Call this in prepare to play
    void prepare(float sampleRate, int blockSize) noexcept
    {
        lowPass->prepare(sampleRate, blockSize);
        lowPass2->prepare(sampleRate, blockSize);
    }
    void setLowCutoff(float fc)
    {
        lowPass2->setCutoff(fc);
       
    }
    void setHighCutoff(float fc)
    {
        lowPass->setCutoff(fc);
    }

    void setHighGain(float g)
    {
        FloatType temp = g;
        smoother_highGain->smooth(temp, highGain);
    }


    void setMidGain(float g)
    {
        FloatType temp = g;
        smoother_midGain->smooth(temp, midGain);

    }


    void setLowGain(float g)
    {
        FloatType temp = g;
        smoother_lowGain->smooth(temp, lowGain);

    }
    template <typename ProcessContext>
    void process(const ProcessContext& context) noexcept
    {
        const auto& inputBlock = context.getInputBlock();
        auto& outputBlock = context.getOutputBlock();
        const auto numChannels = outputBlock.getNumChannels();
        const auto numSamples = outputBlock.getNumSamples();
       
        for (size_t channel = 0; channel < numChannels; ++channel)
        {
            auto* inputSamples = inputBlock.getChannelPointer(channel);
            auto* outputSamples = outputBlock.getChannelPointer(channel);
            if (lowPass->smoother_Cutoff->isSmoothing)
            {
                for (int i = 0; i < numSamples; i++)
                {
                    FloatType input = inputSamples[i];
                    FloatType mid = lowPass->processBlockInter(input);
                    FloatType high = mid - input;
                    FloatType low = lowPass2->processBlockInter(mid);
                    FloatType out = high * highGain + (low - mid) * midGain + low * lowGain;
                    outputSamples[i] = out;
                }
                lowPass->smoother_Cutoff->resetSmoother();

            }
            else if(lowPass2->smoother_Cutoff->isSmoothing)
            {
                for (int i = 0; i < numSamples; i++)
                {
                    FloatType input = inputSamples[i];
                    FloatType mid = lowPass->processBlockInter(input);
                    FloatType high = mid - input;
                    FloatType low = lowPass2->processBlockInter(mid);
                    FloatType out = high * highGain + (low - mid) * midGain + low * lowGain;
                    outputSamples[i] = out;
                }
                lowPass2->smoother_Cutoff->resetSmoother();
            }
            else
            {
                for (int i = 0; i < numSamples; i++)
                {
                    FloatType input = inputSamples[i];
                    FloatType mid = lowPass->processBlockInter(input);
                    FloatType high = mid - input;
                    FloatType low = lowPass2->processBlockInter(mid);
                    FloatType out = high * highGain + (low - mid) * midGain + low * lowGain;
                    outputSamples[i] = out;
                }
            }
        }  
       
    }
    
    std::unique_ptr<BlockSmoothing<FloatType>> smoother_highGain;
    std::unique_ptr<BlockSmoothing<FloatType>> smoother_midGain;
    std::unique_ptr<BlockSmoothing<FloatType>> smoother_lowGain;
    
private:
    std::unique_ptr<FirstOrderAllPass<FloatType>> lowPass;
    std::unique_ptr<FirstOrderAllPass<FloatType>> lowPass2;
    FloatType midGain = 0;
    FloatType lowGain = 0;
    FloatType highGain = 0;
 
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ParametricEQ);
};
