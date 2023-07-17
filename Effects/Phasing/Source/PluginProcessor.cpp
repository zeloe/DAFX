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
    filterL1 = std::make_unique<SecondOrder_Butterworth_bandstop>();
    filterL2 = std::make_unique<SecondOrder_Butterworth_bandstop>();
    filterL3 = std::make_unique<SecondOrder_Butterworth_bandstop>();
   
    
    dryR = std::make_unique<Gain_Block>();
    wetR = std::make_unique<Gain_Block>();
    dryR = std::make_unique<Gain_Block>();
    wetR = std::make_unique<Gain_Block>();
    filterR1 = std::make_unique<SecondOrder_Butterworth_bandstop>();
    filterR2 = std::make_unique<SecondOrder_Butterworth_bandstop>();
    filterR3 = std::make_unique<SecondOrder_Butterworth_bandstop>();
    //not really sure about shared pointers
    hannData = std::make_shared<Hanning>();
    possine_LFO = std::make_shared<WaveTableOscillator<Hanning>>(hannData);
    
    triData = std::make_shared<Triangle>();
    tri_LFO = std::make_shared<WaveTableOscillator<Triangle>>(triData);
    lfo_freq = treeState.getRawParameterValue("LFO_Freq");
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
    
        
        auto P_LFO_Freq = (std::make_unique<juce::AudioParameterFloat>("LFO_Freq","LFO_Freq",0.1,1.0,0.5));
        params.push_back(std::move(P_LFO_Freq));
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
    possine_LFO->prepare(sampleRate, samplesPerBlock);
    tri_LFO->prepare(sampleRate, samplesPerBlock);
    dryR->prepare(samplesPerBlock);
    wetR->prepare(samplesPerBlock);
    filterR1->prepare(sampleRate, samplesPerBlock);
    filterR2->prepare(sampleRate, samplesPerBlock);
    filterR3->prepare(sampleRate, samplesPerBlock);
    
    
    LeftWet.setSize(1, samplesPerBlock);
    LeftWet2.setSize(1, samplesPerBlock);
    LeftWet3.setSize(1, samplesPerBlock);
   
    RightWet.setSize(1, samplesPerBlock);
    RightWet2.setSize(1, samplesPerBlock);
    RightWet3.setSize(1, samplesPerBlock);
    
    
    
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
    
    
    LeftWet.copyFrom(0, 0, buffer, 0, 0, buffer.getNumSamples());
    LeftWet2.copyFrom(0, 0, buffer, 0, 0, buffer.getNumSamples());
    LeftWet3.copyFrom(0, 0, buffer, 0, 0, buffer.getNumSamples());
    
    float* wetLeft = LeftWet.getWritePointer(0);
   
    float* wetLeft2 = LeftWet2.getWritePointer(0);
    
    float* wetLeft3 = LeftWet3.getWritePointer(0);
    //change this 
    float bw = 5;
    possine_LFO->setFrequency(*lfo_freq);
    filterL1->setParams( 1000+ possine_LFO->getNextSample() * 1000 + 100,bw);
    filterL1->process(wetLeft);
   
    possine_LFO->setFrequency(*lfo_freq);
    filterL2->setParams(2000+ possine_LFO->getNextSample() * 1000 + 100,  bw);
    filterL2->process(wetLeft2);
  
    possine_LFO->setFrequency(*lfo_freq);
    filterL3->setParams(3000+ possine_LFO->getNextSample() * 1000 + 100 , bw);
    filterL3->process(wetLeft3);
   
   
   
    
    float* outLeft = buffer.getWritePointer(0);
    for(int i = 0; i < buffer.getNumSamples(); i++)
    {
        outLeft[i] = (wetLeft[i] + wetLeft2[i] + wetLeft3[i]);
    }
    
    
    RightWet.copyFrom(0, 0, buffer, 1, 0, buffer.getNumSamples());
    RightWet2.copyFrom(0, 0, buffer, 1, 0, buffer.getNumSamples());
    RightWet3.copyFrom(0, 0, buffer, 1, 0, buffer.getNumSamples());
   
    float* wetRight = RightWet.getWritePointer(0);
    
    float* wetRight2 = RightWet2.getWritePointer(0);
    
    float* wetRight3 = RightWet3.getWritePointer(0);
    
    possine_LFO->setFrequency(*lfo_freq);
    filterR1->setParams(1000+ possine_LFO->getNextSample() * 1000 + 100, bw);
    filterR1->process(wetRight);
    
    possine_LFO->setFrequency(*lfo_freq);
    filterR2->setParams( 2000 + possine_LFO->getNextSample() * 1000 + 100, bw);
    filterR2->process(wetRight2);
    
    possine_LFO->setFrequency(*lfo_freq);
    filterR3->setParams(3000+ possine_LFO->getNextSample() * 1000 + 100, bw);
    filterR3->process(wetRight3);
   
   
   
    
    float* outRight = buffer.getWritePointer(1);
    for(int i = 0; i < buffer.getNumSamples(); i++)
    {
        outRight[i] = (wetRight[i] + wetRight2[i]  + wetRight3[i]);
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
