#include "PluginProcessor.h"
#include "PluginEditor.h"

SquabDanceAudioProcessorEditor::SquabDanceAudioProcessorEditor (SquabDanceAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    // Set window size large enough to see the movement
    setSize (600, 600);
    
    // Load "Fruity Chan.png"
    juce::File imageFile = juce::File::getSpecialLocation(juce::File::currentExecutableFile)
                       .getParentDirectory().getChildFile("Fruity Chan.png");
                       
    spriteSheet = juce::ImageCache::getFromFile(imageFile);

    // Start with a default speed of 12Hz
    startTimerHz(12);
}

SquabDanceAudioProcessorEditor::~SquabDanceAudioProcessorEditor()
{
    stopTimer();
}

void SquabDanceAudioProcessorEditor::timerCallback()
{
    // 1. Update Frame Counter
    // We cycle 0 -> 1 -> ... -> 7 -> 0
    currentFrameIndex = (currentFrameIndex + 1) % totalCols;

    // 2. Dynamic Speed Update
    // Check the "speed" parameter and update the timer rate if it changed
    float speedHz = *audioProcessor.apvts.getRawParameterValue("speed");
    
    // Safety check to prevent 0 or negative speed
    if (speedHz < 1.0f) speedHz = 1.0f;
    
    // Only update if significantly different (to avoid jitter)
    if (std::abs(getTimerInterval() - (1000.0 / speedHz)) > 1.0)
    {
        startTimerHz((int)speedHz);
    }

    repaint();
}

void SquabDanceAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colours::black); // Background

    if (spriteSheet.isValid())
    {
        // --- READ PARAMETERS ---
        float xOff = *audioProcessor.apvts.getRawParameterValue("xoffset");
        float yOff = *audioProcessor.apvts.getRawParameterValue("yoffset");
        int styleRow = (int)*audioProcessor.apvts.getRawParameterValue("style");
        bool mirror = *audioProcessor.apvts.getRawParameterValue("mirror") > 0.5f;

        // Safety: Ensure styleRow is between 0 and 9
        styleRow = juce::jlimit(0, totalRows - 1, styleRow);

        // --- CALCULATE SPRITE POSITION ---
        // Which column? (Animation Frame)
        int srcX = currentFrameIndex * frameWidth;
        
        // Which row? (Animation Style)
        int srcY = styleRow * frameHeight;

        // Where to draw on screen?
        int destX = (getWidth() - frameWidth) / 2 + (int)xOff;
        int destY = (getHeight() - frameHeight) / 2 + (int)yOff;

        // --- DRAW MAIN SPRITE ---
        g.drawImage(spriteSheet, 
                    destX, destY, frameWidth, frameHeight,   // Destination Rectangle
                    srcX, srcY, frameWidth, frameHeight);    // Source Rectangle (The Slice)

        // --- DRAW REFLECTION ---
        if (mirror)
        {
            int mirrorY = destY + frameHeight;
            
            g.saveState();
            
            // 1. Flip Vertically
            // The flip axis is the middle of the reflection area
            g.addTransform(juce::AffineTransform::verticalFlip((float)(mirrorY + frameHeight/2)));

            // 2. Apply Fade Gradient (Mask)
            juce::ColourGradient gradient (juce::Colours::white.withAlpha(0.6f), 0, (float)mirrorY,
                                           juce::Colours::transparentWhite, 0, (float)mirrorY + frameHeight, false);
            g.setGradientFill(gradient);

            // 3. Draw the SAME sprite frame again
            g.drawImage(spriteSheet, 
                        destX, mirrorY, frameWidth, frameHeight, 
                        srcX, srcY, frameWidth, frameHeight);

            g.restoreState();
        }
    }
    else 
    {
        g.setColour(juce::Colours::white);
        g.drawText("Fruity Chan.png not found!", getLocalBounds(), juce::Justification::centred, true);
    }
}

void SquabDanceAudioProcessorEditor::resized()
{
    // This is where we will eventually put the Dropdown Menus and Knobs
}