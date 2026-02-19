#pragma once
#include <JuceHeader.h>

class SpriteContent : public juce::Component, public juce::Timer
{
public:
    SpriteContent() { 
        setSize(220, 256); 
        startTimerHz(30); 
    }
    ~SpriteContent() override { stopTimer(); }

    void setSpeed(float hz) {
        if (hz < 1.0f) hz = 1.0f;
        if (std::abs(getTimerInterval() - (1000.0/hz)) > 1.0) startTimerHz((int)hz);
    }

    void setImage(juce::Image img) { 
        spriteSheet = img; 
        repaint(); 
    }
    
    void updateParams(int row, int frames, bool mir) {
        currentRow = row;
        totalFrames = frames;
        mirror = mir;
        repaint();
    }

    void timerCallback() override {
        if (totalFrames > 0) {
            currentFrame = (currentFrame + 1) % totalFrames;
            repaint();
        }
    }
void paint(juce::Graphics& g) override {
        // 1. WIPE THE CANVAS CLEAN
        g.fillAll(juce::Colours::transparentBlack);

        if (!spriteSheet.isValid()) return;

        // 2. ANIMATION MATH
        int rowToDraw = isMouseOverOrDragging ? 9 : currentRow;
        int framesToDraw = isMouseOverOrDragging ? 8 : totalFrames; 
        if (framesToDraw <= 0) framesToDraw = 1; 

        int srcX = (currentFrame % framesToDraw) * 220;
        int srcY = rowToDraw * 256;

        // --- THE Y-NUDGE ---
        // Pulls the sprite up to bypass the invisible OS window border.
        // If her feet are still slightly cut off, change this to -25 or -30.
        int yNudge = -21; 

        g.saveState();
        
        // 3. MIRROR REFLECTION
        if (mirror) {
            juce::AffineTransform flip = juce::AffineTransform::scale(-1.0f, 1.0f).translated(220.0f, 0.0f);
            g.addTransform(flip);
        }

        // 4. DRAW FULL OPACITY
        g.setOpacity(1.0f); 
        g.drawImage(spriteSheet, 
                    0, yNudge, 220, 256,   // Destination (Screen)
                    srcX, srcY, 220, 256); // Source (Spritesheet)
                    
        g.restoreState();
    }
    void mouseDown (const juce::MouseEvent& e) override {
        isMouseOverOrDragging = true;
        if (auto* win = findParentComponentOfClass<juce::DocumentWindow>())
            dragger.startDraggingComponent (win, e);
        repaint();
    }
    void mouseDrag (const juce::MouseEvent& e) override {
        if (auto* win = findParentComponentOfClass<juce::DocumentWindow>())
            dragger.dragComponent (win, e, nullptr);
    }
    void mouseUp (const juce::MouseEvent& e) override {
        isMouseOverOrDragging = false;
        repaint();
    }

private:
    juce::Image spriteSheet;
    juce::ComponentDragger dragger;
    bool isMouseOverOrDragging = false;
    int currentFrame = 0;
    int totalFrames = 8; 
    int currentRow = 0;
    bool mirror = false;
};

class SpriteWindow : public juce::DocumentWindow
{
public:
    SpriteWindow(const juce::String& name)
        : DocumentWindow(name, juce::Colours::transparentBlack, 0) 
    {
        setUsingNativeTitleBar(false); 
        setResizable(false, false);
        setAlwaysOnTop(true);
        setOpaque(false);
        setBackgroundColour(juce::Colours::transparentBlack);
        setDropShadowEnabled(false); 

        content = std::make_unique<SpriteContent>();
        
        setContentNonOwned(content.get(), true);
        centreWithSize(220, 256);
        setVisible(true);
    }
    
    void closeButtonPressed() override { setVisible(false); }
    
    SpriteContent* getContent() { return content.get(); }

private:
    std::unique_ptr<SpriteContent> content;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SpriteWindow)
};