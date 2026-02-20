#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <unordered_map>


const int DEFAULT_CHARACTER_INDEX = 11; 

SquabDanceAudioProcessorEditor::SquabDanceAudioProcessorEditor (SquabDanceAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    setSize (420, 240); 
    characterDB = SpriteDatabase::getDatabase();
    preCacheImages();

    // 1. TITLE & BRANDING
    addAndMakeVisible(titleLabel);
    titleLabel.setText("Squab Dance", juce::dontSendNotification);
    titleLabel.setFont(juce::Font(24.0f, juce::Font::plain));
    titleLabel.setColour(juce::Label::textColourId, juce::Colours::white);

    addAndMakeVisible(squabDadLabel);
    squabDadLabel.setText("by Squab Dad", juce::dontSendNotification);
    squabDadLabel.setFont(juce::Font(12.0f, juce::Font::plain));
    squabDadLabel.setColour(juce::Label::textColourId, juce::Colours::black);



// 2. LOAD BIRD LOGO (Updated for Squab_Logo.png)
    addAndMakeVisible(birdLogo);
    birdLogo.setImagePlacement(juce::RectanglePlacement::centred | juce::RectanglePlacement::onlyReduceInSize);
    
    for (int i = 0; i < BinaryData::namedResourceListSize; ++i) {
        juce::String res = BinaryData::namedResourceList[i];
        // Strip all underscores and spaces so "Squab_Logo.png" becomes "squablogopng"
        juce::String cleanRes = res.replace(" ", "").replace("_", "").toLowerCase();
        
        if (cleanRes.contains("squablogo")) {
            int size = 0;
            const char* data = BinaryData::getNamedResource(res.toUTF8(), size);
            birdLogo.setImage(juce::ImageCache::getFromMemory(data, size));
            break;
        }
    }

   // 3. DROPDOWNS
    addAndMakeVisible(categoryBox);
    int id = 1;
    for (auto& charDef : characterDB) categoryBox.addItem(charDef.categoryName, id++);

    // SMART ATTACH: Locks "Category" right above the box
    catLabel.setText("Category", juce::dontSendNotification);
    catLabel.setColour(juce::Label::textColourId, juce::Colour(0xFFAAAAAA));
    catLabel.attachToComponent(&categoryBox, false); 

    categoryBox.onChange = [this] {
        int catIndex = categoryBox.getSelectedId() - 1;
        if (catIndex >= 0 && catIndex < (int)characterDB.size()) {
            animationBox.clear();
            int animId = 1;
            for (auto& anim : characterDB[catIndex].anims) 
                animationBox.addItem(anim.name, animId++);
            
            animationBox.setSelectedId(1);
            
            loadCharacterImage(catIndex);
        }
    };

    addAndMakeVisible(animationBox);
    
    // SMART ATTACH: Locks "Dance Move" right above the box
    animLabel.setText("Dance Move", juce::dontSendNotification);
    animLabel.setColour(juce::Label::textColourId, juce::Colour(0xFFAAAAAA));
    animLabel.attachToComponent(&animationBox, false);

    // 4. RATE KNOB
    addAndMakeVisible(speedLabel); 
    speedLabel.setText("Rate", juce::dontSendNotification);
    speedLabel.setColour(juce::Label::textColourId, juce::Colours::lightgrey);
    speedLabel.setJustificationType(juce::Justification::centred);
    
    addAndMakeVisible(speedSlider);
    speedSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    speedSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 20);
    speedSlider.setColour(juce::Slider::textBoxTextColourId, juce::Colours::lightgrey);
    speedSlider.setNumDecimalPlacesToDisplay(1);
    speedAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts, "speed", speedSlider);

    // 5. BUTTONS
    addAndMakeVisible(mirrorButton);
    mirrorButton.setClickingTogglesState(true);
    mirrorButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xFF1E1E1E)); 
    mirrorButton.setColour(juce::TextButton::textColourOffId, juce::Colours::white);
    mirrorButton.setColour(juce::TextButton::buttonOnColourId, juce::Colour(0xFFFBB03B)); 
    mirrorButton.setColour(juce::TextButton::textColourOnId, juce::Colours::black);
    mirrorAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(audioProcessor.apvts, "mirror", mirrorButton);

    addAndMakeVisible(hzButton); hzButton.setButtonText("Hz");
    hzButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xFFFBB03B));
    hzButton.setColour(juce::TextButton::textColourOffId, juce::Colours::black);
    
    addAndMakeVisible(syncButton); syncButton.setButtonText(juce::CharPointer_UTF8("\xe2\x99\xaa"));
    syncButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xFF666666)); 
    syncButton.setColour(juce::TextButton::textColourOffId, juce::Colours::black);

    // --- THE OPEN/CLOSED TOGGLE FIX ---

    addAndMakeVisible(openButton); 
    openButton.setClickingTogglesState(true);
    
    // Set the default state to ON (since the window spawns automatically)
    openButton.setToggleState(true, juce::dontSendNotification);
    openButton.setButtonText("Open"); 
    
    openButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xFF222222)); // OFF state (Dark)
    openButton.setColour(juce::TextButton::textColourOffId, juce::Colours::white);
    openButton.setColour(juce::TextButton::buttonOnColourId, juce::Colour(0xFFFBB03B)); // ON state (Yellow)
    openButton.setColour(juce::TextButton::textColourOnId, juce::Colours::black);
    
    // Dynamic Text Swapper AND Window Visibility Link
    openButton.onClick = [this] {
        bool isOpen = openButton.getToggleState();
        openButton.setButtonText(isOpen ? "Open" : "Closed");
        
        // Hide or show the actual floating window!
        if (spriteWindow != nullptr) {
            spriteWindow->setVisible(isOpen);
        }
    };

    addAndMakeVisible(resetButton); resetButton.setButtonText("Reset");
    resetButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xFF222222)); 
    resetButton.setColour(juce::TextButton::textColourOffId, juce::Colours::white);
    
    // --- ADD THE CLICK BEHAVIOR ---
    resetButton.onClick = [this] {
        if (spriteWindow != nullptr) {
            spriteWindow->getContent()->resetAnimation();
        }
    };

    // 6. WINDOW INIT
   spriteWindow = std::make_unique<SpriteWindow>("Squab Visuals");
    
    if (DEFAULT_CHARACTER_INDEX <= characterDB.size()) {
        categoryBox.setSelectedId(DEFAULT_CHARACTER_INDEX); 
    } else {
        categoryBox.setSelectedId(1);
    }
    
    startTimerHz(30); 
}

