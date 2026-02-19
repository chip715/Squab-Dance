#pragma once
#include <JuceHeader.h>

class SpriteContent : public juce::Component, public juce::Timer
{
public:
    SpriteContent() { 
        setSize(220, 512); // DOUBLED HEIGHT for reflection
        startTimerHz(30); 
    }
    ~SpriteContent() override { stopTimer(); }

    void setSpeed(float hz) {
        int newHz = (int)juce::jmax(1.0f, hz); 
        if (currentHz != newHz) {              
            currentHz = newHz;
            startTimerHz(currentHz);
        }
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
        g.fillAll(juce::Colours::transparentBlack);
        if (!spriteSheet.isValid()) return;

        int rowToDraw = isMouseOverOrDragging ? 9 : currentRow;
        int framesToDraw = isMouseOverOrDragging ? 8 : totalFrames; 
        if (framesToDraw <= 0) framesToDraw = 1; 

        int srcX = (currentFrame % framesToDraw) * 220;
        int srcY = rowToDraw * 256;
        int yNudge = -20; 

        // 1. DRAW MAIN CHARACTER
        g.setOpacity(1.0f); 
        g.drawImage(spriteSheet, 0, yNudge, 220, 256, srcX, srcY, 220, 256); 
                    
        // 2. DRAW SURFACE REFLECTION (Flipped Upside Down!)
        if (mirror) {
            g.saveState();
            g.setOpacity(0.35f); 
            
            // Math to perfectly flip the Y-axis at the "floor" line
            float flipOffset = (yNudge * 2.0f) + 512.0f; 
            g.addTransform(juce::AffineTransform::scale(1.0f, -1.0f).translated(0.0f, flipOffset));
            
            g.drawImage(spriteSheet, 0, yNudge, 220, 256, srcX, srcY, 220, 256);
            g.restoreState();
        }
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
    int currentHz = 30; 
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
        centreWithSize(220, 512); // DOUBLED HEIGHT
        setVisible(true);
    }
    
    void closeButtonPressed() override { setVisible(false); }
    SpriteContent* getContent() { return content.get(); }

private:
    std::unique_ptr<SpriteContent> content;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SpriteWindow)
};