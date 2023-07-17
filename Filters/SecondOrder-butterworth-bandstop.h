/*
  ==============================================================================

    SecondOrder-butterworth-bandstop.h
    Created: 17 Jul 2023 3:07:30pm
    Author:  Onez

  ==============================================================================
*/
// See Designing Audio Effect Plugins in C++ -> Will C. Pirkle
#pragma once

#include "JuceHeader.h"

class SecondOrder_Butterworth_bandstop
{
public:
    SecondOrder_Butterworth_bandstop(){}
    
    ~SecondOrder_Butterworth_bandstop(){}
    
    void prepare(float sampleRate, int blocksize)
    {
        m_sampleRate = sampleRate;
        bs = blocksize;
        a0 = 0;
        a1 = 0;
        a2 = 0;
        b1 = 0;
        b2 = 0;
        m_a0 = 0;
        m_a1 = 0;
        m_a2 = 0;
        m_b1 = 0;
        m_b2 = 0;
    }
    
    void setParams(float fc, float bw)
    {
        K = tan((M_PI * fc) / m_sampleRate);
        Q = (1.0 / bw);
    }
    
    void process(float* input)
    {
        if(K != current_K || Q != current_q)
        {
            inc_K = (K - current_K) / bs;
            inc_q = (Q - current_q) / bs;
            for(int i = 0; i < bs; i++)
            {
                
                current_K += inc_K;
                current_q += inc_q;
                
                om = current_K * current_K * current_q + current_K + current_q;
                m_a0 = (current_q *(current_K * current_K + 1.0)) / om;
                m_a1 = (current_q * 2.0* (current_K * current_K - 1.0)) / om;
                m_a2 = m_a0;
                m_b1 = (2.0* current_q *(current_K * current_K - 1.0)) / om;
                m_b2 = (current_K * current_K * current_q - current_K + current_q) / om;
                b0 = a0 * m_a0 + a1 * m_a1 + a2 * m_a2 - b1 * m_b1 - b2 * m_b2;
                
                b2 = b1;
                b1 = b0;
                a2 = a1;
                a1 = a0;
                a0 = input[i];
                input[i] = b0;
            }
            current_K = K;
            current_q = Q;
        }
        else
        {
            for(int i = 0; i < bs; i++)
            {
                b0 = a0 * m_a0 + a1 * m_a1 + a2 * m_a2 - b1 * m_b1 - b2 * m_b2;
                b2 = b1;
                b1 = b0;
                a2 = a1;
                a1 = a0;
                a0 = input[i];
                input[i] = b0;
            }
        }
    }
    
private:
    float bw_;
    float current_K;
    float inc_q;
    float fc_;
    float current_fc;
    float inc_K;
    float m_sampleRate;
    int bs;
    float a0, a1, a2;
    float b0,b1, b2;
    float m_a0, m_a1, m_a2;
    float m_b1, m_b2;
    float K , om;
    float current_q;
    float Q;
};
