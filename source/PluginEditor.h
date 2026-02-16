#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "SpriteData.h"   // Include our database
#include "SpriteWindow.h" // Include our window

class SquabDanceAudioProcessorEditor : public juce::AudioProcessorEditor,
                                       public juce::Timer
{
public:
    SquabDanceAudioProcessorEditor (SquabDanceAudioProcessor&);
    ~SquabDanceAudioProcessorEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;
    void timerCallback() override;

private:
    SquabDanceAudioProcessor& audioProcessor;
    
    // Database
    std::vector<CharacterDef> characterDB;
    
    // UI Elements
    juce::ComboBox categoryBox;
    juce::ComboBox animationBox;
    juce::Label catLabel;
    juce::Label animLabel;
    
    // The Floating Window
    std::unique_ptr<SpriteWindow> spriteWindow;
    
    // Helper to load images
    void loadCharacterImage(int index);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SquabDanceAudioProcessorEditor)
};