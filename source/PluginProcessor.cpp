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
    layout.add(std::make_unique<juce::AudioParameterFloat>("react_intensity", "Intensity", 0.0f, 100.0f, 0.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("react_color", "Color", 0.0f, 100.0f, 0.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("react_pump", "Pump", 0.0f, 100.0f, 0.0f));

    // --- AUDIO MANIPULATION ---
    layout.add(std::make_unique<juce::AudioParameterBool>("audio_manip", "Audio Manipulation", false));
    layout.add(std::make_unique<juce::AudioParameterFloat>("manip_dynamic", "Dynamic", 0.0f, 100.0f, 0.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("manip_hue", "Hue Analysis", 0.0f, 100.0f, 0.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("manip_pan", "Panning", 0.0f, 100.0f, 0.0f));

    // --- SATURATION TYPE  ---
    juce::StringArray saturationOptions = { "Wavefolder", "Soft Clip", "Hard Clip", "Bitcrusher" };
    layout.add(std::make_unique<juce::AudioParameterChoice>("sat_type", "Saturation Type", saturationOptions, 0));

    // --- NEW: OUTPUT SECTION ---
    layout.add(std::make_unique<juce::AudioParameterFloat>("dry_wet", "Dry/Wet", 0.0f, 100.0f, 100.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("out_gain", "Output Gain", -60.0f, 12.0f, 0.0f));


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

    // HRTF ITD Micro-Delay (We only need 5 milliseconds of memory for a human head)
    itdBuffer.setSize(2, (int)(sampleRate * 0.005) + 1);
    itdBuffer.clear();
    itdWritePosition = 0;
    lpfStateL = 0.0f;
    lpfStateR = 0.0f;

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

    // Grab these parameters BEFORE the loop so they are ready to use
    float dryWet = *apvts.getRawParameterValue("dry_wet") / 100.0f;
    float outGainDb = *apvts.getRawParameterValue("out_gain");
    float outGainAmt = juce::Decibels::decibelsToGain(outGainDb);
    int satType = (int)*apvts.getRawParameterValue("sat_type");

    float targetMotion = visualMotion.load(std::memory_order_relaxed);
    float targetHue = visualHue.load(std::memory_order_relaxed);
    float targetPan = visualPan.load(std::memory_order_relaxed);

    int delayBufferSize = delayBuffer.getNumSamples();
    int itdBufferSize = itdBuffer.getNumSamples();
    
    if (delayBufferSize < 2 || itdBufferSize < 2 || delayBuffer.getNumChannels() == 0) return;

    float maxItdSamples = 0.00075f * getSampleRate(); 

    // We track peaks out here so they survive the sample loop
    float peakL = 0.0f;
    float peakR = 0.0f;

    for (int sample = 0; sample < buffer.getNumSamples(); ++sample) {
        
        smoothMotion += 0.01f * (targetMotion - smoothMotion);
        smoothHue += 0.002f * (targetHue - smoothHue);
        smoothPan += 0.002f * (targetPan - smoothPan);

        float drive = 1.0f + (smoothMotion * dynamicAmt * 40.0f); 
        float delayMs = 0.1f + (smoothHue * 15.0f * hueAmt); 
        int delaySamples = (int)(delayMs * getSampleRate() / 1000.0f);
        
        if (delaySamples < 1) delaySamples = 1;
        if (delaySamples >= delayBufferSize) delaySamples = delayBufferSize - 1;

        float panAngle = (smoothPan - 0.5f) * panAmt * juce::MathConstants<float>::pi;

        for (int channel = 0; channel < totalNumInputChannels; ++channel) {
            float cleanSig = buffer.getReadPointer(channel)[sample];
            
            // 1. SAVE THE DRY SIGNAL
            float drySig = cleanSig; 

            // ONLY apply the heavy math if Manipulation is turned ON
            if (manipOn && (dynamicAmt > 0.01f || hueAmt > 0.01f || panAmt > 0.01f)) {
                
                // A. SATURATION / DISTORTION ENGINE
                if (dynamicAmt > 0.01f) {
                    float pushedSig = cleanSig * drive;
                    switch (satType) {
                        case 0: cleanSig = std::sin(pushedSig) * 0.7f; break;
                        case 1: cleanSig = std::tanh(pushedSig) * 0.8f; break;
                        case 2: cleanSig = juce::jlimit(-0.8f, 0.8f, pushedSig); break;
                        case 3: 
                            float res = std::pow(2.0f, 2.0f + (1.0f - smoothMotion) * 10.0f);
                            cleanSig = std::round(pushedSig * res) / res;
                            break;
                    }
                }

                // B. HUE COMB FILTER
                if (hueAmt > 0.01f) {
                    auto* delayData = delayBuffer.getWritePointer(channel % delayBuffer.getNumChannels());
                    int readPos = delayWritePosition - delaySamples;
                    while (readPos < 0) readPos += delayBufferSize; 
                    float delayedSig = delayData[readPos];
                    delayData[delayWritePosition] = cleanSig + std::tanh(delayedSig * 0.85f); 
                    cleanSig = (cleanSig * 0.4f) + (delayedSig * 0.6f);
                }

                // C. 3D BINAURAL HRTF PANNING
                if (panAmt > 0.01f && totalNumInputChannels == 2) {
                    float shadowAngle = (channel == 0) ? panAngle : -panAngle; 
                    float penalty = juce::jmax(0.0f, shadowAngle); 
                    float ildGain = std::cos(penalty);
                    cleanSig *= ildGain;

                    float shadowCutoff = 1.0f - juce::jmin(0.9f, penalty * 0.7f);
                    if (channel == 0) {
                        lpfStateL = (cleanSig * shadowCutoff) + (lpfStateL * (1.0f - shadowCutoff));
                        cleanSig = lpfStateL;
                    } else {
                        lpfStateR = (cleanSig * shadowCutoff) + (lpfStateR * (1.0f - shadowCutoff));
                        cleanSig = lpfStateR;
                    }

                    int itdDelaySamples = (int)((penalty / juce::MathConstants<float>::halfPi) * maxItdSamples);
                    auto* itdData = itdBuffer.getWritePointer(channel);
                    itdData[itdWritePosition] = cleanSig;
                    int readPos = itdWritePosition - itdDelaySamples;
                    while (readPos < 0) readPos += itdBufferSize;
                    cleanSig = itdData[readPos];
                }
            }

            // 2. MIX DRY AND WET SIGNALS
            cleanSig = (drySig * (1.0f - dryWet)) + (cleanSig * dryWet);
            
            // 3. APPLY OUTPUT GAIN
            cleanSig *= outGainAmt;

            // 4. TRACK METER PEAKS
            if (channel == 0) peakL = juce::jmax(peakL, std::abs(cleanSig));
            if (channel == 1) peakR = juce::jmax(peakR, std::abs(cleanSig));

            // 5. WRITE TO BUFFER
            buffer.getWritePointer(channel)[sample] = cleanSig;
        }
        
        // Advance the timelines
        if (manipOn) {
            if (hueAmt > 0.01f) {
                delayWritePosition++;
                if (delayWritePosition >= delayBufferSize) delayWritePosition = 0;
            }
            if (panAmt > 0.01f) {
                itdWritePosition++;
                if (itdWritePosition >= itdBufferSize) itdWritePosition = 0;
            }
        }
    }

    if (!manipOn) {
        delayBuffer.clear(); 
        itdBuffer.clear();
    }

    // Push the final peaks to the UI thread
    outputLevelL.store(peakL, std::memory_order_relaxed);
    outputLevelR.store(peakR, std::memory_order_relaxed);

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