/*
  ==============================================================================

    Shelving-Filters.h
    Created: 4 Apr 2023 11:39:50am
    Author:  Onez

  ==============================================================================
*/

#pragma once
#include "JuceHeader.h"
class FirstOrderLowshelvingFilter {
public:
    FirstOrderLowshelvingFilter() {};
    ~FirstOrderLowshelvingFilter() {};
    
    // Call this in perpare to play
    void prepare(float sampleRate, int blocksize) noexcept
    {
        Fs = sampleRate;
        bs = blocksize;
        current_Wc = 0;
        current_V0 = 0;
    }
    
    void setParams(float fc, float g)
    {
        // Wc is normalized cut-off frequency 0<Wc<1
        Wc = (2*fc) / Fs;
        // Normalized Gain
        V0 = pow(10,g/20);
        // G is the Gain in dB
        G = g;
    }
    
    void process(float* input)
    {
        if(current_Wc != Wc || current_V0 != V0){
            inc_Wc = (Wc - current_Wc) / bs;
            inc_V0 = (V0 - current_V0) / bs;
            for(int i = 0; i < bs; i++) {
                current_Wc += inc_Wc;
                current_V0 += inc_V0;
                if(G > 0){
                    c = (tan((M_PI* current_Wc)/2.0)-1.0) / (tan((M_PI * current_Wc)/2.0)+1.0);
                } else {
                    c = (tan((M_PI* current_Wc)/2.0)-current_V0) / (tan((M_PI * current_Wc)/2.0)+current_V0);
                }
                xh_new = input[i] - c * xh;
                H0 =  current_V0 - 1.0;
                ap_y = c * xh_new + xh;
                xh = xh_new;
                input[i] = 0.5 * H0 * (input[i]  + ap_y) + input[i];
            }
            current_Wc = Wc;
            current_V0 = V0;
        } else {
            for(int i = 0; i < bs; i++) {
                xh_new = input[i] - c * xh;
                H0 = V0 - 1.0;
                ap_y = c * xh_new + xh;
                xh = xh_new;
                input[i] = 0.5 * H0 * (input[i]  + ap_y) + input[i];
            }
        }
    }
    
    
private:
    float Fs = 0;
    float Wc = 0;
    float current_Wc = 0;
    float inc_Wc = 0;
    float G = 0;
    float current_V0 = 0;
    float inc_V0 = 0;
    float c = 0;
    float V0 = 0;
    float H0 = 0;
    float xh = 0;
    float xh_new = 0;
    float ap_y  = 0;
    size_t bs  =  0;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FirstOrderLowshelvingFilter);
};

class FirstOrderHighshelvingFilter {
public:
    FirstOrderHighshelvingFilter() {};
    ~FirstOrderHighshelvingFilter() {};
    
    // Call this in perpare to play
    void prepare(float sampleRate, int blocksize) noexcept
    {
        Fs = sampleRate;
        bs = blocksize;
        current_Wc = 0;
        current_V0 = 0;
    }
    
    void setParams(float fc, float g)
    {
        // Wc is normalized cut-off frequency 0<Wc<1
        Wc = (2*fc) / Fs;
        // Normalized Gain
        V0 = pow(10,g/20);
        // G is the Gain in dB
        G  = g;
    }
    
    void process(float* input)
    {
        if(current_Wc != Wc || current_V0 != V0){
            inc_Wc = (Wc - current_Wc) / bs;
            inc_V0 = (V0 - current_V0) / bs;
            for(int i = 0; i < bs; i++) {
                current_Wc += inc_Wc;
                current_V0 += inc_V0;
                if(G > 0){
                    c = (tan((M_PI* current_Wc)/2.0)-1.0) / (tan((M_PI * current_Wc)/2.0)+1.0);
                } else {
                    c = (tan((M_PI* current_Wc)/2.0)-current_V0) / (tan((M_PI * current_Wc)/2.0)+current_V0);
                }
                xh_new = input[i] - c * xh;
                H0 =  current_V0 - 1.0;
                ap_y = c * xh_new + xh;
                xh = xh_new;
                input[i] = 0.5 * H0 * (input[i]  - ap_y) + input[i];
            }
            current_Wc = Wc;
            current_V0 = V0;
        } else {
            for(int i = 0; i < bs; i++) {
                xh_new = input[i] - c * xh;
                H0 = V0 - 1.0;
                ap_y = c * xh_new + xh;
                xh = xh_new;
                input[i] = 0.5 * H0 * (input[i]  - ap_y) + input[i];
            }
        }
    }
    
    
private:
    float Fs = 0;
    float Wc = 0;
    float current_Wc = 0;
    float inc_Wc = 0;
    float current_V0 = 0;
    float inc_V0 = 0;
    float c = 0;
    float G = 0;
    float V0 = 0;
    float H0 = 0;
    float xh = 0;
    float xh_new = 0;
    float ap_y  = 0;
    size_t bs  =  0;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FirstOrderHighshelvingFilter);
};

