/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
MultiBandEffectAudioProcessor::MultiBandEffectAudioProcessor()
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
    LPF1L = std::make_unique<FirstOrderLowpass>();
    LPF2L = std::make_unique<FirstOrderLowpass>();
    LPF1R = std::make_unique<FirstOrderLowpass>();
    LPF2R = std::make_unique<FirstOrderLowpass>();
    LPF1_L = std::make_unique<FirstOrderLowpass>();
    LPF2_L = std::make_unique<FirstOrderLowpass>();
    LPF1_R = std::make_unique<FirstOrderLowpass>();
    LPF2_R = std::make_unique<FirstOrderLowpass>();
    
   
    
    Gain_High = std::make_unique<Gain_Block>();
    Gain_Mid = std::make_unique<Gain_Block>();
    Gain_Low = std::make_unique<Gain_Block>();

    
    
    
    
    
    gain_high = treeState.getRawParameterValue("Gain_High");
    gain_mid = treeState.getRawParameterValue("Gain_Mid");
    gain_low = treeState.getRawParameterValue("Gain_Low");
    cross_high = treeState.getRawParameterValue("Mid_High");
    cross_low = treeState.getRawParameterValue("Low_Mid");
}

MultiBandEffectAudioProcessor::~MultiBandEffectAudioProcessor()
{
}
juce::AudioProcessorValueTreeState::ParameterLayout
MultiBandEffectAudioProcessor::createParameterLayout()
{
    // create parameters
    // you could also use a array with strings and add them in a for loop
    std::vector <std::unique_ptr<juce::RangedAudioParameter>> params;
    auto pParam = (std::make_unique<juce::AudioParameterFloat>("Low_Mid","Low_Mid",250,5000,500));
    auto pParam2 = (std::make_unique<juce::AudioParameterFloat>("Mid_High","Mid_High",5000,10000,5000));
    auto pParam3 = (std::make_unique<juce::AudioParameterFloat>("Gain_High","Gain_High",-60,6,0));
    auto pParam4 = (std::make_unique<juce::AudioParameterFloat>("Gain_Mid","Gain_Mid",-60,6,0));
    auto pParam5 = (std::make_unique<juce::AudioParameterFloat>("Gain_Low","Gain_Low",-60,6,0));
    
    params.push_back(std::move(pParam));
    params.push_back(std::move(pParam2));
    params.push_back(std::move(pParam3));
    params.push_back(std::move(pParam4));
    params.push_back(std::move(pParam5));
    
    
    
    
    
    return {params.begin(),params.end()};
}
//==============================================================================
const juce::String MultiBandEffectAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool MultiBandEffectAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool MultiBandEffectAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool MultiBandEffectAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double MultiBandEffectAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int MultiBandEffectAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int MultiBandEffectAudioProcessor::getCurrentProgram()
{
    return 0;
}

void MultiBandEffectAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String MultiBandEffectAudioProcessor::getProgramName (int index)
{
    return {};
}

void MultiBandEffectAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void MultiBandEffectAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    LPF1L->prepare(sampleRate, samplesPerBlock);
    LPF2L->prepare(sampleRate, samplesPerBlock);
    LPF1R->prepare(sampleRate, samplesPerBlock);
    LPF2R->prepare(sampleRate, samplesPerBlock);
    LPF1_L->prepare(sampleRate, samplesPerBlock);
    LPF2_L->prepare(sampleRate, samplesPerBlock);
    LPF1_R->prepare(sampleRate, samplesPerBlock);
    LPF2_R->prepare(sampleRate, samplesPerBlock);
    
    highL.setSize(1, samplesPerBlock);
    highR.setSize(1, samplesPerBlock);
    midL.setSize(1, samplesPerBlock);
    midR.setSize(1, samplesPerBlock);
    lowL.setSize(1, samplesPerBlock);
    lowR.setSize(1, samplesPerBlock);
    
    Gain_Low->prepare(samplesPerBlock);
    Gain_Mid->prepare(samplesPerBlock);
    Gain_High->prepare(samplesPerBlock);
   
}

void MultiBandEffectAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool MultiBandEffectAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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

void MultiBandEffectAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    
    highL.copyFrom(0, 0, buffer, 0, 0, buffer.getNumSamples());
    midL.copyFrom(0, 0, buffer, 0, 0, buffer.getNumSamples());
    
    // High = High - Mid (High - LPF1)
    //Mid = Mid
    //Low = Mid - Low (LPF2 - LPF1)
    float* Lmid = midL.getWritePointer(0);
    LPF1L->setParams(*cross_high);
    LPF1L->process(Lmid);
    LPF1_L->setParams(*cross_high);
    LPF1_L->process(Lmid);
    lowL.makeCopyOf(midL);
    float* Llow = lowL.getWritePointer(0);
    LPF2L->setParams(*cross_low);
    LPF2L->process(Llow);
    LPF2_L->setParams(*cross_low);
    LPF2_L->process(Llow);
    float* Lhigh = highL.getWritePointer(0);
    for(int i = 0; i < buffer.getNumSamples(); i++)
    {
        Lhigh[i] -= Lmid[i];
    }
    for(int i = 0; i < buffer.getNumSamples(); i++)
    {
        Lmid[i] -= Llow[i];
    }
    
   
    
    float* outputL = buffer.getWritePointer(0);
    Gain_Low->setGain(pow(10,*gain_low/20));
    Gain_Mid->setGain(pow(10,*gain_mid/20));
    Gain_High->setGain(pow(10,*gain_high/20));
    Gain_Low->process(Llow);
    Gain_Mid->process(Lmid);
    Gain_High->process(Lhigh);
    for(int i = 0; i < buffer.getNumSamples(); i++)
    {
        outputL[i] = Llow[i] + Lmid[i] + Lhigh[i];
    }
    
    
    highR.copyFrom(0, 0, buffer, 1, 0, buffer.getNumSamples());
    midR.copyFrom(0, 0, buffer, 1, 0, buffer.getNumSamples());
   
    // High = High - Mid (High - LPF1)
    //Mid = Mid
    //Low = Mid - Low (LPF2 - LPF1)
   
    float* Rmid = midR.getWritePointer(0);
    LPF1R->setParams(*cross_high);
    LPF1R->process(Rmid);
    LPF1_R->setParams(*cross_high);
    LPF1_R->process(Rmid);
    lowR.makeCopyOf(midR);
    float* Rlow = lowR.getWritePointer(0);
    LPF2R->setParams(*cross_low);
    LPF2R->process(Rlow);
    LPF2_R->setParams(*cross_low);
    LPF2_R->process(Rlow);
    float* Rhigh = highR.getWritePointer(0);
    for(int i = 0; i < buffer.getNumSamples(); i++)
    {
        Rhigh[i] -= Rmid[i];
    }
    for(int i = 0; i < buffer.getNumSamples(); i++)
    {
        Rmid[i] -= Rlow[i];
    }
    
    
    float* outputR = buffer.getWritePointer(1);
    Gain_Low->setGain(pow(10,*gain_low/20));
    Gain_Mid->setGain(pow(10,*gain_mid/20));
    Gain_High->setGain(pow(10,*gain_high/20));
    Gain_Low->process(Rlow);
    Gain_Mid->process(Rmid);
    Gain_High->process(Rhigh);
    for(int i = 0; i < buffer.getNumSamples(); i++)
    {
        outputR[i] = Rlow[i] + Rmid[i] + Rhigh[i];
    }
    
}

//==============================================================================
bool MultiBandEffectAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* MultiBandEffectAudioProcessor::createEditor()
{
    return new juce::GenericAudioProcessorEditor(*this);  // return new MultiBandEffectAudioProcessorEditor (*this);
}

//==============================================================================
void MultiBandEffectAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    juce::MemoryOutputStream mos (destData, true);
           treeState.state.writeToStream(mos);
}

void MultiBandEffectAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
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
    return new MultiBandEffectAudioProcessor();
}
