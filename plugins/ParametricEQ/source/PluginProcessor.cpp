/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
PluginAudioProcessor::PluginAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       ), treeState(*this, nullptr, juce::Identifier("Parameters"), PluginParameter::createParameterLayout())
#endif
{
  
    simdEQ = std::make_unique<SIMDEQ>();
    for (auto param : PluginParameter::getPluginParameterList())
    {
            treeState.addParameterListener(param, this);
    }
    
}

PluginAudioProcessor::~PluginAudioProcessor()
{
    for (auto param : PluginParameter::getPluginParameterList())
        treeState.removeParameterListener(param, this);
}



void PluginAudioProcessor::parameterChanged(const juce::String& parameterID, float newValue)
{
        if (parameterID == PluginParameter::LOW_CUTOFF_FREQUENCY)
        {
            simdEQ->eq->setLowCutoff(newValue);
          
        }

        if (parameterID == PluginParameter::HIGH_CUTOFF_FREQUENCY)
        {
            simdEQ->eq->setHighCutoff(newValue);

        }
        if (parameterID == PluginParameter::HIGH_GAIN)
        {
            simdEQ->eq->setHighGain(newValue);

        }

        if (parameterID == PluginParameter::MID_GAIN)
        {
            simdEQ->eq->setMidGain(newValue);

        }



        if (parameterID == PluginParameter::LOW_GAIN)
        {
            simdEQ->eq->setLowGain(newValue);

        }

}

void PluginAudioProcessor::initParams()
{
    simdEQ->eq->setLowCutoff(treeState.getRawParameterValue(PluginParameter::LOW_CUTOFF_FREQUENCY)->load());
    simdEQ->eq->setHighCutoff(treeState.getRawParameterValue(PluginParameter::HIGH_CUTOFF_FREQUENCY)->load());
    simdEQ->eq->setHighGain(treeState.getRawParameterValue(PluginParameter::HIGH_GAIN)->load());
    simdEQ->eq->setMidGain(treeState.getRawParameterValue(PluginParameter::MID_GAIN)->load());
    simdEQ->eq->setLowGain(treeState.getRawParameterValue(PluginParameter::LOW_GAIN)->load());
}
 
//==============================================================================
const juce::String PluginAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool PluginAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool PluginAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool PluginAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double PluginAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int PluginAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int PluginAudioProcessor::getCurrentProgram()
{
    return 0;
}

void PluginAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String PluginAudioProcessor::getProgramName (int index)
{
    return {};
}

void PluginAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void PluginAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
     juce::dsp::ProcessSpec specs;

    specs.sampleRate = sampleRate;
    specs.maximumBlockSize = samplesPerBlock;
    specs.numChannels = 2;
    simdEQ->prepare(specs);
    this->initParams();
}

void PluginAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool PluginAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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

void PluginAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // Use the actual number of channels from the buffer
    size_t numChannels = buffer.getNumChannels();

    // Prepare the process context with the input and output buffers
    juce::dsp::AudioBlock<float> audioBlock(buffer.getArrayOfWritePointers(), numChannels, buffer.getNumSamples());
    juce::dsp::ProcessContextReplacing<float> context(audioBlock);

    // Ensure thread safety when accessing the SIMD comb filter
    juce::ScopedLock audioLock(audioCallbackLock);

    // Process the audio using the SIMD comb filter
    simdEQ->process(context);
}


//==============================================================================
bool PluginAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* PluginAudioProcessor::createEditor()
{
    return new juce::GenericAudioProcessorEditor (*this);//new PluginAudioProcessorEditor (*this);
}

//==============================================================================
void PluginAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    juce::MemoryOutputStream mos(destData, true);
    treeState.state.writeToStream(mos);
}

void PluginAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    auto tree = juce::ValueTree::readFromData(data, sizeInBytes);
    if (tree.isValid())
    {
        treeState.replaceState(tree);
    }
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new PluginAudioProcessor();
}
