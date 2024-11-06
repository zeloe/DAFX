


#ifndef SPINLOCK_H
#define SPINLOCK_H
#pragma once
#include "JuceHeader.h"
class SpinLock {
public:
    void lock() noexcept {
        while (flag.test_and_set());
    }
    void unlock() noexcept {
        flag.clear();
    }
    bool try_lock() noexcept {
        return ! flag.test_and_set(); 
    }
    
private:
    std::atomic_flag flag = ATOMIC_FLAG_INIT;
    
    
    
    
};
#endif //SPINLOCK_H
