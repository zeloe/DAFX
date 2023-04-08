/*
  ==============================================================================

    FirstOrder-Filters.h
    Created: 3 Apr 2023 5:39:05pm
    Author:  Onez

  ==============================================================================
*/
#include "JuceHeader.h"
#pragma once
class FirstOrderLowpass
{
public:
    FirstOrderLowpass() {};
    ~FirstOrderLowpass() {};
    
    //Call this in prepare to play
    void prepare(float sampleRate, int blocksize) noexcept
    {
        Fs = sampleRate;
        bs = blocksize;
        current_Wc = 0;
    }
    void setParams(float fc)
    {
        // Wc is normalized cut-off frequency 0<Wc<1
        Wc = (2*fc) / Fs;
        
    }
    void process(float* input)
    {
        
        if(current_Wc != Wc){
            inc_Wc = (Wc - current_Wc) / bs;
            for(int i = 0; i < bs; i++) {
                current_Wc += inc_Wc;
                c = (tan((M_PI*current_Wc)/2.0)-1.0) / (tan((M_PI*current_Wc)/2.0)+1.0);
                xh_new = input[i] - c * xh;
                ap_y = c * xh_new + xh;
                xh = xh_new;
                input[i] =  0.5 * (input[i] + ap_y);
                
            }
            current_Wc = Wc;
        } else {
            for(int i = 0; i < bs; i++) {
                xh_new = input[i] - c * xh;
                ap_y = c * xh_new + xh;
                xh = xh_new;
                float temp =  0.5 * (input[i] + ap_y);
                input[i] = temp;
            }
            
        }
    }
    
    
    
private:
    float Fs = 0;
    float Wc = 0;
    float current_Wc = 0;
    float inc_Wc = 0;
    float c = 0;
    float xh = 0;
    float xh_new = 0;
    float ap_y  = 0;
    size_t bs  =  0;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FirstOrderLowpass);
};
class FirstOrderHighPass
{
public:
    FirstOrderHighPass() {};
    ~FirstOrderHighPass() {};
    //Call this in prepare to play
    void prepare(float sampleRate, int blocksize) noexcept
    {
       
        Fs = sampleRate;
        bs = blocksize;
        current_Wc = 0;
        
    }
    void setParams(float fc)
    {
        // Wc is normalized cut-off frequency 0<Wc<1
        Wc = (2*fc) / Fs;
        
    }
    void process(float* input)
    {
        if(current_Wc != Wc){
            inc_Wc = (Wc - current_Wc) / bs;
            for(int i = 0; i < bs; i++) {
                current_Wc += inc_Wc;
                c = (tan((M_PI*current_Wc)/2.0)-1.0) / (tan((M_PI*current_Wc)/2.0)+1.0);
                xh_new = input[i] - c * xh;
                ap_y = c * xh_new + xh;
                xh = xh_new;
                input[i] = 0.5 * (input[i] - ap_y);
            }
            current_Wc = Wc;
        } else {
            for(int i = 0; i < bs; i++) {
                xh_new = input[i] - c * xh;
                ap_y = c * xh_new + xh;
                xh = xh_new;
                input[i] = 0.5 * (input[i] - ap_y);
            }
            
        }
    }
    
    
    
private:
    float Fs = 0;
    float Wc = 0;
    float current_Wc = 0;
    float inc_Wc = 0;
    float c = 0;
    float xh = 0;
    float xh_new = 0;
    float ap_y  = 0;
    size_t bs  =  0;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FirstOrderHighPass);
};
