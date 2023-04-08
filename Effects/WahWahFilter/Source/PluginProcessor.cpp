/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
WahWahFilterAudioProcessor::WahWahFilterAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       ) ,treeState(*this, nullptr, "PARAMETERS", createParameterLayout())
#endif
{
    gainBlockDryL = std::make_unique<Gain_Block>();
    gainBlockWetL = std::make_unique<Gain_Block>();
    gainBlockDryR = std::make_unique<Gain_Block>();
    gainBlockWetR = std::make_unique<Gain_Block>();
    bandpassL = std::make_unique<BandpassFilter>();
    bandpassR = std::make_unique<BandpassFilter>();
    waveTable = std::make_unique<SineWave>(512);
    mix = treeState.getRawParameterValue("Mix");
    lfo_freq = treeState.getRawParameterValue("LFO_Freq");
    bw  = treeState.getRawParameterValue("BandWidth");
    cf = treeState.getRawParameterValue("CenterFrequency");
    depth  = treeState.getRawParameterValue("Depth");
    
    
}

WahWahFilterAudioProcessor::~WahWahFilterAudioProcessor()
{
}
juce::AudioProcessorValueTreeState::ParameterLayout
WahWahFilterAudioProcessor::createParameterLayout()
{
    // create parameters
    // you could also use a array with strings and add them in a for loop
    std::vector <std::unique_ptr<juce::RangedAudioParameter>> params;
    
        auto P_Mix = (std::make_unique<juce::AudioParameterFloat>("Mix","Mix",0.0,1.0,0.5));
        params.push_back(std::move(P_Mix));
        auto P_LFO_Freq = (std::make_unique<juce::AudioParameterFloat>("LFO_Freq","LFO_Freq",2,10,5));
        params.push_back(std::move(P_LFO_Freq));
        auto P_BandWidth = (std::make_unique<juce::AudioParameterFloat>("BandWidth","BandWidth",10,100,50));
        params.push_back(std::move(P_BandWidth));
        auto P_CenterFrequency = (std::make_unique<juce::AudioParameterFloat>("CenterFrequency","CenterFrequency",10,15000,3000));
        params.push_back(std::move(P_CenterFrequency));
        auto P_Depth = (std::make_unique<juce::AudioParameterFloat>("Depth","Depth",200,5000,500));
        params.push_back(std::move(P_Depth));
    
    
    
    return {params.begin(),params.end()};
}


//==============================================================================
const juce::String WahWahFilterAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool WahWahFilterAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool WahWahFilterAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool WahWahFilterAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double WahWahFilterAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int WahWahFilterAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int WahWahFilterAudioProcessor::getCurrentProgram()
{
    return 0;
}

void WahWahFilterAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String WahWahFilterAudioProcessor::getProgramName (int index)
{
    return {};
}

void WahWahFilterAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void WahWahFilterAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    gainBlockDryL->prepare(samplesPerBlock);
    gainBlockWetL->prepare(samplesPerBlock);
    gainBlockDryR->prepare(samplesPerBlock);
    gainBlockWetR->prepare(samplesPerBlock);
    bandpassL->prepare(sampleRate, samplesPerBlock);
    bandpassR->prepare(sampleRate, samplesPerBlock);
    waveTable->prepare(sampleRate, samplesPerBlock);
    LeftDry.setSize(1, samplesPerBlock);
    LeftWet.setSize(1, samplesPerBlock);
    RightDry.setSize(1, samplesPerBlock);
    RightWet.setSize(1, samplesPerBlock);
}

void WahWahFilterAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool WahWahFilterAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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

void WahWahFilterAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;

    
    LeftDry.copyFrom(0, 0, buffer, 0, 0, buffer.getNumSamples());
    LeftWet.copyFrom(0, 0, buffer, 0, 0, buffer.getNumSamples());
    float* dryLeft = LeftDry.getWritePointer(0);
    gainBlockDryL->setGain(1.0 - *mix);
    gainBlockDryL->process(dryLeft);
    gainBlockWetL->setGain(*mix);
    float* wetLeft = LeftWet.getWritePointer(0);
    gainBlockWetL->process(wetLeft);
    
    waveTable->setFreq(*lfo_freq);
    bandpassL->setParams(*cf + waveTable->process() * *depth, *bw);
    bandpassL->process(wetLeft);
    float* outLeft = buffer.getWritePointer(0);
    for(int i = 0; i < buffer.getNumSamples(); i++)
    {
        outLeft[i] = wetLeft[i] + dryLeft[i];
    }
    
    RightDry.copyFrom(0, 0, buffer, 1, 0, buffer.getNumSamples());
    RightWet.copyFrom(0, 0, buffer, 1, 0, buffer.getNumSamples());
    
    float* dryRight = RightDry.getWritePointer(0);
    gainBlockDryR->setGain(1.0 - *mix);
    gainBlockDryR->process(dryRight);
    float* wetRight = RightWet.getWritePointer(0);
    
    gainBlockWetR->setGain(*mix);
    gainBlockWetR->process(wetRight);
    
    waveTable->setFreq(*lfo_freq);
    bandpassR->setParams(*cf + waveTable->process() * *depth, *bw);
    bandpassR->process(wetRight);
    float* outRight = buffer.getWritePointer(1);
    for(int i = 0; i < buffer.getNumSamples(); i++)
    {
        outRight[i] = wetRight[i] + dryRight[i];
    }
    
}

//==============================================================================
bool WahWahFilterAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* WahWahFilterAudioProcessor::createEditor()
{
    return new juce::GenericAudioProcessorEditor (*this);//new WahWahFilterAudioProcessorEditor (*this);
}

//==============================================================================
void WahWahFilterAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    juce::MemoryOutputStream mos (destData, true);
       treeState.state.writeToStream(mos);
}

void WahWahFilterAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
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
    return new WahWahFilterAudioProcessor();
}
