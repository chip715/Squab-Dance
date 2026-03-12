#pragma once
#include <JuceHeader.h>
#include "SpriteData.h"

class SpriteContent : public juce::Component, public juce::Timer
{
public:
    SpriteContent() { 
        // 1. MASSIVE FIXED CANVAS: 960x2280 supports exactly up to Scale 300%
        setSize(960, 2280); 
        frameBuffer = juce::Image(juce::Image::ARGB, 220, 256, true);
        reflectionBuffer = juce::Image(juce::Image::ARGB, 220, 256, true);
        lastFrameBuffer = juce::Image(juce::Image::ARGB, 220, 256, true); 
        
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
    
    void setSpriteData(bool isGrid, const std::vector<AnimationDef>& anims, const std::vector<juce::Image>& imgs) {
        isGridSprite = isGrid;
        currentAnims = anims;
        spriteSheets = imgs;
        juce::MessageManager::callAsync([this]() { repaint(); });
    }
    
    void updateParams(int row, int frames, int hRow, int hFrames, bool mir) {
        currentRow = row; totalFrames = frames; heldRow = hRow; heldFrames = hFrames; mirror = mir;
        repaint();
    }

    void resetAnimation() { currentFrame = 0; repaint(); }

    void timerCallback() override {
        // --- THE BOUNCE PHYSICS ---
        if (audioLevel > smoothPump) smoothPump = audioLevel; 
        else smoothPump *= 0.85f; // Decay speed

        int activeFrames = isMouseOverOrDragging ? heldFrames : totalFrames;
        if (activeFrames <= 0) return;

        if (isSyncMode) {
            if (isPlaying && currentBeatLength > 0.0) {
                int newFrame = static_cast<int>(currentPpq / currentBeatLength) % activeFrames;
                if (newFrame < 0) newFrame += activeFrames; 
                if (currentFrame != newFrame) {
                    currentFrame = newFrame;
                    repaint();
                }
            }
        } else {
            currentFrame = (currentFrame + 1) % activeFrames;
            repaint();
        }
    }

    int getGlobalFrameOffset(int targetRow) {
        int offset = 0;
        for (int i = 0; i < targetRow && i < (int)currentAnims.size(); ++i) offset += currentAnims[i].frameCount;
        return offset;
    }

    void paint(juce::Graphics& g) override {
        g.fillAll(juce::Colours::transparentBlack);
        if (spriteSheets.empty()) return;

        // 2. THE NEW ANCHOR MATH
        float winCenterX = getWidth() / 2.0f;   // 480
        float winCenterY = getHeight() / 2.0f;  // 1140 (This acts as the "Floor")

        // apply scale
        g.addTransform(juce::AffineTransform::scale(currentScale, currentScale, winCenterX, winCenterY));

        // Offset the original 320x760 coordinates into the new massive canvas
        float offsetX = winCenterX - 160.0f; 
        float offsetY = winCenterY - 380.0f;

        int activeRow = isMouseOverOrDragging ? heldRow : currentRow;
        int activeFrames = isMouseOverOrDragging ? heldFrames : totalFrames; 
        if (activeFrames <= 0) activeFrames = 1; 

        int sx = 0, sy = 0, sheetIndex = 0;

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

        // --- 1. THE PHYSICS PUMP ---
        float pScale = 1.0f;
        if (audioReactOn && smoothPump > 0.001f && reactPump > 0.0f) {
            pScale += (smoothPump * (reactPump / 100.0f) * 0.45f); 
        }

        float destW = 220.0f * pScale;
        float destH = 256.0f * pScale;
        
        // Draw exactly in the center of the massive canvas
        float destX = offsetX + 160.0f - (destW * 0.5f);
        float destY = offsetY + 380.0f - destH;

        if (sheetIndex < (int)spriteSheets.size() && spriteSheets[sheetIndex].isValid()) {
            auto& currentImg = spriteSheets[sheetIndex];
            juce::Image sourceToDraw = currentImg; 
            int drawSourceX = sx;
            int drawSourceY = sy;

            // --- 2. FAST PIXEL HUE/SATURATION ENGINE ---
            if (audioReactOn && audioLevel > 0.001f && (reactColor > 0.0f || reactIntensity > 0.0f)) {
                frameBuffer.clear(frameBuffer.getBounds(), juce::Colours::transparentBlack);
                {
                    juce::Graphics gFrame(frameBuffer);
                    gFrame.drawImage(currentImg, 0, 0, 220, 256, sx, sy, 220, 256);
                }

                juce::Image::BitmapData data(frameBuffer, juce::Image::BitmapData::readWrite);
                
                float hueShift = audioLevel * (reactColor / 100.0f);
                float intensityFactor = audioLevel * (reactIntensity / 100.0f);
                float satBoost = intensityFactor * 2.0f; 
                float brightBoost = intensityFactor * 0.6f; 
            
                for (int y = 0; y < 256; ++y) {
                    for (int x = 0; x < 220; ++x) {
                        juce::Colour c = data.getPixelColour(x, y);
                        
                        // THE CPU SAVER: Only run the heavy math if the pixel is actually visible!
                        if (c.getAlpha() > 10) {
                            data.setPixelColour(x, y, juce::Colour::fromHSV(
                                std::fmod(c.getHue() + hueShift, 1.0f),
                                juce::jmin(1.0f, c.getSaturation() * (1.0f + satBoost)), 
                                juce::jmin(1.0f, c.getBrightness() + brightBoost),       
                                c.getFloatAlpha()
                            ));
                        }
                    }
                }
                sourceToDraw = frameBuffer; 
                drawSourceX = 0; drawSourceY = 0;
            }

            // --- TRUE PIXEL-DELTA SENSOR ENGINE ---
            float totalX = 0, totalHue = 0, pixelCount = 0, deltaCount = 0;
            juce::Image::BitmapData scanData(sourceToDraw, juce::Image::BitmapData::readOnly);
            juce::Image::BitmapData oldData(lastFrameBuffer, juce::Image::BitmapData::readOnly);

            for (int y = 0; y < 256; y += 4) {
                for (int x = 0; x < 220; x += 4) {
                    juce::Colour c = scanData.getPixelColour(sx + x, sy + y);
                    juce::Colour oldC = oldData.getPixelColour(x, y);

                    if (c.getAlpha() > 50) {
                        totalX += x;
                        totalHue += c.getHue();
                        pixelCount++;
                    }
                    if (std::abs(c.getBrightness() - oldC.getBrightness()) > 0.05f) deltaCount++;
                }
            }

           if (pixelCount > 0) {
                currentPan = (totalX / pixelCount) / 220.0f; 
                currentHue = totalHue / pixelCount;
                float frameMotion = (deltaCount / pixelCount) * 15.0f;
                currentMotion = juce::jmax(currentMotion, juce::jmin(1.0f, frameMotion));
            }
            
            currentMotion *= 0.70f; // Fast decay

            // Save the current frame for motion sensing
            {
                lastFrameBuffer.clear(lastFrameBuffer.getBounds(), juce::Colours::transparentBlack);
                juce::Graphics gLast(lastFrameBuffer);
                gLast.drawImage(currentImg, 0, 0, 220, 256, sx, sy, 220, 256);
            }

            g.setOpacity(1.0f); 
            g.drawImage(sourceToDraw, destX, destY, destW, destH, drawSourceX, drawSourceY, 220, 256); 
            
            // --- REFLECTION ---
            if (mirror) {
                if (sourceToDraw != frameBuffer) {
                    frameBuffer.clear(frameBuffer.getBounds(), juce::Colours::transparentBlack);
                    {
                        juce::Graphics gFrame(frameBuffer);
                        gFrame.drawImage(sourceToDraw, 0, 0, 220, 256, drawSourceX, drawSourceY, 220, 256);
                    }
                }

                reflectionBuffer.clear(reflectionBuffer.getBounds(), juce::Colours::transparentBlack);
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
                            if (c.getAlpha() > 0) destData.setPixelColour(x, y, c.withMultipliedAlpha(alphaMod));
                        }
                    }
                }
                g.drawImage(reflectionBuffer, destX, offsetY + 380.0f, destW, destH, 0, 0, 220, 256);
            }
        }
    }
    
    // 3. PERFECT CLICK-THROUGH HIT TESTING
    bool hitTest(int x, int y) override {
        float dx = (float)x - (getWidth() / 2.0f);
        float dy = (float)y - (getHeight() / 2.0f); 
        
        float hitWidth = 140.0f * currentScale;
        float hitHeightUp = 260.0f * currentScale;
        float hitHeightDown = 160.0f * currentScale;

        if (std::abs(dx) > hitWidth) return false;
        if (dy < -hitHeightUp || dy > hitHeightDown) return false;
        
        return true;
    }

    void mouseDown (const juce::MouseEvent& e) override {
        isMouseOverOrDragging = true;
        if (auto* win = findParentComponentOfClass<juce::DocumentWindow>()) dragger.startDraggingComponent (win, e);
        repaint();
    }
    void mouseDrag (const juce::MouseEvent& e) override {
        if (auto* win = findParentComponentOfClass<juce::DocumentWindow>()) dragger.dragComponent (win, e, nullptr);
    }
    void mouseUp (const juce::MouseEvent& e) override {
        isMouseOverOrDragging = false;
        repaint();
    }

    void updateSync(bool synced, double beatLength, double ppq, bool playing) {
        if (isSyncMode != synced) {
            isSyncMode = synced;
            if (isSyncMode) startTimerHz(60); 
            else startTimerHz(currentHz);
        }
        currentBeatLength = beatLength;
        currentPpq = ppq;
        isPlaying = playing;
    }

    // 4. PURE MATH SCALING (No OS Calls!)
    void setScale(float newScale) {
        newScale = juce::jmax(0.1f, newScale); 
        if (currentScale != newScale) {
            currentScale = newScale;
            repaint();
        }
    }

    void updateAudioReact(bool on, float intensity, float color, float pump, float floatLevel) {
        audioReactOn = on; reactIntensity = intensity; reactColor = color; reactPump = pump; audioLevel = floatLevel;
    }

    float getMotion() const { return currentMotion; }
    float getHue() const { return currentHue; }
    float getPan() const { return currentPan; }

