#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "SpriteData.h"
#include "SpriteWindow.h"

// --- Custom Sleek Slider Look ---
class CustomRotarySlider : public juce::LookAndFeel_V4
{
public:
    void drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
                          float sliderPos, const float rotaryStartAngle, const float rotaryEndAngle,
                          juce::Slider& slider) override
    {
        // Increased padding from 4.0f to 8.0f for more breathing room
        auto radius = (float)juce::jmin(width / 2, height / 2) - 8.0f;
        
        auto centreX = (float)x + (float)width  * 0.5f;
        // THE FIX: Removed the "- 10.0f" that was shoving the circle into the label!
        auto centreY = (float)y + (float)height * 0.5f; 
        
        auto rx = centreX - radius;
        auto ry = centreY - radius;
        auto rw = radius * 2.0f;
        auto angle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);
        auto trackThickness = 2.5f; 

        // Draw Background Track
        juce::Path backgroundArc;
        backgroundArc.addCentredArc(centreX, centreY, radius, radius, 0.0f, rotaryStartAngle, rotaryEndAngle, true);
        g.setColour(juce::Colour(0xFF2A2A2A)); 
        g.strokePath(backgroundArc, juce::PathStrokeType(trackThickness, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

        // Draw Active Filled Track
        juce::Path filledArc;
        filledArc.addCentredArc(centreX, centreY, radius, radius, 0.0f, rotaryStartAngle, angle, true);
        g.setColour(juce::Colour(0xFF5A9CC5)); 
        g.strokePath(filledArc, juce::PathStrokeType(trackThickness, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

        // Draw Small Pointer Dot
        auto thumbWidth = 5.0f; 
        juce::Point<float> thumbPoint(centreX + radius * std::cos(angle - juce::MathConstants<float>::halfPi),
                                      centreY + radius * std::sin(angle - juce::MathConstants<float>::halfPi));
        g.setColour(juce::Colours::white);
        g.fillEllipse(thumbPoint.x - thumbWidth / 2.0f, thumbPoint.y - thumbWidth / 2.0f, thumbWidth, thumbWidth);
    }
};

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
    CustomRotarySlider customLookAndFeel;

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
    
    juce::Slider syncSlider;
    
    juce::Slider scaleSlider;
    juce::Label scaleLabel;

    juce::TextButton mirrorButton;
    juce::TextButton hzButton;
    juce::TextButton syncButton;
    juce::TextButton openButton;
    juce::TextButton resetButton;

    // --- AUDIO REACTIVITY UI ---
    juce::Label reactTitleLabel;
    juce::TextButton reactButton;
    
    juce::Slider intensitySlider;
    juce::Label intensityLabel;
    juce::Slider colorSlider;
    juce::Label colorLabel;
    juce::Slider pumpSlider;
    juce::Label pumpLabel;

    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> reactAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> intensityAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> colorAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> pumpAttachment;

    // Attachments
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> categoryAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> animationAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> speedAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> mirrorAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> syncAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> scaleAttachment;

    // --- AUDIO MANIPULATION UI ---
    juce::Label manipTitleLabel;
    juce::TextButton manipButton;
    
    juce::Slider dynamicSlider;
    juce::Label dynamicLabel;
    juce::Slider hueSlider;
    juce::Label hueLabel;
    juce::Slider panningSlider;
    juce::Label panningLabel;

    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> manipAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> dynamicAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> hueAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> panningAttachment;

    

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