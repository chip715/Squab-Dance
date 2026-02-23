#pragma once
#include <JuceHeader.h>
#include "SpriteData.h"

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
    
    // --- REPLACED SINGLE IMAGE WITH DATA SETTER ---
    void setSpriteData(bool isGrid, const std::vector<AnimationDef>& anims, const std::vector<juce::Image>& imgs) {
        isGridSprite = isGrid;
        currentAnims = anims;
        spriteSheets = imgs;
        
        // Use this to ensure the UI updates immediately
        juce::MessageManager::callAsync([this]() { repaint(); });
    }
    
    void updateParams(int row, int frames, int hRow, int hFrames, bool mir) {
        currentRow = row;
        totalFrames = frames;
        heldRow = hRow;       
        heldFrames = hFrames; 
        mirror = mir;
        repaint();
    }

    void resetAnimation() {
        currentFrame = 0;            
        repaint();
    }

    void timerCallback() override {
        int activeFrames = isMouseOverOrDragging ? heldFrames : totalFrames;
        if (activeFrames > 0) {
            currentFrame = (currentFrame + 1) % activeFrames;
            repaint();
        }
    }

    int getGlobalFrameOffset(int targetRow) {
        int offset = 0;
        for (int i = 0; i < targetRow && i < (int)currentAnims.size(); ++i) {
            offset += currentAnims[i].frameCount;
        }
        return offset;
    }

    void paint(juce::Graphics& g) override {
        g.fillAll(juce::Colours::transparentBlack);
        
        if (spriteSheets.empty()) return;

        int activeRow = isMouseOverOrDragging ? heldRow : currentRow;
        int activeFrames = isMouseOverOrDragging ? heldFrames : totalFrames; 
        if (activeFrames <= 0) activeFrames = 1; 

        int sx = 0, sy = 0, sheetIndex = 0;

        // --- COORDINATE MAPPING LOGIC ---
        if (isGridSprite) {
            int startOffset = getGlobalFrameOffset(activeRow);
            int globalFrame = startOffset + (currentFrame % activeFrames);
            
            sheetIndex = globalFrame / 72;
            int localFrameIndex = globalFrame % 72;
            
            sx = (localFrameIndex % 9) * 220;
            sy = (localFrameIndex / 9) * 256;
        } else {
            sx = (currentFrame % activeFrames) * 220;
            sy = activeRow * 256;
            sheetIndex = 0;
        }

        // --- RENDERING ---
        if (sheetIndex < (int)spriteSheets.size() && spriteSheets[sheetIndex].isValid()) {
            int yNudge = -20; 
            auto& currentImg = spriteSheets[sheetIndex];

            g.setOpacity(1.0f); 
            g.drawImage(currentImg, 0, yNudge, 220, 256, sx, sy, 220, 256);
            
            if (mirror) {
                frameBuffer.clear(frameBuffer.getBounds(), juce::Colours::transparentBlack);
                reflectionBuffer.clear(reflectionBuffer.getBounds(), juce::Colours::transparentBlack);

                {
                    juce::Graphics gFrame(frameBuffer);
                    gFrame.drawImage(currentImg, 0, 0, 220, 256, sx, sy, 220, 256);
                } 

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
                g.drawImageAt(reflectionBuffer, 0, yNudge + 256);
            }
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
    std::vector<juce::Image> spriteSheets;
    std::vector<AnimationDef> currentAnims;
    bool isGridSprite = false;

    juce::Image frameBuffer;
    juce::Image reflectionBuffer; 
    juce::ComponentDragger dragger;
    
    int currentHz = 30; 
    bool isMouseOverOrDragging = false;
    int currentFrame = 0, totalFrames = 8, currentRow = 0, heldRow = 9, heldFrames = 8;
    bool mirror = false;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SpriteContent)
};

class SpriteWindow : public juce::DocumentWindow
{
public:
    SpriteWindow(const juce::String& name)
        : DocumentWindow(name, juce::Colours::transparentBlack, 0) 
    {
        setUsingNativeTitleBar(false); 
        setTitleBarHeight(0);
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