/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
PhasingAudioProcessor::PhasingAudioProcessor()
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
    dryL = std::make_unique<Gain_Block>();
    wetL = std::make_unique<Gain_Block>();
    filterL1 = std::make_unique<BandrecjectFilter>();
    filterL2 = std::make_unique<BandrecjectFilter>();
    filterL3 = std::make_unique<BandrecjectFilter>();
   
    
    dryR = std::make_unique<Gain_Block>();
    wetR = std::make_unique<Gain_Block>();
    dryR = std::make_unique<Gain_Block>();
    wetR = std::make_unique<Gain_Block>();
    filterR1 = std::make_unique<BandrecjectFilter>();
    filterR2 = std::make_unique<BandrecjectFilter>();
    filterR3 = std::make_unique<BandrecjectFilter>();
    waveTable = std::make_unique<SineWave>(512);
    mix = treeState.getRawParameterValue("Mix");
    lfo_freq = treeState.getRawParameterValue("LFO_Freq");
    bandwidth = treeState.getRawParameterValue("BandWidth");
    depth = treeState.getRawParameterValue("Depth");
    cf = treeState.getRawParameterValue("CenterFrequency");
}

PhasingAudioProcessor::~PhasingAudioProcessor()
{
}

juce::AudioProcessorValueTreeState::ParameterLayout
PhasingAudioProcessor::createParameterLayout()
{
    // create parameters
    // you could also use a array with strings and add them in a for loop
    std::vector <std::unique_ptr<juce::RangedAudioParameter>> params;
    
        auto P_Mix = (std::make_unique<juce::AudioParameterFloat>("Mix","Mix",0.0,1.0,0.5));
        params.push_back(std::move(P_Mix));
        auto P_LFO_Freq = (std::make_unique<juce::AudioParameterFloat>("LFO_Freq","LFO_Freq",0.1,10,0.5));
        params.push_back(std::move(P_LFO_Freq));
        auto P_BandWidth = (std::make_unique<juce::AudioParameterFloat>("BandWidth","BandWidth",10,99,50));
        params.push_back(std::move(P_BandWidth));
        auto P_Depth = (std::make_unique<juce::AudioParameterFloat>("Depth","Depth",10,100,50));
        params.push_back(std::move(P_Depth));
        auto P_Center_Freq = (std::make_unique<juce::AudioParameterFloat>("CenterFrequency","CenterFrequency",100,16000,500));
        params.push_back(std::move(P_Center_Freq));
    
    
    
    return {params.begin(),params.end()};
}

//==============================================================================
const juce::String PhasingAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool PhasingAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool PhasingAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool PhasingAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double PhasingAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int PhasingAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int PhasingAudioProcessor::getCurrentProgram()
{
    return 0;
}

void PhasingAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String PhasingAudioProcessor::getProgramName (int index)
{
    return {};
}

void PhasingAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void PhasingAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    dryL->prepare(samplesPerBlock);
    wetL->prepare(samplesPerBlock);
    filterL1->prepare(sampleRate, samplesPerBlock);
    filterL2->prepare(sampleRate, samplesPerBlock);
    filterL3->prepare(sampleRate, samplesPerBlock);
    waveTable->prepare(sampleRate, samplesPerBlock);
    
    dryR->prepare(samplesPerBlock);
    wetR->prepare(samplesPerBlock);
    filterR1->prepare(sampleRate, samplesPerBlock);
    filterR2->prepare(sampleRate, samplesPerBlock);
    filterR3->prepare(sampleRate, samplesPerBlock);
    
    LeftDry.setSize(1, samplesPerBlock);
    LeftWet.setSize(1, samplesPerBlock);
    
    RightDry.setSize(1, samplesPerBlock);
    RightWet.setSize(1, samplesPerBlock);
    
    
}

void PhasingAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool PhasingAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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

void PhasingAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    
    
    LeftDry.copyFrom(0, 0, buffer, 0, 0, buffer.getNumSamples());
    LeftWet.copyFrom(0, 0, buffer, 0, 0, buffer.getNumSamples());
    
    float* dryLeft = LeftDry.getWritePointer(0);
    dryL->setGain(1.0 - *mix);
    dryL->process(dryLeft);
    wetL->setGain(*mix);
    float* wetLeft = LeftWet.getWritePointer(0);
    wetL->process(wetLeft);
    
    
    waveTable->setFreq(*lfo_freq);
    filterL1->setParams(*cf + 100 + waveTable->process() *  *depth, *bandwidth);
    filterL1->process(wetLeft);
    waveTable->setFreq(*lfo_freq + 0.1);
    filterL2->setParams(*cf + 200 + waveTable->process() * *depth, *bandwidth);
    filterL2->process(wetLeft);
    waveTable->setFreq(*lfo_freq + 0.2);
    filterL3->setParams(*cf + 300 + waveTable->process() * *depth, *bandwidth);
    filterL3->process(wetLeft);
   
   
   
    
    float* outLeft = buffer.getWritePointer(0);
    for(int i = 0; i < buffer.getNumSamples(); i++)
    {
        outLeft[i] = wetLeft[i] + dryLeft[i];
    }
    
    
    RightDry.copyFrom(0, 0, buffer, 1, 0, buffer.getNumSamples());
    RightWet.copyFrom(0, 0, buffer, 1, 0, buffer.getNumSamples());
    
    float* dryRight = RightDry.getWritePointer(0);
    dryR->setGain(1.0 - *mix);
    dryR->process(dryRight);
    wetR->setGain(*mix);
    float* wetRight = RightWet.getWritePointer(0);
    wetR->process(wetRight);
    
    
    waveTable->setFreq(*lfo_freq);
    filterR1->setParams(*cf + 100 +  waveTable->process() *  *depth, *bandwidth);
    filterR1->process(wetRight);
    waveTable->setFreq(*lfo_freq + 0.1);
    filterR2->setParams(*cf + 200 +   waveTable->process() * *depth, *bandwidth);
    filterR2->process(wetRight);
    waveTable->setFreq(*lfo_freq + 0.2);
    filterR3->setParams(*cf + 300 +  waveTable->process() * *depth, *bandwidth);
    filterR3->process(wetRight);
   
   
   
    
    float* outRight = buffer.getWritePointer(1);
    for(int i = 0; i < buffer.getNumSamples(); i++)
    {
        outRight[i] = wetRight[i] + dryRight[i];
    }
    
}

//==============================================================================
bool PhasingAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* PhasingAudioProcessor::createEditor()
{
    return  new juce::GenericAudioProcessorEditor(*this);//new PhasingAudioProcessorEditor (*this);
}

//==============================================================================
void PhasingAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    juce::MemoryOutputStream mos (destData, true);
           treeState.state.writeToStream(mos);
}

void PhasingAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
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
    return new PhasingAudioProcessor();
}
