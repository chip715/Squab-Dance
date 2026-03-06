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
    
    // 3. Scale (0% to 300%, default 100%)
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "scale", 
        "Scale", 
        juce::NormalisableRange<float>(0.0f, 300.0f, 1.0f), 100.0f));
        
    // 4. Speed (HZ) - Range 1Hz to 30Hz, default 12Hz
   layout.add(std::make_unique<juce::AudioParameterFloat>(
    "speed", 
    "Speed", 
    juce::NormalisableRange<float>(1.0f, 30.0f, 1.0f), 30.0f ));

// 1. The Mode Toggle (Hz vs Note)
    layout.add(std::make_unique<juce::AudioParameterBool>("sync_mode", "Sync Mode", false));

    // 2. The Sync Rate Dropdown/Slider
    juce::StringArray syncOptions = { 
        "1/2048", "1/1024", "1/512", "1/256", "1/128", "1/64", "1/48", "1/32", "1/24", 
        "1/16", "1/12", "1/8", "1/6", "3/16", "1/4", "5/16", "1/3", "3/8", "1/2", "3/4", "1" 
    };
    // Default to index 14 (which is "1/4" note)
    layout.add(std::make_unique<juce::AudioParameterChoice>("sync_rate", "Sync Rate", syncOptions, 14));

    // 4. X and Y Offsets (Keep these for positioning)
    layout.add(std::make_unique<juce::AudioParameterFloat>("xoffset", "X Offset", -500.0f, 500.0f, 0.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("yoffset", "Y Offset", -500.0f, 500.0f, 0.0f));

    // --- AUDIO REACTIVITY ---
    layout.add(std::make_unique<juce::AudioParameterBool>("audio_react", "Audio Reactivity", false));
    layout.add(std::make_unique<juce::AudioParameterFloat>("react_intensity", "Intensity", 0.0f, 100.0f, 100.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("react_color", "Color", 0.0f, 100.0f, 100.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("react_pump", "Pump", 0.0f, 100.0f, 100.0f));

    // --- AUDIO MANIPULATION (NEW) ---
    layout.add(std::make_unique<juce::AudioParameterBool>("audio_manip", "Audio Manipulation", false));
    layout.add(std::make_unique<juce::AudioParameterFloat>("manip_dynamic", "Dynamic", 0.0f, 100.0f, 100.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("manip_hue", "Hue Analysis", 0.0f, 100.0f, 100.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("manip_pan", "Panning", 0.0f, 100.0f, 100.0f));


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
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // --- OPTIMIZED PLAYHEAD FETCH ---
    if (auto* ph = getPlayHead()) {
        if (auto pos = ph->getPosition()) { 
            currentBpm.store(pos->getBpm().orFallback(120.0), std::memory_order_relaxed);
            currentPpq.store(pos->getPpqPosition().orFallback(0.0), std::memory_order_relaxed);
            isPlaying.store(pos->getIsPlaying(), std::memory_order_relaxed);
        }
    }

    // --- AUDIO ENVELOPE FOLLOWER ---
    float rms = 0.0f;
    for (int channel = 0; channel < totalNumInputChannels; ++channel) {
        auto* readPointer = buffer.getReadPointer(channel);
        for (int i = 0; i < buffer.getNumSamples(); ++i) {
            rms += readPointer[i] * readPointer[i];
        }
    }
    
    // DRAMATIC BOOST: Scaled to 10.0x for aggressive visual response
    rms = std::sqrt(rms / (totalNumInputChannels * buffer.getNumSamples() + 1e-6f)) * 10.0f;
    
    // ROCK SOLID ENVELOPE: Eliminates the visual trembling
    float prevLevel = currentAudioLevel.load(std::memory_order_relaxed);
    float smoothedLevel = prevLevel;
    
    if (rms > prevLevel) {
        // Fast Attack (Punches instantly on the drum hit)
        smoothedLevel = prevLevel + 0.4f * (rms - prevLevel); 
    } else {
        // Slow Release (Glides smoothly down, ignoring wave ripple)
        smoothedLevel = prevLevel + 0.08f * (rms - prevLevel); 
    }
    
    currentAudioLevel.store(juce::jmin(1.0f, smoothedLevel), std::memory_order_relaxed);

    // Audio pass-through (silence cleanup)
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