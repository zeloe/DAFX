/*
  ==============================================================================

    DelayLine.h
    Created: 13 Apr 2023 11:08:39am
    Author:  Onez

  ==============================================================================
*/

#pragma once

#include "JuceHeader.h"
#include "../Utility/Interpolation.h"
#include "../Utility/Block_Smoothing.h"
#include "immintrin.h"
class DelayLine
{
public:
    DelayLine()
    {
        smoother_DelayTime = std::make_unique<block_Smoothing>();
        smoother_Fraction = std::make_unique<block_Smoothing>();
    }
    ~DelayLine(){}
    
    void prepare(int delayLineSize, int& maxBlockSize, int channels)
    {
        this->channels = channels;
        delBuffer.setSize(this->channels,delayLineSize);
        delBuffer.clear();
        size = delayLineSize;
        t_output[0] = 0;
        t_output[1] = 0;
        smoother_DelayTime->prepare(maxBlockSize);
        smoother_Fraction->prepare(maxBlockSize);
    }
    
    void setParams(float del)
    {
        float temp = floor(del);
        fract = del - temp;
        delay = temp;
        //smoother_DelayTime->calcCoeff(delay,current_delay);
        smoother_DelayTime->smooth(delay, current_delay);
        smoother_Fraction->smooth(fract, current_fract);
    }
    
    void incrementDelayLine()
    {
        writePointer = (writePointer + 1) % size;
    }
    
    void incrementDelayLineInter()
    {
        writePointer = (writePointer + 1) % size;
    }
    
    void resetDLine()
    {
        writePointer = 0;
        readPointer = 0;
        k = 0;
    }
    
    void resetSmoother()
    {
        smoother_DelayTime->resetSmoother();
        smoother_Fraction->resetSmoother();
    }
    
    void process(juce::AudioBuffer<float>& buffer, int bs)
    {
        const float* input = buffer.getReadPointer(0);
        float* output = buffer.getWritePointer(0);
        const float* delRead = delBuffer.getReadPointer(0);
        float* delWrite = delBuffer.getWritePointer(0);
        current_bs = bs;
        if (smoother_DelayTime->isSmoothing == true)
        {
            for(int i = 0; i < bs; i++)
            {
                 
                readPointer = (writePointer - smoother_DelayTime->smoothing());
                delWrite[writePointer] = input[i];
                if (readPointer - 3 < 0)
                {
                    readPointer += size;
                }
                const float y0 = delRead[(readPointer - 3) % size];
                const float y1 = delRead[(readPointer - 2) % size];
                const float y2 = delRead[(readPointer - 1) % size];
                const float y3 = delRead[(readPointer) % size];
                
                const float t_output = cubicInterpolation(y0, y1, y2, y3, fract);
                output[i] = t_output;
                this->incrementDelayLine();
               
            }
            current_delay = delay;
            current_fract = fract;
            smoother_DelayTime->resetSmoother();
            smoother_Fraction->resetSmoother();
        }
        else
        {
            for(int i = 0; i < bs; i++)
            {
                delWrite[writePointer] = input[i];
                readPointer = (writePointer - int(current_delay) + size) % size;
                const float y0 = delRead[readPointer];
                output[i] = y0;
                this->incrementDelayLine();
            }
        }
    }
    
    
    