private:
    std::vector<juce::Image> spriteSheets;
    std::vector<AnimationDef> currentAnims;
    bool isGridSprite = false;

    juce::Image frameBuffer;
    juce::Image reflectionBuffer; 
    juce::Image lastFrameBuffer; 

    juce::ComponentDragger dragger;

    bool isSyncMode = false;
    double currentBeatLength = 1.0;
    double currentPpq = 0.0;
    bool isPlaying = false;
    float currentScale = 1.0f;
    
    int currentHz = 30; 
    bool isMouseOverOrDragging = false;
    int currentFrame = 0, totalFrames = 8, currentRow = 0, heldRow = 9, heldFrames = 8;
    bool mirror = false;
    bool audioReactOn = false;
    float reactIntensity = 0.0f;
    float reactColor = 0.0f;
    float reactPump = 0.0f;
    float audioLevel = 0.0f;

    float currentMotion = 0.0f;
    float currentHue = 0.0f;
    float currentPan = 0.5f;
    int lastAnalyzedFrame = -1;

    float smoothPump = 0.0f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SpriteContent)
};

class SpriteWindow : public juce::DocumentWindow
{
public:
    SpriteWindow(const juce::String& name)
        : DocumentWindow(name, juce::Colours::transparentBlack, 0) 
    {
        setUsingNativeTitleBar(false); setTitleBarHeight(0); setResizable(false, false);
        setAlwaysOnTop(true); setOpaque(false); setBackgroundColour(juce::Colours::transparentBlack);
        setDropShadowEnabled(false); 

        // CRITICAL: Disable JUCE bounds constrainer so the giant invisible window can overflow off the screen edges!
        if (auto* constrainer = getConstrainer()) {
            constrainer->setMinimumOnscreenAmounts(0, 0, 0, 0);
        }

        content = std::make_unique<SpriteContent>();
        setContentNonOwned(content.get(), true);
        
        // Spawn the massive invisible canvas perfectly centered on the user's primary monitor
        centreWithSize(960, 2280); 
        setVisible(true);
    }

    void closeButtonPressed() override { setVisible(false); }
    SpriteContent* getContent() { return content.get(); }

private:
    std::unique_ptr<SpriteContent> content;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SpriteWindow)
};