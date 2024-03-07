// See Designing Audio Effect Plugins in C++ -> Will C. Pirkle

#pragma once
#include "JuceHeader.h"
#include "math.h"
class So_Hpf_Butter
{
public:
    So_Hpf_Butter()  {}
    ~So_Hpf_Butter() {}
    
    void prepare(float sr) noexcept
    {
        m_sr = sr;
        a0 = 0;
        a1 = 0;
        a2 = 0;
        b1 = 0;
        b2 = 0;
    }
    
    void setCutoff(float fc)
    {
        m_c = tan((M_PI* fc) / m_sr);
    }
    
    void process(juce::AudioBuffer<float>& bufferToProcess, int bs)
    {
        const float* inputL = bufferToProcess.getReadPointer(0);
        const float* inputR = bufferToProcess.getReadPointer(1);
        float* outL = bufferToProcess.getWritePointer(0);
        float* outR = bufferToProcess.getWritePointer(1);
        if(m_c != temp_c)
        {
            // this works for block based processing
            c_inc = (m_c - temp_c) / bs;
            for (int i = 0; i < bs; ++i)
            {
                //y(n) = a0*x(n) + a1*x(n-1) + a2*x(n-2) - b*y(n-1) + b2*y(n-2)
                temp_c += c_inc;
                a0 = 1.0 / (1.0 + sqrt2*temp_c + pow(temp_c, 2.0));
                a1 = -2.0 * a0;
                a2 = a0;
                b1 = 2.0 * a0*(pow(temp_c, 2.0) - 1.0);
                //m_coeffs.a0 * (1.0 - sqrt2*c + pow(c, 2.0));
                b2 = a0 * (1.0 - sqrt2*temp_c + pow(temp_c, 2.0));
                const float inL = inputL[i];
                const float inR = inputR[i];
                b_0L = a_0L * a0 + a_1L * a1 + a_2L * a2 - b_1L * b1 - b_2L * b2;
                b_0R = a_0R * a0 + a_1R * a1 + a_2R * a2 - b_1R * b1 - b_2R * b2;
                
                b_2L = b_1L;
                b_1L = b_0L;
                a_2L = a_1L;
                a_1L = a_0L;
                a_0L = inL;
                
                b_2R = b_1R;
                b_1R = b_0R;
                a_2R = a_1R;
                a_1R = a_0R;
                a_0R = inR;
                
                outL[i] = b_0L;
                outR[i] = b_0R;
            }
            temp_c = m_c;
        } else {
            for (int i = 0; i < bs; ++i)
            {
               
                
                const float inL = inputL[i];
                const float inR = inputR[i];
                b_0L = a_0L * a0 + a_1L * a1 + a_2L * a2 - b_1L * b1 - b_2L * b2;
                b_0R = a_0R * a0 + a_1R * a1 + a_2R * a2 - b_1R * b1 - b_2R * b2;
                
                b_2L = b_1L;
                b_1L = b_0L;
                a_2L = a_1L;
                a_1L = a_0L;
                a_0L = inL;
                
                b_2R = b_1R;
                b_1R = b_0R;
                a_2R = a_1R;
                a_1R = a_0R;
                a_0R = inR;
                
                outL[i] = b_0L;
                outR[i] = b_0R;
            }
        }
             
    }
private:
    const float sqrt2 = (2.0f * 0.707106781186547524401f);
    float m_sr;
    float m_c;
    float temp_c;
    float c_inc = 0;
    float a0,a1,a2;
    float b1,b2;
    
    float a_0L,a_1L,a_2L;
    float b_0L,b_1L,b_2L;
    float a_0R,a_1R,a_2R;
    float b_0R,b_1R,b_2R;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (So_Hpf_Butter)
};