    const float processBlock(const float& input, int bs)
    {
        
        const float* delRead = delBuffer.getReadPointer(0);
        float* delWrite = delBuffer.getWritePointer(0);
        current_bs = bs;
        if (smoother_DelayTime->isSmoothing == true)
        {
            
            readPointer = (writePointer - smoother_DelayTime->smoothing());
            delWrite[writePointer] = input;
            if (readPointer - 3 < 0)
            {
                readPointer += size;
            }
            const float y0 = delRead[(readPointer - 3) % size];
            const float y1 = delRead[(readPointer - 2) % size];
            const float y2 = delRead[(readPointer - 1) % size];
            const float y3 = delRead[(readPointer) % size];
                
            const float t_output = cubicInterpolation(y0, y1, y2, y3, smoother_Fraction->smoothing());
                
            this->incrementDelayLine();
            return t_output;
               
        }
      
        else
        {
            delWrite[writePointer] = input;
            readPointer = (writePointer - int(current_delay) + size) % size;
            const float y0 = delRead[readPointer];
            this->incrementDelayLine();
            return y0;
        }
    }
    
    
    float processBlockInter(float input)
    {
        
        const float* delRead = delBuffer.getReadPointer(0);
        float* delWrite = delBuffer.getWritePointer(0);

        readPointer = (writePointer - current_delay);
        delWrite[writePointer] = input;

        if (smoother_DelayTime->isSmoothing)
        {
            if (readPointer - 7 < 0)
            {
                readPointer += size;
            }

            const float y0 = delRead[(readPointer - 7) % size];
            const float y1 = delRead[(readPointer - 5) % size];
            const float y2 = delRead[(readPointer - 3) % size];
            const float y3 = delRead[(readPointer - 1) % size];

            const float output = cubicInterpolation(y0, y1, y2, y3, current_fract);
            this->incrementDelayLineInter();
            return output;
        }
        else
        {
            const float y0 = delRead[(writePointer - int(current_delay) + size) % size];
            this->incrementDelayLineInter();
            return y0;
        }
    }
    
    
    
    
    float* processBuffer(juce::AudioBuffer<float>& input, int bs)
    {
        
        const float* leftIn = input.getReadPointer(0);
        const float* rightIn = input.getReadPointer(1);
        
        
        const float* delReadL = delBuffer.getReadPointer(0);
        const float* delReadR = delBuffer.getReadPointer(1);
        float* delWriteL = delBuffer.getWritePointer(0);
        float* delWriteR = delBuffer.getWritePointer(1);
        current_bs = bs;
        if (smoother_DelayTime->isSmoothing == true)
        {
            
            readPointer = (writePointer - smoother_DelayTime->smoothing());
            delWriteL[writePointer] = *leftIn++;
            delWriteR[writePointer] = *rightIn++;
            if (readPointer - 3 < 0)
            {
                readPointer += size;
            }
            const float fract = smoother_Fraction->smoothing();
            const float y0L = delReadL[(readPointer - 3 + size) % size];
            const float y1L = delReadL[(readPointer - 2 + size) % size];
            const float y2L = delReadL[(readPointer - 1 + size) % size];
            const float y3L = delReadL[(readPointer + size) % size];
            
            const float y0R = delReadR[(readPointer - 3 + size) % size];
            const float y1R = delReadR[(readPointer - 2 + size) % size];
            const float y2R = delReadR[(readPointer - 1 + size) % size];
            const float y3R = delReadR[(readPointer + size) % size];
                
            const float t_outputL = cubicInterpolation(y0L, y1L, y2L, y3L,fract);
            const float t_outputR = cubicInterpolation(y0R, y1R, y2R, y3R, fract);
            
            t_output[0] = t_outputL;
            t_output[1] = t_outputR;
            
            this->incrementDelayLine();
            return t_output;
               
        }
        else
        {
            delWriteL[writePointer] = *leftIn++;
            delWriteR[writePointer] = *rightIn++;
            readPointer = (writePointer - int(current_delay) + size) % size;
            const float y0L = delReadL[readPointer];
            const float y0R = delReadR[readPointer];
           
            t_output[0] = y0L;
            t_output[1] = y0R;
            this->incrementDelayLine();
            return t_output;
        }
    }
    
    
    
    
    std::unique_ptr<block_Smoothing> smoother_DelayTime;
    std::unique_ptr<block_Smoothing> smoother_Fraction;
private:
    juce::AudioBuffer<float> delBuffer;
    int current_bs = 0;
    float fract = 0;
    float delay = 0;
    float current_fract = 0;
    float current_delay = 0;
    float t_output [2] {0};
    int readPointer = 0;
    int writePointer = 0;
    int k = 1;
    int channels = 0;
    int size = 0;
};

class SIMDDelayLine
{
public:
    SIMDDelayLine() {};
    ~SIMDDelayLine() {};


    void prepare(int delayLineSize, int& maxBlockSize, int channels)
    {
        delBuffer.setSize(channels, delayLineSize);
        delBuffer.clear();
        this->delayLineSize = delayLineSize;
        // Initialize read and write indices to zero
        readPos = 0;
        writePos = 0;
    }
    void setParams(float del)
    {
        int temp = static_cast<int>(floor(del)); // Convert float to int
        current_delay = temp; // Initialize current_delay with the integer value
    }
        __m128 read()
        {
            // Get read and write pointers from the delay buffer
            const float* delRead = delBuffer.getReadPointer(0);

            // Calculate read position using modulo arithmetic
            int readPointer = (writePos - current_delay + delayLineSize) % delayLineSize;

            // Gather delayed samples from the delay buffer
            __m128 y0 = _mm_set_ps(delRead[readPointer + 3], delRead[readPointer + 2], delRead[readPointer + 1], delRead[readPointer]);

           

            return y0;
        }



    void incrementDelaySIMD() {
        // Increment write index by 4 (assuming processSample processes 4 samples at once)
        writePos = (writePos + 4) % delayLineSize; // Wrap around if index exceeds buffer size
    }

    void write(__m128 samples)
    {
        float* delWrite = delBuffer.getWritePointer(0);
        // Extract individual elements from the __m128 variable using intrinsic functions
        float sampleArray[4];
        _mm_storeu_ps(sampleArray, samples);
        // Loop over the four samples
        for (int i = 0; i < 4; ++i) {
            // Calculate the index where the sample should be written, wrapping around if necessary
            int index = (writePos + i) % delayLineSize;
            // Write the sample to the delay buffer
            delWrite[index] = sampleArray[i];
        }
         // Increment the delay line
            this->incrementDelaySIMD();
    }


    __m128 SIMDcubicInterpolation(__m128 y0, __m128 y1, __m128 y2, __m128 y3, __m128 x) {
        const __m128 a0 = _mm_sub_ps(_mm_sub_ps(_mm_add_ps(y3, y1), y2), y0);
        const __m128 a1 = _mm_sub_ps(_mm_sub_ps(y0, y1), a0);
        const __m128 a2 = _mm_sub_ps(y2, y0);
        const __m128 a3 = y1;
        return _mm_add_ps(_mm_add_ps(_mm_mul_ps(_mm_mul_ps(_mm_mul_ps(a0, x), x), x), _mm_mul_ps(_mm_mul_ps(a1, x), x)), _mm_add_ps(_mm_mul_ps(a2, x), a3));
    }

private:
    juce::AudioBuffer<float> delBuffer;
    __m128 fract;
    __m128 delay;
    __m128 current_fract;
    int current_delay;
    unsigned int readPos;
    unsigned int writePos;
    unsigned int delayLineSize;
};



