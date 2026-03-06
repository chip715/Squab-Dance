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

void SquabDanceAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock) {
delayBuffer.setSize(getTotalNumOutputChannels(), (int)(sampleRate * 0.1) + 1);
    delayBuffer.clear();
    delayWritePosition = 0;
}

void SquabDanceAudioProcessor::releaseResources() {}

void SquabDanceAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

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
        for (int i = 0; i < buffer.getNumSamples(); ++i) rms += readPointer[i] * readPointer[i];
    }

    rms = std::sqrt(rms / (totalNumInputChannels * buffer.getNumSamples() + 1e-6f)) * 10.0f;

    float prevLevel = currentAudioLevel.load(std::memory_order_relaxed);

    float smoothedLevel = prevLevel;
    if (rms > prevLevel) smoothedLevel = prevLevel + 0.4f * (rms - prevLevel); 
    else smoothedLevel = prevLevel + 0.08f * (rms - prevLevel); 
    currentAudioLevel.store(juce::jmin(1.0f, smoothedLevel), std::memory_order_relaxed);


    // ========================================================
    // --- OPTIMIZED AUDIO MANIPULATION ENGINE
    // ========================================================
    bool manipOn = *apvts.getRawParameterValue("audio_manip") > 0.5f;
    float dynamicAmt = *apvts.getRawParameterValue("manip_dynamic") / 100.0f; 
    float hueAmt = *apvts.getRawParameterValue("manip_hue") / 100.0f;
    float panAmt = *apvts.getRawParameterValue("manip_pan") / 100.0f;

    float motion = visualMotion.load(std::memory_order_relaxed);
    float hue = visualHue.load(std::memory_order_relaxed);
    float pan = visualPan.load(std::memory_order_relaxed);

    // --- TRUE 1-SECOND TIMER (Fixed!) ---
    static int sampleCounter = 0;
    sampleCounter += buffer.getNumSamples(); // Accurately counts samples instead of blocks
    bool shouldPrintDebug = false;
    
    if (sampleCounter >= 44100) { 
        shouldPrintDebug = true;
        sampleCounter = 0; // Reset timer
        
        DBG("=========================================");
        DBG("3A. DSP CHECK | ManipOn: " << (manipOn ? "YES" : "NO"));
        DBG("3B. KNOBS   | Dyn: " << dynamicAmt << " | Hue: " << hueAmt << " | Pan: " << panAmt);
        DBG("3C. SENSORS | Motion: " << motion << " | Hue: " << hue << " | Pan: " << pan);
    }

    if (manipOn && (dynamicAmt > 0.01f || hueAmt > 0.01f || panAmt > 0.01f)) {

        // 1. SINE WAVEFOLDER MATH
        float drive = 1.0f + (motion * dynamicAmt * 40.0f); 
        
        if (shouldPrintDebug) {
            DBG("3D. ENGINE  | Wavefolder Drive Multiplier: " << drive);
            // Let's test the math on a fake audio signal of 0.5 volume
            float testSig = std::sin((0.5f * drive)) * 0.7f;
            DBG("3E. MATH    | Fake Input: 0.50 -> Mangled Output: " << testSig);
        }

        // 2. PANNING MATH
        float centeredPan = 0.5f + ((pan - 0.5f) * panAmt);
        float leftGain = std::cos(centeredPan * juce::MathConstants<float>::halfPi) * 1.414f;
        float rightGain = std::sin(centeredPan * juce::MathConstants<float>::halfPi) * 1.414f;

        // 3. COMB FILTER / FLANGER MATH
        float delayMs = 0.5f + (hue * 10.0f * hueAmt); 
        int delaySamples = (int)(delayMs * getSampleRate() / 1000.0f);
        if (delaySamples < 1) delaySamples = 1;
        int delayBufferSize = delayBuffer.getNumSamples();

        for (int channel = 0; channel < totalNumInputChannels; ++channel) {
            auto* channelData = buffer.getWritePointer(channel);
            auto* delayData = delayBufferSize > 0 ? delayBuffer.getWritePointer(channel % delayBuffer.getNumChannels()) : nullptr;
            
            int tempDelayPos = delayWritePosition;

            for (int sample = 0; sample < buffer.getNumSamples(); ++sample) {
                float cleanSig = channelData[sample];

                // A. SINE WAVEFOLDER
                if (dynamicAmt > 0.01f) {
                    float pushedSig = cleanSig * drive;
                    cleanSig = std::sin(pushedSig) * 0.7f; 
                }

                // B. HUE COMB FILTER
                if (hueAmt > 0.01f && delayData != nullptr) {
                    int readPos = tempDelayPos - delaySamples;
                    if (readPos < 0) readPos += delayBufferSize;
                    
                    float delayedSig = delayData[readPos];
                    delayData[tempDelayPos] = cleanSig + (delayedSig * 0.6f); 
                    cleanSig = (cleanSig * 0.5f) + (delayedSig * 0.5f);
                    
                    tempDelayPos++;
                    if (tempDelayPos >= delayBufferSize) tempDelayPos = 0;
                }

                // C. AUTO-PANNER
                if (panAmt > 0.01f && totalNumInputChannels == 2) {
                    if (channel == 0) cleanSig *= leftGain;
                    if (channel == 1) cleanSig *= rightGain;
                }

                channelData[sample] = cleanSig;
            }
        }
        
        if (hueAmt > 0.01f && delayBufferSize > 0) {
            delayWritePosition = (delayWritePosition + buffer.getNumSamples()) % delayBufferSize;
        }

    } else {
        delayBuffer.clear(); 
    }

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