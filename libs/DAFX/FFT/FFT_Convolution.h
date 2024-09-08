
#pragma once
#include "JuceHeader.h"
#include "FFT.h"
template<typename T>
class FFT_Convolution {

public:
    FFT_Convolution() {}
    ~FFT_Convolution() {}
    
    
    FFT_Convolution(juce::AudioBuffer<float>& IR, int FFTSIZE)
    {
        //Pad Buffer and set array sizes
        fftSize = FFTSIZE;
        fft = std::make_unique<FFT<T>>((fftSize));
        int tempParts = ((IR.getNumSamples() / fftSize) + 1);
        size = tempParts * FFTSIZE;
        juce::AudioBuffer<float> tempBuffer;
        tempBuffer.setSize(size,1);
        tempBuffer.clear();
        tempBuffer.copyFrom(0, 0, IR, 0, 0, IR.getNumSamples());
      
        std::vector<typename FFT<T>::ComplexT> tempFDB;
        //Resize
        tempFDB.resize(fftSize);
        //Clear
        std::fill(tempFDB.begin(),tempFDB.end(),0);
        //Resize
        FDIRB.resize(size);
        FDL.resize(size);
        FDB.resize(fftSize);
        
        int offset = 0;
        float* timeDomainData = tempBuffer.getWritePointer(0);
        //Perpare Data
        for (int parts = 0; parts < tempParts; parts++) {
            
            //Load temporary data
            for(int i = 0; i < fftSize; i++) {
                tempFDB[i].real(timeDomainData[i + offset]);
                tempFDB[i].imag = 0;
            }
            //clear previous data
            std::fill(tempFDB.begin(),tempFDB.end(),0);
            //analysis
            fft->perform(tempFDB.data(),fftSize,fft->twiddle.data());
            //Copy into Frequency Domain Impulse Response Buffer
            for(int i = 0; i < fftSize; i++) {
                FDIRB[i + offset].real(tempFDB[i].real());
                FDIRB[i + offset].imag(tempFDB[i].imag());
            }
            //advance in data
            offset += fftSize;
        }
    }
    
    
    template <typename ProcessContext>
    void process(const ProcessContext& context) noexcept
    {
        const auto& inputBlock = context.getInputBlock();
        auto& outputBlock = context.getOutputBlock();
        const auto numChannels = outputBlock.getNumChannels();
        const auto numSamples = outputBlock.getNumSamples();
        
        
        auto* inputData = inputBlock.getChannelPointer(0);
        auto* FDBptr = FDB.data();
        
        for(int i = 0; i < fftSize; i++) {
            FDBptr[i].real = inputData[i];
            FDBptr[i].imag = T(0);
        }
        
        fft->perform(FDB.data(),fftSize,fft->twiddle.data());
    
        frequencyDomainDelayLine(FDL.data(),FDB.data());
        
        
        
    }
    
    
    void frequencyDomainDelayLine(typename FFT<T>::ComplexT* freqdl,typename FFT<T>::ComplexT* freqdat)
    {
        //shift everything by fftsize
        // Calculate the number of bytes to shift
        size_t shiftSize = size_t(fftSize) * sizeof(typename FFT<T>::ComplexT);

        // Move the contents down by `fftSize` elements
        std::memmove(freqdl, freqdl + fftSize, (size - size_t(fftSize)) * sizeof(typename FFT<T>::ComplexT));
        // Copy new content
        std::memcpy(freqdl, freqdat, shiftSize);
    }
    
    
    
    
    
    
    
    
private:
    //FFT CLASS
    std::unique_ptr<FFT<T>> fft;
    //Frequency Domain Impulse Response Buffer
    std::vector<typename FFT<T>::ComplexT> FDIRB;
    //Frequency Domain Delay Line
  
    std::vector<typename FFT<T>::ComplexT> FDL;
    //Frequency Domain Buffer
    std::vector<typename FFT<T>::ComplexT> FDB;
    //AccumBuffer
    std::vector<typename FFT<T>::ComplexT> AccumBuffer;
    
    size_t size = 0;
    int fftSize = 0;
  
};
