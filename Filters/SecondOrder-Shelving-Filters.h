/*
  ==============================================================================

    SecondOrder-Shelving-Filters.h
    Created: 5 Apr 2023 11:13:35am
    Author:  Onez

  ==============================================================================
*/

#pragma once
#include "JuceHeader.h"

class SecondOrderBandPassShelfFilter {
  
public:
    SecondOrderBandPassShelfFilter() {};
    ~SecondOrderBandPassShelfFilter() {};
    // Call this in perpare to play
    void prepare(float sampleRate, int blockSize) noexcept
    {
        fs = sampleRate;
        bs = blockSize;
        xh[0] = 0;
        xh[1] = 0;
        current_Wc = 0;
        current_V0 = 0;
        current_Wb = 0;
    }
    
    void setParams(float fc,float bw, float g)
    {
        // Wc is normalized cut-off frequency 0<Wc<1
        Wc = (2* fc) / fs;
        // Wb is normalized bandwodth 0<Wb<1
        Wb = (2* bw) / fs;
        // Normalized Gain
        V0 = pow(10,g/20);
        // G is gain in dB
        G = g;
        
    }
    
    
    void process(float* input)
    {
        
        if(current_Wc != Wc || current_V0 != V0 || current_Wb != Wb){
            inc_Wc = (Wc - current_Wc) / bs;
            inc_V0 = (V0 - current_V0) / bs;
            inc_Wb = (Wb - current_Wb) / bs;
            for(int i = 0; i < bs; i++){
                current_Wc += inc_Wc;
                current_V0 += inc_V0;
                current_Wb += inc_Wb;
                if(G > 0){
                    c = (tan((M_PI* current_Wb)/2.0)-1.0) / (tan((M_PI * current_Wb)/2.0)+1.0);
                } else {
                    c = (tan((M_PI* current_Wb)/2.0)-current_V0) / (tan((M_PI * current_Wb)/2.0)+current_V0);
                }
                H0 = current_V0 -1.0;
                d = -cos(M_PI * current_Wc);
                xh_new = input[i] - d * (1.0 - c) * xh[0] + c * xh[1];
                ap_y = -c * xh_new + d * (1.0 - c) * xh[0] + xh[1];
                xh[1] = xh[0];
                xh[0] = xh_new;
                input[i] = 0.5 * H0 *(input[i] - ap_y) + input[i];
                
            }
            current_Wc = Wc;
            current_Wb = Wb;
            current_V0 = V0;
        } else {
            for(int i = 0; i < bs; i++){
                xh_new = input[i] - d * (1.0 - c) * xh[0] + c * xh[1];
                ap_y = -c * xh_new + d * (1.0 - c) * xh[0] + xh[1];
                xh[1] = xh[0];
                xh[0] = xh_new;
                input[i] = 0.5 * H0 *(input[i] - ap_y) + input[i];
            }
        }
    }
    
    
    
private:
    float fs = 0;
    size_t bs = 0;
    float Wc = 0;
    float V0 = 0;
    float Wb = 0;
    float ap_y = 0;
    float G = 0;
    float current_Wc = 0;
    float inc_Wc = 0;
    float current_V0 = 0;
    float inc_V0 = 0;
    float current_Wb = 0;
    float inc_Wb = 0;
    float c = 0;
    float d = 0;
    float H0 = 0;
    float xh_new = 0;
    float xh[2] = {0};
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SecondOrderBandPassShelfFilter);
};
