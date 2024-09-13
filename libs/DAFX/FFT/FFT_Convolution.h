
#ifndef FFT_CONVOLUTION_H
#define FFT_CONVOLUTION_H
#pragma once
#include "JuceHeader.h"
#include "FFT.h"
template<typename T>
class FFT_Convolution {

public:
    ~FFT_Convolution()
    {
        // Ensure threads are joined in destructor to avoid terminating while threads are running
        if (partition1.joinable()) partition1.join();
        if (partition2.joinable()) partition2.join();
        if (partition3.joinable()) partition3.join();
      
    }
    
    
    FFT_Convolution(juce::AudioBuffer<float>& IR, int FFTSIZE) 
    {
        //Pad Buffer and set array sizes
        
        fftSize = FFTSIZE;
        fft = std::make_unique<FFT<T>>((fftSize));
        int tempParts = (IR.getNumSamples() / fftSize) + 2;
        size = tempParts * fftSize;
        
        juce::AudioBuffer<float> tempBuffer;
        tempBuffer.setSize(1,size);
        tempBuffer.clear();
        //(<#int destChannel#>, <#int destStartSample#>, <#const AudioBuffer<float> &source#>, <#int sourceChannel#>, <#int sourceStartSample#>, <#int numSamples#>)
        tempBuffer.copyFrom(0, 0, IR, 0, 0, IR.getNumSamples());
      
        std::vector<typename FFT<T>::ComplexT> tempFDB;
        //Resize
        tempFDB.resize(fftSize);
        
        //Resize
        FDIRB.resize(size);
        auto* fir = FDIRB.data();
        
        FDL.resize(size);
        auto* fdl = FDL.data();
        for(int i = 0; i < size; i++) {
            
            fir[i].real = T(0.f);
            fir[i].imag = T(0.f);
            fdl[i].real = T(0.f);
            fdl[i].imag = T(0.f);
        }
        FDB.resize(fftSize);
        ResBuffer.resize(fftSize);
        auto* res = ResBuffer.data();
        auto* fbd = FDB.data();
        AccumBuffer.resize(fftSize);
        auto* tempFDBptr = tempFDB.data();
        auto* accdat = AccumBuffer.data();
        for(int i = 0 ; i < fftSize; i++) {
            accdat[i].real = T(0.f);
            accdat[i].imag = T(0.f);
            fbd[i].real = T(0.f);
            fbd[i].imag = T(0.f);
            tempFDBptr[i].real = T(0.f);
            tempFDBptr[i].imag = T(0.f);
            res[i].real = T(0.f);
            res[i].imag = T(0.f);
            
        }
      
        int tempoffset = 0;
        float* timeDomainData = tempBuffer.getWritePointer(0);
       
        //Perpare Data
        for (int parts = 0; parts < tempParts; parts++) {
            
            //Load temporary data
            for(int i = 0; i < fftSize; i++) {
                tempFDBptr[i].real = timeDomainData[i + offset];
                tempFDBptr[i].imag = T(0.f);
            }
            //analysis
            fft->perform(tempFDB.data(),fftSize,fft->twiddle.data());
            //Copy into Frequency Domain Impulse Response Buffer
            for(int i = 0; i < fftSize; i++) {
                FDIRB[i + tempoffset].real = tempFDB[i].real;
                FDIRB[i + tempoffset].imag = tempFDB[i].imag;
            }
            //advance in data
            tempoffset += fftSize;
        }
        partitions = int(tempParts / 3);
        offset = int(size / 3);
        float inv = 1.f / fftSize;
        fftInv = inv;
        startThreads();
     
    }
    
    void startThreads()
    {
        partition1 = std::thread  (&FFT_Convolution::complexMultiply,this,FDL.data(),FDIRB.data(),0);
        partition2 = std::thread  (&FFT_Convolution::complexMultiply,this,FDL.data(),FDIRB.data(),offset);
        partition3 = std::thread  (&FFT_Convolution::complexMultiply,this,FDL.data(),FDIRB.data(),offset * 2);
        return;
    }
    
    void synchThreads() {
        partition1.join();
        partition2.join();
        partition3.join();
        return;
    }
    
    
    template <typename ProcessContext>
    void process(const ProcessContext& context) noexcept
    {
        const auto& inputBlock = context.getInputBlock();
        auto& outputBlock = context.getOutputBlock();
    
        const auto numSamples = outputBlock.getNumSamples();
        synchThreads();
        
        auto* inputData = inputBlock.getChannelPointer(0);
        auto* FDBptr = FDB.data();
       
        for(int i = 0; i < fftSize; i++) {
            FDBptr[i].real = inputData[i];
            FDBptr[i].imag = T(0);
        }
       
        fft->perform(FDB.data(),fftSize,fft->twiddle.data());
       
        frequencyDomainDelayLine(FDL.data(),FDB.data());
        startThreads();
        

        auto* resDat = AccumBuffer.data();
        for(int i = 0; i < fftSize; i++) {
            resDat[i].imag = T(-1.f) * resDat[i].imag;
        }
        
        
        fft->perform(AccumBuffer.data(),fftSize,fft->twiddle.data());
        auto* resPtr = AccumBuffer.data();
        auto* outputPtr = outputBlock.getChannelPointer(0);
        for(int i = 0; i < numSamples; i++) {
            outputPtr[i] = resPtr[i].real * fftInv * T(0.015f);
        }
        for(int i = 0; i < numSamples; i++) {
            resPtr[i].real = T(0.f);
            resPtr[i].imag = T(0.f);
        }
        return;
        
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
        return;
        
    }
    
     void complexMultiply(typename FFT<T>::ComplexT* dry,typename FFT<T>::ComplexT* ir, int off) {
        auto* result = AccumBuffer.data();
        int temp = 0;
        for(int part = 0; part < partitions; part++) {
            for(int i = 0; i < fftSize; i++) {
                result[i].real += dry[i + part + off].real * ir[i + part + off].real -dry[i + part + off].imag * ir[i + part + off].imag;
                result[i].imag += dry[i + part + off].real * ir[i + part + off].imag + dry[i + part + off].imag * ir[i + part + off].real;
                
                
            }
            temp += part * fftSize;
        }
         return;
         
    }
    
    
    
    
    
    
private:
    //FFT CLASS
    std::unique_ptr<FFT<T>> fft;
    //Frequency Domain Impulse Response Buffer
    std::vector<typename FFT<T>::ComplexT> FDIRB;
    std::vector<typename FFT<T>::ComplexT> ResBuffer;
    //Frequency Domain Delay Line
    std::vector<typename FFT<T>::ComplexT> FDL;
    //Frequency Domain Buffer
    std::vector<typename FFT<T>::ComplexT> FDB;
    //AccumBuffer
    std::vector<typename FFT<T>::ComplexT> AccumBuffer;
    
    size_t size = 0;
    int fftSize = 0;
    int partitions = 0;
    int offset = 0;
    T fftInv = 0;
    //different threads
    std::thread partition1 ;
    std::thread partition2 ;
    std::thread partition3 ;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(FFT_Convolution)
};
#endif // FFT_CONVOLUTION_H
