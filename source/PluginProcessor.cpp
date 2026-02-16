#include "PluginProcessor.h"
#include "PluginEditor.h"

SquabDanceAudioProcessor::SquabDanceAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                     .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
       apvts(*this, nullptr, "Parameters", createParameterLayout())
#endif
{
}

SquabDanceAudioProcessor::~SquabDanceAudioProcessor() {}

// Define your parameters here
juce::AudioProcessorValueTreeState::ParameterLayout SquabDanceAudioProcessor::createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    // 1. Mirror Toggle
    layout.add(std::make_unique<juce::AudioParameterBool>("mirror", "Mirror Reflection", false));

    // 2. Style Selector (Row 0 to 9)
    // We use a Range of 0-9, with a default of 0 (Row 1)
    layout.add(std::make_unique<juce::AudioParameterInt>("style", "Animation Style", 0, 9, 0));

    // 3. Speed (HZ) - Range 1Hz to 30Hz, default 12Hz
    layout.add(std::make_unique<juce::AudioParameterFloat>("speed", "Speed (Hz)", 1.0f, 30.0f, 12.0f));

    // 4. X and Y Offsets (Keep these for positioning)
    layout.add(std::make_unique<juce::AudioParameterFloat>("xoffset", "X Offset", -500.0f, 500.0f, 0.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("yoffset", "Y Offset", -500.0f, 500.0f, 0.0f));

    return layout;
}

const juce::String SquabDanceAudioProcessor::getName() const { return JucePlugin_Name; }
bool SquabDanceAudioProcessor::acceptsMidi() const { return true; }
bool SquabDanceAudioProcessor::producesMidi() const { return false; }
bool SquabDanceAudioProcessor::isMidiEffect() const { return false; }
double SquabDanceAudioProcessor::getTailLengthSeconds() const { return 0.0; }
int SquabDanceAudioProcessor::getNumPrograms() { return 1; }
int SquabDanceAudioProcessor::getCurrentProgram() { return 0; }
void SquabDanceAudioProcessor::setCurrentProgram (int index) {}
const juce::String SquabDanceAudioProcessor::getProgramName (int index) { return {}; }
void SquabDanceAudioProcessor::changeProgramName (int index, const juce::String& newName) {}

void SquabDanceAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock) {}
void SquabDanceAudioProcessor::releaseResources() {}

void SquabDanceAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    // Audio pass-through (silence cleanup)
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());
}

bool SquabDanceAudioProcessor::hasEditor() const { return true; }
juce::AudioProcessorEditor* SquabDanceAudioProcessor::createEditor()
{
    return new SquabDanceAudioProcessorEditor (*this);
}

void SquabDanceAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // Save state
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml (state.createXml());
    copyXmlToBinary (*xml, destData);
}

void SquabDanceAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // Load state
    std::unique_ptr<juce::XmlElement> xmlState (getXmlFromBinary (data, sizeInBytes));
    if (xmlState.get() != nullptr)
        if (xmlState->hasTagName (apvts.state.getType()))
            apvts.replaceState (juce::ValueTree::fromXml (*xmlState));
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new SquabDanceAudioProcessor();
}