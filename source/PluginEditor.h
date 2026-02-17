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
    std::vector<CharacterDef> characterDB;
    
    juce::ComboBox categoryBox, animationBox;
    juce::Label catLabel, animLabel;

    juce::Slider speedSlider;
    juce::ToggleButton mirrorButton;
    juce::Label speedLabel;

    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    using ButtonAttachment = juce::AudioProcessorValueTreeState::ButtonAttachment;

    std::unique_ptr<SliderAttachment> speedAttachment;
    std::unique_ptr<ButtonAttachment> mirrorAttachment;
    
    std::unique_ptr<SpriteWindow> spriteWindow;
    
    void loadCharacterImage(int index);
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SquabDanceAudioProcessorEditor)
};