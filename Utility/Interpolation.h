/*
  ==============================================================================

    Interpolation.h
    Created: 9 Apr 2023 9:37:48pm
    Author:  Onez

  ==============================================================================
*/

#pragma once
#include "JuceHeader.h"

inline float linear_interp(float const v0, float const v1, float const t )
{
    return (1 - t) * v0 + t * v1;
}


inline float cubicInterpolation(float y0, float y1, float y2, float y3, float x) {
    const float a0 = y3 - y2 - y0 + y1;
    const float a1 = y0 - y1 - a0;
    const float a2 = y2 - y0;
    const float a3 = y1;
    return a0 * x * x * x + a1 * x * x + a2 * x + a3;
}

inline float splineInterpolation(float y0, float y1, float y2, float y3, float t)
{
    float a0, a1, a2, a3;
    float t2 = t * t;
    a0 = y3 - y2 - y0 + y1;
    a1 = y0 - y1 - a0;
    a2 = y2 - y0;
    a3 = y1;
    return a0 * t * t2 + a1 * t2 + a2 * t + a3;
}