SquabDanceAudioProcessorEditor::~SquabDanceAudioProcessorEditor() { 
    stopTimer(); 
    spriteWindow = nullptr; 
}

void SquabDanceAudioProcessorEditor::timerCallback() {
    if (spriteWindow == nullptr) return;

    float speed = *audioProcessor.apvts.getRawParameterValue("speed");
    bool mir = *audioProcessor.apvts.getRawParameterValue("mirror") > 0.5f;
    int style = animationBox.getSelectedId() - 1; 
    
    mirrorButton.setButtonText(mir ? "Reflection" : "No Reflection");

    int catIdx = categoryBox.getSelectedId() - 1;
    
    int frames = 8;
    int hRow = 9;    // Default fallback
    int hFrames = 8; // Default fallback

    if (catIdx >= 0 && catIdx < (int)characterDB.size()) {
        auto& anims = characterDB[catIdx].anims;
        
        // 1. Get current normal dance move frames
        if (style >= 0 && style < (int)anims.size()) {
            frames = anims[style].frameCount;
        }

        // 2. SMARTS SEARCH: Find the frame data specifically for this character's "Held" state
        bool foundHeld = false;
        for (int i = 0; i < (int)anims.size(); ++i) {
            juce::String name = anims[i].name.toLowerCase();
            // If the animation is named Held, Drag, or Pick, grab its specific row and frame count
            if (name.contains("held") || name.contains("drag") || name.contains("pick")) {
                hRow = i;
                hFrames = anims[i].frameCount;
                foundHeld = true;
                break; 
            }
        }

        // 3. Fallback: If not explicitly named, but row 9 exists, use row 9's frame count
        if (!foundHeld && anims.size() > 9) {
            hRow = 9;
            hFrames = anims[9].frameCount;
        }
    }

    spriteWindow->getContent()->setSpeed(speed);
    
    // Pass all the data (including the newly discovered Held row & frames) to the window
    spriteWindow->getContent()->updateParams(style, frames, hRow, hFrames, mir); 
}

void SquabDanceAudioProcessorEditor::paint (juce::Graphics& g) { 
    g.fillAll (juce::Colour(0xFF424242)); // Match Mockup Background Grey
}

void SquabDanceAudioProcessorEditor::resized() {
    // Left Column
    titleLabel.setBounds(20, 15, 200, 30);
    birdLogo.setBounds(10, 50, 200, 110);
    mirrorButton.setBounds(20, 180, 180, 25);

    // Right Column
    int rightCol = 230;
    
    categoryBox.setBounds(rightCol, 35, 160, 22);
    animationBox.setBounds(rightCol, 85, 160, 22);

    speedLabel.setBounds(rightCol, 120, 75, 20); 
    speedSlider.setBounds(rightCol, 140, 75, 75); 

    int btnX = rightCol + 85;
    int btnY = 145;
    hzButton.setBounds(btnX, btnY, 35, 20);
    openButton.setBounds(btnX + 40, btnY, 50, 20);
    
    syncButton.setBounds(btnX, btnY + 25, 35, 20);
    resetButton.setBounds(btnX + 40, btnY + 25, 50, 20);

    squabDadLabel.setBounds(btnX, 205, 100, 20);
} 

void SquabDanceAudioProcessorEditor::preCacheImages() {
    resourceCache.clear();

    std::unordered_map<juce::String, int> resourceMap;
    for (int i = 0; i < BinaryData::namedResourceListSize; ++i) {
        juce::String cleanName = juce::String(BinaryData::namedResourceList[i]).replace("_", "").toLowerCase();
        resourceMap[cleanName] = i;
    }

    for (auto& charDef : characterDB) {
        juce::String cleanTarget = charDef.filename.replace(" ", "").replace(".", "").replace("_", "").toLowerCase();

        ResourcePointer ptr;
        if (resourceMap.count(cleanTarget)) {
            int idx = resourceMap[cleanTarget];
            ptr.data = BinaryData::getNamedResource(BinaryData::namedResourceList[idx], ptr.size);
        }
        resourceCache.push_back(ptr); 
    }
}

void SquabDanceAudioProcessorEditor::loadCharacterImage(int index) {
    if (spriteWindow != nullptr && index >= 0 && index < (int)resourceCache.size()) {
        auto& res = resourceCache[index];
        
        if (res.data != nullptr) {
            // Only decompress the ONE image we actually need right now
            auto img = juce::ImageCache::getFromMemory(res.data, res.size);
            spriteWindow->getContent()->setImage(img);
        }
    }
}