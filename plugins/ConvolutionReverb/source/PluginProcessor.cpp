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
  
    convPipeLine = std::make_unique<FilterPartitionThread>() ;
   
    
}

PluginAudioProcessor::~PluginAudioProcessor()
{
    
}



void PluginAudioProcessor::parameterChanged(const juce::String& parameterID, float newValue)
{
       

}

void PluginAudioProcessor::initParams()
{


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
    //for overlap save method
    specs.maximumBlockSize = 2048;
    specs.numChannels = 2;
    convPipeLine->prepare(specs);
    tempBuffer.setSize(2,specs.maximumBlockSize);
    tempBuffer.clear();
    overlapBuffer.setSize(2, specs.maximumBlockSize);
    overlapBuffer.clear();
    this->initParams();
    bs = samplesPerBlock;
    
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
    
    
    tempBuffer.clear();
    tempBuffer.copyFrom(0, 0, buffer, 0, 0, buffer.getNumSamples());
    tempBuffer.copyFrom(1, 0, buffer, 1, 0, buffer.getNumSamples());
        
     
    // Prepare the process context with the input and output buffers
 
    convPipeLine->push(tempBuffer);
    //copy results back
    
    
    // Retrieve processed data from the SIMD FIFO (for example, after overlap-add processing)
    auto readScoped = convPipeLine->conv->outputFifo->read(buffer.getNumSamples());

        if (readScoped.blockSize1 > 0)
        {
            for (int chan = 0; chan < totalNumOutputChannels; ++chan)
            {
                // Copy processed data from the FIFO buffer back to the output audio buffer
                buffer.copyFrom(chan, 0, convPipeLine->conv->outputFifoBuffer, chan, readScoped.startIndex1, readScoped.blockSize1);
            }
        }

        if (readScoped.blockSize2 > 0)
        {
            for (int chan = 0; chan < totalNumOutputChannels; ++chan)
            {
                // Copy remaining processed data if the FIFO wrapped around
                buffer.copyFrom(chan, readScoped.blockSize1,  convPipeLine->conv->outputFifoBuffer, chan, readScoped.startIndex2, readScoped.blockSize2);
            }
        }

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
