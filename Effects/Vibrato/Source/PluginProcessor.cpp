/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
VibratoAudioProcessor::VibratoAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       ),treeState(*this, nullptr, "PARAMETERS", createParameterLayout())
#endif
{
    sine_LFO = std::make_unique<SineWave>(512);
    delayL = std::make_unique<DelayLine>();
    delayR = std::make_unique<DelayLine>();
    dryL = std::make_unique<Gain_Block>();
    dryR = std::make_unique<Gain_Block>();
    wetL = std::make_unique<Gain_Block>();
    wetR = std::make_unique<Gain_Block>();
    mix = treeState.getRawParameterValue("Mix");
    lfo_freq = treeState.getRawParameterValue("Mod_Freq");
    width  = treeState.getRawParameterValue("Width");
    
    
}

VibratoAudioProcessor::~VibratoAudioProcessor()
{
}

juce::AudioProcessorValueTreeState::ParameterLayout
VibratoAudioProcessor::createParameterLayout()
{
    // create parameters
    // you could also use a array with strings and add them in a for loop
    std::vector <std::unique_ptr<juce::RangedAudioParameter>> params;
    auto pParam = (std::make_unique<juce::AudioParameterFloat>("Mod_Freq","Mod_Freq",0.1,30,0.1));
    //Width sould < delayLineSize
    auto pParam2 = (std::make_unique<juce::AudioParameterFloat>("Width","Width",10,10000,1000));
    auto pParam3 = (std::make_unique<juce::AudioParameterFloat>("Mix","Mix",0,1,0.75));
    params.push_back(std::move(pParam));
    params.push_back(std::move(pParam2));
    params.push_back(std::move(pParam3));
    
    
    
    return {params.begin(),params.end()};
}




//==============================================================================
const juce::String VibratoAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool VibratoAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool VibratoAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool VibratoAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double VibratoAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int VibratoAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int VibratoAudioProcessor::getCurrentProgram()
{
    return 0;
}

void VibratoAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String VibratoAudioProcessor::getProgramName (int index)
{
    return {};
}

void VibratoAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void VibratoAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    sine_LFO->prepare(sampleRate, samplesPerBlock);
    delayL->prepare(sampleRate, samplesPerBlock);
    delayR->prepare(sampleRate, samplesPerBlock);
    leftWet.setSize(1, samplesPerBlock);
    rightWet.setSize(1, samplesPerBlock);
    leftDry.setSize(1, samplesPerBlock);
    rightDry.setSize(1, samplesPerBlock);
    
    dryL->prepare(samplesPerBlock);
    dryR->prepare(samplesPerBlock);
    wetL->prepare(samplesPerBlock);
    wetR->prepare(samplesPerBlock);
}

void VibratoAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool VibratoAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void VibratoAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    
    
    leftDry.copyFrom(0, 0, buffer, 0, 0, buffer.getNumSamples());
    leftWet.copyFrom(0, 0, buffer, 0, 0, buffer.getNumSamples());
    
    dryL->setGain(1.0 - *mix);
    float* dryLeft = leftDry.getWritePointer(0);
    dryL->process(dryLeft);
    
    
    float* wetLeft = leftWet.getWritePointer(0);
    wetL->setGain(*mix);
    wetL->process(wetLeft);
    
    sine_LFO->setFreq(*lfo_freq);
    delayL->setParams(*width + 1 +sine_LFO->process() * *width);
    delayL->process(wetLeft);
    float* outLeft = buffer.getWritePointer(0);
    for(int i = 0; i < buffer.getNumSamples(); i++)
    {
        outLeft[i] = dryLeft[i] + wetLeft[i];
    }
    
    
    rightDry.copyFrom(0, 0, buffer, 1, 0, buffer.getNumSamples());
    rightWet.copyFrom(0, 0, buffer, 1, 0, buffer.getNumSamples());
    
    
    
    dryR->setGain(1.0 - *mix);
    float* dryRight = rightDry.getWritePointer(0);
    dryR->process(dryRight);
    
    
    float* wetRight = rightWet.getWritePointer(0);
    wetR->setGain(*mix);
    wetR->process(wetRight);
    
    sine_LFO->setFreq(*lfo_freq);
    delayR->setParams(*width + 1 +sine_LFO->process() * *width);
    delayR->process(wetRight);
    float* outRight = buffer.getWritePointer(1);
    for(int i = 0; i < buffer.getNumSamples(); i++)
    {
        outRight[i] = dryRight[i] + wetRight[i];
    }
    
}

//==============================================================================
bool VibratoAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* VibratoAudioProcessor::createEditor()
{
    return new juce::GenericAudioProcessorEditor (*this);//return new VibratoAudioProcessorEditor (*this);
}

//==============================================================================
void VibratoAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    juce::MemoryOutputStream mos (destData, true);
           treeState.state.writeToStream(mos);
}

void VibratoAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    auto tree = juce::ValueTree::readFromData(data, sizeInBytes);
                                     if(tree.isValid() )
                                     {
                                         treeState.replaceState(tree);
                                     }
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new VibratoAudioProcessor();
}
