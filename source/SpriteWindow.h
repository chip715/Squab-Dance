#pragma once
#include <JuceHeader.h>

class SpriteContent : public juce::Component, public juce::Timer
{
public:
    SpriteContent() { 
        setSize(220, 512); 
        frameBuffer = juce::Image(juce::Image::ARGB, 220, 256, true);
        reflectionBuffer = juce::Image(juce::Image::ARGB, 220, 256, true);
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
        // Clear the main window
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
                    
        // 2. PERFECT FLIP & FADE (Zero Lag, Fixed Ghosting)
        if (mirror) {
            
            // A. HARD ERASE THE MEMORY BUFFERS
            // This instantly fixes the glitching/duplication!
            frameBuffer.clear(frameBuffer.getBounds(), juce::Colours::transparentBlack);
            reflectionBuffer.clear(reflectionBuffer.getBounds(), juce::Colours::transparentBlack);

            // B. Snapshot current frame
            {
                juce::Graphics gFrame(frameBuffer);
                gFrame.drawImage(spriteSheet, 0, 0, 220, 256, srcX, srcY, 220, 256);
            } 

            // C. Manually read upside down and apply the fade
            {
                juce::Image::BitmapData srcData(frameBuffer, juce::Image::BitmapData::readOnly);
                juce::Image::BitmapData destData(reflectionBuffer, juce::Image::BitmapData::writeOnly);

                int fadeEnd = 100; 

                for (int y = 0; y < 256; ++y) {
                    if (y >= fadeEnd) break; 

                    int flippedY = 255 - y; 

                    float progress = (float)y / (float)fadeEnd;
                    float curve = (1.0f - progress) * (1.0f - progress); 
                    float alphaMod = 0.45f * curve; 

                    for (int x = 0; x < 220; ++x) {
                        juce::Colour c = srcData.getPixelColour(x, flippedY);
                        if (c.getAlpha() > 0) {
                            destData.setPixelColour(x, y, c.withMultipliedAlpha(alphaMod));
                        }
                    }
                }
            }

            // D. STAMP IT ON SCREEN
            g.drawImageAt(reflectionBuffer, 0, yNudge + 256);
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
    juce::Image frameBuffer;
    juce::Image reflectionBuffer; 
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
        centreWithSize(220, 512); 
        setVisible(true);
    }
    
    void closeButtonPressed() override { setVisible(false); }
    SpriteContent* getContent() { return content.get(); }

private:
    std::unique_ptr<SpriteContent> content;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SpriteWindow)
};