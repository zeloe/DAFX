# DAFX
 C++ implementation of Matlabscripts from the book DAFX: Digital Audio Effects by Udo Zölzer. \
 I use [JUCE](https://juce.com/get-juce/) to debug VST3 plugins inside hosts. \
 I did this for study purposes. 

##
```shell
git clone https://github.com/zeloe/DAFX.git
cd DAFX
git submodule update --init --recursive
```

## Added cmake file
to run install cmake \
depending on your IDE and your operating system run in terminal for mac: \
cmake -G Xcode -B test

## To Do
- Add stereo support for each class
- Refactor FX and make them Cmake compatible

## Not Working
