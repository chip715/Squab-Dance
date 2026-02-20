#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "SpriteData.h"
#include "SpriteWindow.h"

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

    // UI Components
    juce::Label titleLabel;
    juce::ImageComponent birdLogo;
    juce::Label squabDadLabel;

    juce::ComboBox categoryBox;
    juce::Label catLabel;
    juce::ComboBox animationBox;
    juce::Label animLabel;

    juce::Slider speedSlider;
    juce::Label speedLabel;
    
    juce::TextButton mirrorButton;
    juce::TextButton hzButton;
    juce::TextButton syncButton;
    juce::TextButton openButton;
    juce::TextButton resetButton;

    // Attachments
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> categoryAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> animationAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> speedAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> mirrorAttachment;

    std::vector<CharacterDef> characterDB;
    std::unique_ptr<SpriteWindow> spriteWindow;
    
    void preCacheImages();
    void loadCharacterImage(int index);
    bool isSyncMode = false; 
    void triggerBackgroundLoad(int index);
    std::vector<juce::Image> cachedSprites;

    struct ResourcePointer {
    const char* data = nullptr;
    int size = 0;
};
std::vector<ResourcePointer> resourceCache;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SquabDanceAudioProcessorEditor)
};