/*
  ==============================================================================

    UniversalCombFilter.h
    Created: 10 Apr 2023 11:03:36am
    Author:  Onez

  ==============================================================================
*/

#pragma once
#include "../DelayLine/DelayLine.h"
#include "../Utility/UtilFunctions.h"
template<typename FloatType>
class UniversalComb {
public:
    UniversalComb()
    {
        fbdelayLine  = std::make_unique<DelayLine<FloatType>>();
        ffdelayLine = std::make_unique<DelayLine<FloatType>>();
        smoother = std::make_unique<BlockSmoothing<float>>();
        
        
    }
    ~UniversalComb(){}

    void prepare(int delayLineSize, int maxBlockSize, double cur_sampleRate, int channels) noexcept
    {
        ffdelayLine->prepare(delayLineSize, maxBlockSize, channels);
        fbdelayLine->prepare(delayLineSize,maxBlockSize, channels);
      
        smoother->prepare(maxBlockSize);
        this->sampleRate = cur_sampleRate;
        tempFB = 0;
        tempFF = 0;
         
    }
    void setFrequency(float hz)
    {
        const float period  = 1.f / hz;
        float delayTimeSamples =  (period * sampleRate);
        ffdelayLine->setParams(delayTimeSamples);
        fbdelayLine->setParams(delayTimeSamples);
         
    }
    void setLinGain(float g)
    {
        a = g;
        smoother->smooth(a, current_a);
    }
   
    template <typename ProcessContext>
    void process(const ProcessContext& context) noexcept
    {
     
        const auto& inputBlock = context.getInputBlock();
        auto& outputBlock = context.getOutputBlock();
        const auto numChannels = outputBlock.getNumChannels();
        const auto numSamples = outputBlock.getNumSamples();

         
        FloatType input;

        if (ffdelayLine->smoother_DelayTime->isSmoothing == true)
        {
            for (size_t channel = 0; channel < numChannels; ++channel)
            {
                auto* inputSamples = inputBlock.getChannelPointer(channel);
                auto* outputSamples = outputBlock.getChannelPointer(channel);
                for (int i = 0; i < numSamples; i++)
                {
                    input = inputSamples[i];
                    tempFF = ffdelayLine->processBlockInter(input * current_a);
                    tempFB = (input)+tempFF + fbdelayLine->processBlockInter(tempFB * current_a);
                    outputSamples[i] = scale * (tempFB);
                   

                }
            }
            ffdelayLine->resetSmoother();
            fbdelayLine->resetSmoother();
        }
        else
        {
            for (size_t channel = 0; channel < numChannels; ++channel)
            {
                auto* inputSamples = inputBlock.getChannelPointer(channel);
                auto* outputSamples = outputBlock.getChannelPointer(channel);
                for (int i = 0; i < numSamples; i++)
                {
                    input = inputSamples[i];
                    tempFF = ffdelayLine->processBlockInter(input * current_a);
                    tempFB = (input)+tempFF + fbdelayLine->processBlockInter(tempFB * current_a);
                    outputSamples[i] = scale * (tempFB);
                }
            }
        }
    }
    
    
private:
    std::unique_ptr<DelayLine<FloatType>> ffdelayLine;
    std::unique_ptr<DelayLine<FloatType>> fbdelayLine;
    std::unique_ptr<BlockSmoothing<float>> smoother;
   
    double sampleRate = 0;
    float a = 0;
    float current_a = 0;
    int current_bs = 0;
    FloatType tempFB = 0;
    FloatType tempFF = 0;

    const FloatType scale = 0.1f;
};

template<typename T>
class C_UniversalComb {
public:
    C_UniversalComb() {
        fbdelayLine  = std::make_unique<C_DelayLine<T>>();
        ffdelayLine = std::make_unique<C_DelayLine<T>>();
    }
    
    ~C_UniversalComb(){}
    
    void prepare(int maxBlockSize, double cur_sampleRate) {
        //20.f is min HZ and then converted to samples
        const int maxSize = std::ceil((1.f / 20.f) * cur_sampleRate) * 2;
        // gives a bigger delaylinesize based on blocksize
        const int maxDelayLineSize = nextPowerOfBlockSize(maxBlockSize, maxSize);
        //prepare delaylines
        ffdelayLine->prepare(maxDelayLineSize, maxBlockSize);
        fbdelayLine->prepare(maxDelayLineSize, maxBlockSize);
        const int initDelay = std::ceil((1.f / 20.f) * cur_sampleRate);
        ffdelayLine->getDelay(initDelay);
        fbdelayLine->getDelay(initDelay);
        output = 0.f;
        return;
    }
    
    template <typename ProcessContext>
    void process(const ProcessContext& context) noexcept {
        
        const auto& inputBlock = context.getInputBlock();
        auto& outputBlock = context.getOutputBlock();
        const auto numChannels = outputBlock.getNumChannels();
        const auto numSamples = outputBlock.getNumSamples();
        
        ffdelayLine->setWriteBlock();
        fbdelayLine->setWriteBlock();
        for(size_t ch = 0; ch < numChannels; ch++) {
            auto* inputSamples = inputBlock.getChannelPointer(ch);
            auto* outputSamples = outputBlock.getChannelPointer(ch);
            for(size_t i = 0; i < numSamples; i++) {
                T input = inputSamples[i];
                ffdelayLine->write(input * 0.9f);
                output = input + ffdelayLine->read() + fbdelayLine->read();
                fbdelayLine->write(output * 0.9f);
                outputSamples[i] = output * scale;
            }
        }
        ffdelayLine->incrementWriteBlock();
        fbdelayLine->incrementWriteBlock();
        return;
    }
    
    
    
    
    
    
    
    
    
    
    
    
    
    
private:
    std::unique_ptr<C_DelayLine<T>> ffdelayLine;
    std::unique_ptr<C_DelayLine<T>> fbdelayLine;
    T output = 0.f;
    const T scale = .03f;
};
 
