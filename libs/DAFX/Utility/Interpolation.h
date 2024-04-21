/*
  ==============================================================================

    Interpolation.h
    Created: 9 Apr 2023 9:37:48pm
    Author:  Onez

  ==============================================================================
*/

#pragma once
#include "JuceHeader.h"

template<typename FloatType>
class Interpolation {
public:
    static FloatType linear(FloatType v0, FloatType v1, FloatType t) {
        return (1 - t) * v0 + t * v1;
    }

    static FloatType cubic(FloatType y0, FloatType y1, FloatType y2, FloatType y3, FloatType x) {
        const FloatType a0 = y3 - y2 - y0 + y1;
        const FloatType a1 = y0 - y1 - a0;
        const FloatType a2 = y2 - y0;
        const FloatType a3 = y1;
        return a0 * x * x * x + a1 * x * x + a2 * x + a3;
    }

    static FloatType spline(FloatType y0, FloatType y1, FloatType y2, FloatType y3, FloatType t) {
        FloatType a0, a1, a2, a3;
        FloatType t2 = t * t;
        a0 = y3 - y2 - y0 + y1;
        a1 = y0 - y1 - a0;
        a2 = y2 - y0;
        a3 = y1;
        return a0 * t * t2 + a1 * t2 + a2 * t + a3;
    }

    static FloatType allPass(FloatType y0, FloatType y1, FloatType ya_alt, FloatType t) {
        return y1 + (1.0 - t) * y0 - (1.0 - t) * ya_alt;
    }
};
