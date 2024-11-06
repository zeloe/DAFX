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

template<typename FloatType>
class DelayLine
{
public:
    DelayLine()
    {
        smoother_DelayTime = std::make_unique<BlockSmoothing<float>>();
        smoother_Fraction = std::make_unique<BlockSmoothing<float>>();
        cubicInter = std::make_unique<Interpolation<FloatType>>();
    }
    ~DelayLine(){}
    
    void prepare(int delayLineSize, int& maxBlockSize, int channels)
    {
        this->channels = channels;
        buffer.resize(delayLineSize);
        std::fill(buffer.begin(), buffer.end(), 0.0f);
       // delBuffer(&buffer);
        //delBuffer.clear();
        size = delayLineSize;
        
        smoother_DelayTime->prepare(maxBlockSize);
        smoother_Fraction->prepare(maxBlockSize);
    }
    
    void setParams(float del)
    {
        float temp = floor(del);
        fract = del - temp;
        delay = temp;
         
        smoother_DelayTime->smooth(delay, current_delay);
        smoother_Fraction->smooth(fract, current_fract);
    }
    
    void incrementDelayLine()
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
     
    
     
    
    
    const FloatType processBlockInter(FloatType input)
    {
       
        
        const FloatType* delRead = buffer.data();
        FloatType* delWrite = buffer.data();

        delWrite[writePointer] = (input);

        if (smoother_DelayTime->isSmoothing)
        {
            readPointer = (writePointer - current_delay);
         
            if (readPointer - 3 < 0)
            {
                readPointer += size;
            }

            const FloatType y0 = delRead[(readPointer - 3) % size];
            const FloatType y1 = delRead[(readPointer - 2) % size];
            const FloatType y2 = delRead[(readPointer - 1) % size];
            const FloatType y3 = delRead[(readPointer) % size];

            const FloatType output = cubicInter->cubic(y0, y1, y2, y3, current_fract);
            this->incrementDelayLine();
            return output;
        }
        else
        {
            const FloatType y0 = delRead[(writePointer - int(current_delay) + size) % size];
            this->incrementDelayLine();
            return y0;
        }
    }
    
    
     
    
    
    
    
    std::unique_ptr<BlockSmoothing<float>> smoother_DelayTime;
    std::unique_ptr<BlockSmoothing<float>> smoother_Fraction;
private:
    std::vector<FloatType> buffer;
    // delBuffer;
    size_t current_bs = 0;
    float fract = 0;
    float delay = 0;
    float current_fract = 0;
    float current_delay = 0;
    std::unique_ptr<Interpolation<FloatType>> cubicInter;
    size_t readPointer = 0;
    size_t writePointer = 0;
    size_t k = 1;
    size_t channels = 0;
    size_t size = 0;
};
 

//not working as expected
template<typename T>
class C_DelayLine
{
public:
    C_DelayLine() {}
    ~C_DelayLine() {}
    //Prepare
    void prepare(int delayLineSize, int blockSize) noexcept {
        size = delayLineSize;
        buffer.resize(size);
        
        
        // fill with 0
        T* ptr = buffer.data();
        for(int i =0; i < size; i++) {
            ptr[i] =T(0.f);
        }
        //initialise pointers (they are nullptrs)
        writeBlock = &ptr[0];
        readBlock = &ptr[1];
        //get actual blocksize
        bs = blockSize;
        //get number of blocks
        numBlocks = delayLineSize / blockSize;
        //set the current slice to 0
        currentBlock = 0;
        delay = 0;
        currentReadIndex = 0;
        canDelay = false;
        return;
    }
    // Increment at end of processing (not in for loop)
    void incrementWriteBlock() {
        currentBlock = (currentBlock + bs) % size;
        return;
    }
    //  get Delay from GUI or other thread
    //  delay should be in samples (integer)
    //  this will be currentBlock - integer
    //  (reading previous samples)
    //  set this outside for loop
    void getDelay(int nextReadIndex) {
        T* ptr = buffer.data();
        delay = nextReadIndex;
       // currentReadIndex = nextReadIndex;
        
        //delay = (readPosition - currentReadIndex) % size;
      //  readBlock = &ptr[delay];
    }
    // get current Write block at beggining (block based)
    void setWriteBlock() {
        T* ptr = buffer.data();
        writeBlock = &ptr[currentBlock];
        return;
    }
    // here you write to delay line
    //inside for loop
    void write(T inputSample) {
        *writeBlock = inputSample;
        ++writeBlock;
        return;
    }
    //here you get data from delay line
    //inside for loop
    const T read() {
        const T delayedSamples = readBlock[readPosition];
        readPosition = (readPosition + 1 - delay) % size;
        return delayedSamples;
    }
    
    void resetRead() {
        T* ptr = buffer.data();
        if(canDelay)
        readBlock = &ptr[(readPosition) % size];
    }
    
    void getReadPosition() {
        canDelay = false;
        if(buffer.data() - writeBlock >= delay) {
            readPosition =(readPosition - size_t(buffer.data() - writeBlock)) % size;
            canDelay = true;
        }
        return;
    }
    
    
private:
    // vector for holding buffer
    std::vector<T> buffer;
    // actual size of delay line
    size_t size = 0;
    // actual block size
    size_t bs = 0;
    // number of Blocks
    size_t numBlocks = 0;
    // current Block
    size_t currentBlock = 0;
    // pointer for writing to delay line
    T* writeBlock = nullptr;
    // pointer for reading from delay line
    const T* readBlock = nullptr;
    size_t delay = 0;
    int currentReadIndex = 0;
    size_t readPosition = 0;
    bool canDelay = false;
};
