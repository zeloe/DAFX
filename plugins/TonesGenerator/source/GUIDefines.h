#ifndef GUIDEFINES_H
#define GUIDEFINES_H
#pragma once
#include <JuceHeader.h>
#include "pluginparamers/PluginParameters.h"


const int guiwidth = 400;
const int guiheight = 200;


struct sliderDefines {
    int x;
    int y;
    int width;
    int height;
};



const int sliderSize = 100;
inline static std::vector<sliderDefines> sliderPos =  {
    {10,10,sliderSize,sliderSize}, // GAIN
    {110,10,sliderSize,sliderSize} // FREQEUNCY
};

const int tonesHeight = 20;
const int tonesWidth = 100;
const int tonesx = 220;



#endif // GUIDEFINES_H











