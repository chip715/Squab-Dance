#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"

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
    juce::Image spriteSheet;
    
    // Animation State
    int currentFrameIndex = 0;
    
    // Sprite Sheet Constants
    const int frameWidth = 220;
    const int frameHeight = 256;
    const int totalCols = 8;
    const int totalRows = 10;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SquabDanceAudioProcessorEditor)
  
};