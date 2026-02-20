#include "PluginProcessor.h"
#include "PluginEditor.h"

const int DEFAULT_CHARACTER_INDEX = 11; 

SquabDanceAudioProcessorEditor::SquabDanceAudioProcessorEditor (SquabDanceAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    setSize (420, 240); 
    characterDB = SpriteDatabase::getDatabase();

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
    addAndMakeVisible(catLabel); catLabel.setText("Category", juce::dontSendNotification);
    catLabel.setColour(juce::Label::textColourId, juce::Colours::darkgrey.darker());
    addAndMakeVisible(categoryBox);
    int id = 1;
    for (auto& charDef : characterDB) categoryBox.addItem(charDef.categoryName, id++);

    categoryBox.onChange = [this] {
        int catIndex = categoryBox.getSelectedId() - 1;
        if (catIndex >= 0 && catIndex < (int)characterDB.size()) {
            animationBox.clear();
            int animId = 1;
            for (auto& anim : characterDB[catIndex].anims) animationBox.addItem(anim.name, animId++);
            animationBox.setSelectedId(1);
            loadCharacterImage(catIndex);
        }
    };

    addAndMakeVisible(animLabel); animLabel.setText("Dance Move", juce::dontSendNotification);
    animLabel.setColour(juce::Label::textColourId, juce::Colours::darkgrey.darker());
    addAndMakeVisible(animationBox);

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

void SquabDanceAudioProcessorEditor::loadCharacterImage(int index) {
    if (spriteWindow == nullptr) return;
    juce::String filename = characterDB[index].filename;
    juce::String cleanTarget = filename.replace(" ", "").replace(".", "").replace("_", "").toLowerCase();
    
    int imageSize = 0;
    const char* imageData = nullptr;

    for (int i = 0; i < BinaryData::namedResourceListSize; ++i) {
        juce::String res = BinaryData::namedResourceList[i];
        if (res.replace("_", "").toLowerCase() == cleanTarget) {
            imageData = BinaryData::getNamedResource(BinaryData::namedResourceList[i], imageSize);
            break;
        }
    }

    if (imageData != nullptr) {
        spriteWindow->getContent()->setImage(juce::ImageCache::getFromMemory(imageData, imageSize));
    }
}

void SquabDanceAudioProcessorEditor::timerCallback() {
    if (spriteWindow != nullptr) {
        float speed = *audioProcessor.apvts.getRawParameterValue("speed");
        bool mir = *audioProcessor.apvts.getRawParameterValue("mirror") > 0.5f;
        int style = animationBox.getSelectedId() - 1; 
        
        // --- UPDATE TOGGLE TEXT DYNAMICALLY ---
        mirrorButton.setButtonText(mir ? "Reflection" : "No Reflection");

        int catIdx = categoryBox.getSelectedId() - 1;
        int frames = 8;

        if (catIdx >= 0 && catIdx < (int)characterDB.size() && style >= 0 && style < (int)characterDB[catIdx].anims.size()) {
            frames = characterDB[catIdx].anims[style].frameCount;
        }

        spriteWindow->getContent()->setSpeed(speed);
        spriteWindow->getContent()->updateParams(style, frames, mir); 
    }
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
    catLabel.setBounds(rightCol, 15, 100, 20);
    categoryBox.setBounds(rightCol, 35, 160, 22);

    animLabel.setBounds(rightCol, 65, 100, 20);
    animationBox.setBounds(rightCol, 85, 160, 22);

    // Rate Controls (Made Slider 25% larger and shifted everything right)
    speedLabel.setBounds(rightCol, 120, 75, 20); 
    speedSlider.setBounds(rightCol, 140, 75, 75); 

    // Sub Buttons Grid (Shifted Right to fit bigger slider)
    int btnX = rightCol + 85;
    int btnY = 145;
    hzButton.setBounds(btnX, btnY, 35, 20);
    openButton.setBounds(btnX + 40, btnY, 50, 20);
    
    syncButton.setBounds(btnX, btnY + 25, 35, 20);
    resetButton.setBounds(btnX + 40, btnY + 25, 50, 20);

    // Footer
    squabDadLabel.setBounds(btnX, 205, 100, 20);
}