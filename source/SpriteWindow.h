#pragma once
#include <JuceHeader.h>

// This component lives INSIDE the transparent window
class SpriteContent : public juce::Component, public juce::Timer
{
public:
    SpriteContent() { startTimerHz(12); }
    ~SpriteContent() { stopTimer(); }

    void setSpeed(float hz) {
        if (hz < 1.0f) hz = 1.0f;
        if (std::abs(getTimerInterval() - (1000.0/hz)) > 1.0) startTimerHz((int)hz);
    }

    void setImage(juce::Image img) { spriteSheet = img; repaint(); }
    
    // Updates parameters from the Editor
    void updateParams(int row, int frames, bool mir, float x, float y) {
        currentRow = row;
        totalFrames = frames;
        mirror = mir;
        xOffset = x;
        yOffset = y;
        repaint();
    }

    void timerCallback() override {
        if (totalFrames > 0) {
            currentFrame = (currentFrame + 1) % totalFrames;
            repaint();
        }
    }

    void paint(juce::Graphics& g) override {
        // No fillAll() -> This keeps it transparent!
        
        if (spriteSheet.isValid()) {
            int frameWidth = 220; 
            int frameHeight = 256;
            
            // Calculate offsets to center in the window
            int destX = (getWidth() - frameWidth)/2 + (int)xOffset;
            int destY = (getHeight() - frameHeight)/2 + (int)yOffset;

            int srcX = currentFrame * frameWidth;
            int srcY = currentRow * frameHeight;

            g.drawImage(spriteSheet, destX, destY, frameWidth, frameHeight, srcX, srcY, frameWidth, frameHeight);

            if (mirror) {
                int mirrorY = destY + frameHeight;
                g.saveState();
                g.addTransform(juce::AffineTransform::verticalFlip((float)(mirrorY + frameHeight/2)));
                
                juce::ColourGradient gradient (juce::Colours::white.withAlpha(0.6f), 0, (float)mirrorY,
                                               juce::Colours::transparentWhite, 0, (float)mirrorY + frameHeight, false);
                g.setGradientFill(gradient);
                g.drawImage(spriteSheet, destX, mirrorY, frameWidth, frameHeight, srcX, srcY, frameWidth, frameHeight);
                g.restoreState();
            }
        }
    }

private:
    juce::Image spriteSheet;
    int currentFrame = 0;
    int totalFrames = 8; 
    int currentRow = 0;
    bool mirror = false;
    float xOffset = 0;
    float yOffset = 0;
};

// The Actual Floating Window Class
class SpriteWindow : public juce::DocumentWindow
{
public:
    SpriteWindow(const juce::String& name)
        : DocumentWindow(name, juce::Colours::transparentBlack, DocumentWindow::allButtons)
    {
        setUsingNativeTitleBar(true); // Standard window frame
        setResizable(true, true);
        setAlwaysOnTop(true);         // Float above everything
        
        content = std::make_unique<SpriteContent>();
        setContentOwned(content.get(), true);
        
        // Make the background transparent
        setBackgroundColour(juce::Colours::transparentBlack);
        setOpaque(false);
        
        centreWithSize(400, 600);
        setVisible(true);
    }
    
    // Pass commands down to the content
    SpriteContent* getContent() { return content.get(); }

    void closeButtonPressed() override { 
        setVisible(false); // Just hide, don't destroy
    }

private:
    std::unique_ptr<SpriteContent> content;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SpriteWindow)
};