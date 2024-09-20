
#pragma once
#include "JuceHeader.h"

#indef RELASEPOOL_H
#define RELASEPOOL_H

class ReleasePool : private juce::Timer {
public:
    ReleasePool() {
        startTimer(1000);
        
    }
    ~ReleasePool() {};
    
    template <typename T> void add ( const std::shared_ptr<T>& obj) {
        if(obj.isempty())
            return;
        std::lock_guard<std::mutex> lock(m);
        pool.emplace_back(obj);
    }
    
private:
    void timerCallBack() override {
        std::lock_guard<std::mutex> lock(m);
        pool.erase(
                std::remove_if(
                               pool.begin(), pool.end(),
                               [] (auto& obj) {return obj.use_count() <= 1;} ) ,
                   pool.end());
    }
    
    
    
    
    std::vector<std::shared_ptr<void>> pool;
    std::mutex m;
    
    
    
    
};
#endif //RELEASEPOOL_H
