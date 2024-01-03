#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
PhaserDSPAudioProcessor::PhaserDSPAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       ), 
    
    apvts (*this, nullptr, "Parameters", createParameters())

#endif
{
    apvts.addParameterListener("RATE", this);
    apvts.addParameterListener("DEPTH", this);
    apvts.addParameterListener("CENTREFREQUENCY", this);
    apvts.addParameterListener("FEEDBACK", this);
    apvts.addParameterListener("MIX", this);
}

PhaserDSPAudioProcessor::~PhaserDSPAudioProcessor()
{
    apvts.removeParameterListener("RATE", this);
    apvts.removeParameterListener("DEPTH", this);
    apvts.removeParameterListener("CENTREFREQUENCY", this);
    apvts.removeParameterListener("FEEDBACK", this);
    apvts.removeParameterListener("MIX", this);
}

//==============================================================================
const juce::String PhaserDSPAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool PhaserDSPAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool PhaserDSPAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool PhaserDSPAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double PhaserDSPAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int PhaserDSPAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int PhaserDSPAudioProcessor::getCurrentProgram()
{
    return 0;
}

void PhaserDSPAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String PhaserDSPAudioProcessor::getProgramName (int index)
{
    return {};
}

void PhaserDSPAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void PhaserDSPAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    juce::dsp::ProcessSpec spec;
    spec.maximumBlockSize = samplesPerBlock;
    spec.sampleRate = sampleRate;

    phaser.prepare(spec);
    phaser.reset();
}

void PhaserDSPAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool PhaserDSPAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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

void PhaserDSPAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    juce::dsp::AudioBlock<float> sampleBlock(buffer);
    phaser.process(juce::dsp::ProcessContextReplacing<float>(sampleBlock));
}

//==============================================================================
bool PhaserDSPAudioProcessor::hasEditor() const
{
    return false; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* PhaserDSPAudioProcessor::createEditor()
{
    return new PhaserDSPAudioProcessorEditor (*this);
}

//==============================================================================
void PhaserDSPAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void PhaserDSPAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new PhaserDSPAudioProcessor();
}

void PhaserDSPAudioProcessor::reset()
{
    phaser.reset();
}

void PhaserDSPAudioProcessor::parameterChanged(const juce::String& parameterID, float newValue)
{
    if (parameterID == "RATE")
        phaser.setRate(newValue);
    if (parameterID == "DEPTH")
        phaser.setDepth(newValue);
    if (parameterID == "CENTREFREQUENCY")
        phaser.setCentreFrequency(newValue);
    if (parameterID == "FEEDBACK")
        phaser.setFeedback(newValue);
    if (parameterID == "MIX")
        phaser.setMix(newValue);
}

juce::AudioProcessorValueTreeState::ParameterLayout PhaserDSPAudioProcessor::createParameters()
{
    juce::AudioProcessorValueTreeState::ParameterLayout params;

    using Range = juce::NormalisableRange<float>;

    params.add(std::make_unique<juce::AudioParameterFloat>("RATE", "Rate", Range{ 0.0f, 20.0f, 0.05f }, 5.0f));
    params.add(std::make_unique<juce::AudioParameterFloat>("DEPTH", "Depth", Range{ 0.0f, 1.0f, 0.01f }, 0.0f));
    params.add(std::make_unique<juce::AudioParameterInt> ("CENTREFREQUENCY", "Centre Frequency", 0, 300, 100));
    params.add(std::make_unique<juce::AudioParameterFloat>("FEEDBACK", "Feedback", Range{ -1.0f, 1.0f, 0.01f }, 0.0f));
    params.add(std::make_unique<juce::AudioParameterFloat>("MIX", "Mix", Range{ 0.0f, 1.0f, 0.01f }, 0.0f));

    return params;
}