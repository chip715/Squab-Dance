#include "PluginProcessor.h"
#include "PluginEditor.h"

SquabDanceAudioProcessorEditor::SquabDanceAudioProcessorEditor (SquabDanceAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    setSize (400, 200); // Main window controls only
    
    // 1. Load Database
    characterDB = SpriteDatabase::getDatabase();

    // 2. Setup Category Box
    addAndMakeVisible(categoryBox);
    int id = 1;
    for (auto& charDef : characterDB)
    {
        categoryBox.addItem(charDef.categoryName, id++);
    }
    
    // When Category changes...
    categoryBox.onChange = [this] {
        int catIndex = categoryBox.getSelectedId() - 1;
        if (catIndex >= 0 && catIndex < characterDB.size())
        {
            // Populate Animation Box
            animationBox.clear();
            int animId = 1;
            for (auto& anim : characterDB[catIndex].anims)
            {
                animationBox.addItem(anim.name, animId++);
            }
            animationBox.setSelectedId(1); // Select first animation by default
            
            // Load the PNG for this character
            loadCharacterImage(catIndex);
        }
    };

    // 3. Setup Animation Box
    addAndMakeVisible(animationBox);
    animationBox.onChange = [this] {
        // We just need to trigger an update, the timer handles sending data
    };

    // 4. Create Labels
    addAndMakeVisible(catLabel); catLabel.setText("Category", juce::dontSendNotification);
    addAndMakeVisible(animLabel); animLabel.setText("Animation", juce::dontSendNotification);

    // 5. Create Floating Window
    spriteWindow = std::make_unique<SpriteWindow>("Squab Dance Visuals");

    // Initialize with first item
    categoryBox.setSelectedId(11); // Default to Fruity Chan (Index 11 in list)
    
    startTimerHz(30); // UI Update loop
}

SquabDanceAudioProcessorEditor::~SquabDanceAudioProcessorEditor()
{
    stopTimer();
    spriteWindow = nullptr; // Close window when plugin closes
}

void SquabDanceAudioProcessorEditor::loadCharacterImage(int index)
{
    if (spriteWindow == nullptr) return;

    juce::String filename = characterDB[index].filename;
    juce::File imageFile = juce::File::getSpecialLocation(juce::File::currentExecutableFile)
                           .getParentDirectory().getChildFile(filename);

    juce::Image newImg = juce::ImageCache::getFromFile(imageFile);
    
    if (newImg.isNull()) {
        DBG("Failed to load: " + filename); 
    }
    
    spriteWindow->getContent()->setImage(newImg);
}

void SquabDanceAudioProcessorEditor::timerCallback()
{
    // Continuously update the Floating Window with parameter values
    if (spriteWindow != nullptr)
    {
        // Get raw values
        float speed = *audioProcessor.apvts.getRawParameterValue("speed");
        float xOff = *audioProcessor.apvts.getRawParameterValue("xoffset");
        float yOff = *audioProcessor.apvts.getRawParameterValue("yoffset");
        bool mirror = *audioProcessor.apvts.getRawParameterValue("mirror") > 0.5f;
        
        // Get current animation info
        int catIdx = categoryBox.getSelectedId() - 1;
        int animIdx = animationBox.getSelectedId() - 1;
        
        int frameCount = 8; // Default
        int row = 0;

        if (catIdx >= 0 && catIdx < characterDB.size()) {
            if (animIdx >= 0 && animIdx < characterDB[catIdx].anims.size()) {
                frameCount = characterDB[catIdx].anims[animIdx].frameCount;
                row = animIdx; // The row matches the menu index
            }
        }

        // Send to Window
        spriteWindow->getContent()->setSpeed(speed);
        spriteWindow->getContent()->updateParams(row, frameCount, mirror, xOff, yOff);
    }
}

void SquabDanceAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colours::darkgrey);
}

void SquabDanceAudioProcessorEditor::resized()
{
    // Layout the controls
    catLabel.setBounds(20, 20, 100, 20);
    categoryBox.setBounds(20, 45, 150, 30);
    
    animLabel.setBounds(200, 20, 100, 20);
    animationBox.setBounds(200, 45, 150, 30);
}