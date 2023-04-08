/*
  ==============================================================================

    SecondOrder-FIlters.h
    Created: 3 Apr 2023 9:01:24pm
    Author:  Onez

  ==============================================================================
*/

#pragma once
#include "JuceHeader.h"
class BandpassFilter {
public:
    BandpassFilter() {};
    ~BandpassFilter() {};
    
    //Call this in prepare to play
    void prepare(float sampleRate, int blocksize) noexcept
    {
        Fs = sampleRate;
        bs = blocksize;
        xh[0] = 0;
        xh[1] = 0;
        current_Wc = 0;
        current_Wb = 0;
    }
    
    void setParams(float fc, float bw)
    {
        // Wc is normalized cut-off frequency 0<Wc<1
        Wc = (2*fc) / Fs;
        // Wb is normalized bandwidth 0<Wb<1
        Wb = (2*bw) / Fs;
    }
    
    void process(float* input)
    {
        if(current_Wc != Wc || current_Wb != Wb){
            inc_Wc = (Wc - current_Wc) / bs;
            inc_Wb = (Wb - current_Wb) / bs;
            for(int i = 0; i < bs; i++) {
                current_Wc += inc_Wc;
                current_Wb += inc_Wb;
                c = (tan((M_PI* current_Wb)/2.0)-1.0) / (tan((M_PI * current_Wb)/2.0)+1.0);
                d = -cos(M_PI * current_Wc);
                xh_new = input[i] - d * (1.0 - c ) * xh[0] + c * xh[1];
                ap_y = -c * xh_new + d* (1.0 - c ) * xh[0] + xh[1];
                xh[1] = xh[0];
                xh[0] = xh_new;
                input[i] = 0.5 * (input[i]  - ap_y);
            }
            current_Wc = Wc;
            current_Wb = Wb;
        } else {
            for(int i = 0; i < bs; i++) {
                xh_new = input[i] - d * (1.0 - c ) * xh[0] + c * xh[1];
                ap_y = -c * xh_new + d* (1.0 - c ) * xh[0] + xh[1];
                xh[1] = xh[0];
                xh[0] = xh_new;
                input[i] = 0.5 * (input[i]  - ap_y);
            }
        }
    }
    
    
private:
    float Fs = 0;
    float Wc = 0;
    float current_Wc = 0;
    float inc_Wc = 0;
    float Wb = 0;
    float current_Wb = 0;
    float inc_Wb = 0;
    float c = 0;
    float d = 0;
    float xh[2] {0};
    float xh_new = 0;
    float ap_y  = 0;
    size_t bs  =  0;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BandpassFilter);
};

class BandrecjectFilter {
public:
    BandrecjectFilter() {};
    ~BandrecjectFilter() {};
    
    //Call this in prepare to play
    void prepare(float sampleRate, int blocksize) noexcept
    {
        Fs = sampleRate;
        bs = blocksize;
        xh[0] = 0;
        xh[1] = 0;
        current_Wc = 0;
        current_Wb = 0;
    }
    
    void setParams(float fc, float bw)
    {
        // Wc is normalized cut-off frequency 0<Wc<1
        Wc = (2*fc) / Fs;
        // Wb is normalized bandwidth 0<Wb<1
        Wb = (2*bw) / Fs;
    }
    
    void process(float* input)
    {
        if(current_Wc != Wc || current_Wb != Wb){
            inc_Wc = (Wc - current_Wc) / bs;
            inc_Wb = (Wb - current_Wb) / bs;
            for(int i = 0; i < bs; i++) {
                current_Wc += inc_Wc;
                current_Wb += inc_Wb;
                c = (tan((M_PI* current_Wb)/2.0)-1.0) / (tan((M_PI * current_Wb)/2.0)+1.0);
                d = -cos(M_PI * current_Wc);
                xh_new = input[i] - d * (1.0 - c ) * xh[0] + c * xh[1];
                ap_y = -c * xh_new + d* (1.0 - c ) * xh[0] + xh[1];
                xh[1] = xh[0];
                xh[0] = xh_new;
                input[i] = 0.5 * (input[i]  + ap_y);
            }
            current_Wc = Wc;
            current_Wb = Wb;
        } else {
            for(int i = 0; i < bs; i++) {
                xh_new = input[i] - d * (1.0 - c ) * xh[0] + c * xh[1];
                ap_y = -c * xh_new + d* (1.0 - c ) * xh[0] + xh[1];
                xh[1] = xh[0];
                xh[0] = xh_new;
                input[i] = 0.5 * (input[i]  + ap_y);
            }
        }
    }
    
    
private:
    float Fs = 0;
    float Wc = 0;
    float current_Wc = 0;
    float inc_Wc = 0;
    float Wb = 0;
    float current_Wb = 0;
    float inc_Wb = 0;
    float c = 0;
    float d = 0;
    float xh[2] {0};
    float xh_new = 0;
    float ap_y  = 0;
    size_t bs  =  0;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BandrecjectFilter);
};
    
