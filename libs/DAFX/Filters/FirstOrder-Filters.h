/*
  ==============================================================================

    FirstOrder-Filters.h
    Created: 3 Apr 2023 5:39:05pm
    Author:  Onez

  ==============================================================================
*/
#include "JuceHeader.h"
#include "../Utility/Block_Smoothing.h"
#pragma once



template<typename FloatType>
class FirstOrderAllPass
{
public:
    FirstOrderAllPass(FloatType sign)
    {
        smoother_Cutoff = std::make_unique<BlockSmoothing<FloatType>>();
        // 1.0 = lowpass , -1.0 = highpass
        flip = sign;
    }
    ~FirstOrderAllPass() {};
    
    //Call this in prepare to play
    void prepare(float sampleRate, int blocksize) noexcept
    {
        Fs = sampleRate;
        smoother_Cutoff->prepare(blocksize);
    }
    void setCutoff(float fc)
    {
        // Wc is normalized cut-off frequency 0<Wc<1
        Wc = (2*fc) / Fs;
        c = (tan((juce::MathConstants<float>::pi  *  Wc) / 2.0) - 1.0) / (tan((juce::MathConstants<float>::pi *  Wc) / 2.0) + 1.0);
         
        smoother_Cutoff->calcCoeff(c,current_c);
       
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
            if (smoother_Cutoff->isSmoothing) 
            {
                for (int i = 0; i < numSamples; i++)
                {

                    xh_new = inputSamples[i] - smoother_Cutoff->smoothing() * xh;
                    ap_y = current_c * xh_new + xh;
                    xh = xh_new;
                    outputSamples[i] = scale * (inputSamples[i] + (ap_y* flip));
                }
            smoother_Cutoff->resetSmoother();
            }
            else
            {
                for (int i = 0; i < numSamples; i++)
                {
                xh_new = inputSamples[i] - current_c * xh;
                ap_y = current_c * xh_new + xh;
                xh = xh_new;
                outputSamples[i] = scale * (inputSamples[i] + (ap_y * flip));
                }
            }
        }  
       
    }
    
    const FloatType processBlockInter(FloatType input)
    {
        if (smoother_Cutoff->isSmoothing)
        {
            xh_new = input - smoother_Cutoff->smoothing() * xh;
            ap_y = current_c * xh_new + xh;
            xh = xh_new;
            FloatType out = scale * (input + (ap_y * flip));
            return out;
        }
        else
        {
            xh_new = input - current_c * xh;
            ap_y = current_c * xh_new + xh;
            xh = xh_new;
            FloatType out = scale * (input + (ap_y * flip));
            return out;
        }
    }




    std::unique_ptr<BlockSmoothing<FloatType>> smoother_Cutoff;
    
private:
   
    float Fs = 0;
    float Wc = 0;
    float current_Wc = 0;
    FloatType current_c = 0;
    FloatType c = 0;
    FloatType xh = 0;
    FloatType xh_new = 0;
    FloatType ap_y  = 0;
    const FloatType scale = 0.5;
    FloatType flip = 1.0;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FirstOrderAllPass);
};