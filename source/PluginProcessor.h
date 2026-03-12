#pragma once
#include <JuceHeader.h>

class SquabDanceAudioProcessor : public juce::AudioProcessor
{
public:
    SquabDanceAudioProcessor();
    ~SquabDanceAudioProcessor() override;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    const juce::String getName() const override;
    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    // This handles your parameters (Mirror, X, Y, Sync)
    juce::AudioProcessorValueTreeState apvts;

    // NEW PLAYHEAD VARIABLES ---
    std::atomic<double> currentBpm { 120.0 };
    std::atomic<double> currentPpq { 0.0 };
    std::atomic<bool> isPlaying { false };
    std::atomic<float> currentAudioLevel { 0.0f };

    // OUTPUT METER LEVELS ---
    std::atomic<float> outputLevelL { 0.0f };
    std::atomic<float> outputLevelR { 0.0f };

    // --- VISUAL ANALYSIS SENSORS ---
    std::atomic<float> visualMotion { 0.0f }; // 0.0 to 1.0
    std::atomic<float> visualHue { 0.0f };    // 0.0 to 1.0
    std::atomic<float> visualPan { 0.5f };    // 0.0 to 1.0

private:
    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    
    // --- NEW: FLANGER / DELAY MEMORY ---
    juce::AudioBuffer<float> delayBuffer;
    int delayWritePosition = 0;

    // --- HRTF BINAURAL MEMORY ---
    juce::AudioBuffer<float> itdBuffer;
    int itdWritePosition = 0;

    // --- MULTI-CHANNEL DSP VECTORS ---
    std::vector<float> lpfState;
    std::vector<float> svfLp;
    std::vector<float> svfHp;
    std::vector<float> svfBp;


    float smoothMotion = 0.0f;
    float smoothHue = 0.0f;
    float smoothPan = 0.5f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SquabDanceAudioProcessor)
};