#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <unordered_map>

const int DEFAULT_CHARACTER_INDEX = 11; 

SquabDanceAudioProcessorEditor::SquabDanceAudioProcessorEditor (SquabDanceAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    setSize (460, 260); 
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

    // 2. LOAD BIRD LOGO
    addAndMakeVisible(birdLogo);
    birdLogo.setImagePlacement(juce::RectanglePlacement::centred | juce::RectanglePlacement::onlyReduceInSize);
    
    for (int i = 0; i < BinaryData::namedResourceListSize; ++i) {
        juce::String res = BinaryData::namedResourceList[i];
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
    speedSlider.setLookAndFeel(&customLookAndFeel);
    speedSlider.setDoubleClickReturnValue(true, 10.0, juce::ModifierKeys::noModifiers);
    speedAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts, "speed", speedSlider);

    // SCALE KNOB
    addAndMakeVisible(scaleLabel); 
    scaleLabel.setText("Scale", juce::dontSendNotification);
    scaleLabel.setColour(juce::Label::textColourId, juce::Colours::lightgrey);
    scaleLabel.setJustificationType(juce::Justification::centred);
    
    addAndMakeVisible(scaleSlider);
    scaleSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    scaleSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 20);
    scaleSlider.setColour(juce::Slider::textBoxTextColourId, juce::Colours::lightgrey);
    scaleSlider.setNumDecimalPlacesToDisplay(0); 
    scaleSlider.setTextValueSuffix(" %");        
    scaleSlider.setLookAndFeel(&customLookAndFeel);
    scaleSlider.setDoubleClickReturnValue(true, 100.0, juce::ModifierKeys::noModifiers);
    scaleAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts, "scale", scaleSlider);

    // SYNC KNOB
    addAndMakeVisible(syncSlider);
    syncSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    syncSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 20);
    syncSlider.setColour(juce::Slider::textBoxTextColourId, juce::Colours::lightgrey);
    syncSlider.setLookAndFeel(&customLookAndFeel);
    syncSlider.setDoubleClickReturnValue(true, 14.0, juce::ModifierKeys::noModifiers);
    syncAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts, "sync_rate", syncSlider);

    // 5. BUTTONS
    addAndMakeVisible(mirrorButton);
    mirrorButton.setClickingTogglesState(true);
    mirrorButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xFF1E1E1E)); 
    mirrorButton.setColour(juce::TextButton::textColourOffId, juce::Colours::white);
    mirrorButton.setColour(juce::TextButton::buttonOnColourId, juce::Colour(0xFFFBB03B)); 
    mirrorButton.setColour(juce::TextButton::textColourOnId, juce::Colours::black);
    mirrorAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(audioProcessor.apvts, "mirror", mirrorButton);

    // HZ BUTTON 
    addAndMakeVisible(hzButton); 
    hzButton.setButtonText("Hz");
    hzButton.setClickingTogglesState(true);
    hzButton.setRadioGroupId(1); 
    hzButton.setToggleState(true, juce::dontSendNotification); 
    hzButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xFF222222)); 
    hzButton.setColour(juce::TextButton::buttonOnColourId, juce::Colour(0xFFFBB03B)); 
    hzButton.setColour(juce::TextButton::textColourOffId, juce::Colours::white);
    hzButton.setColour(juce::TextButton::textColourOnId, juce::Colours::black);

    // SYNC BUTTON 
    addAndMakeVisible(syncButton); 
    syncButton.setButtonText(juce::CharPointer_UTF8("\xe2\x99\xaa")); 
    syncButton.setClickingTogglesState(true);
    syncButton.setRadioGroupId(1); 
    syncButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xFF222222)); 
    syncButton.setColour(juce::TextButton::buttonOnColourId, juce::Colour(0xFFFBB03B)); 
    syncButton.setColour(juce::TextButton::textColourOffId, juce::Colours::white);
    syncButton.setColour(juce::TextButton::textColourOnId, juce::Colours::black);

    hzButton.onClick = [this] {
        audioProcessor.apvts.getParameter("sync_mode")->setValueNotifyingHost(0.0f);
    };
    syncButton.onClick = [this] {
        audioProcessor.apvts.getParameter("sync_mode")->setValueNotifyingHost(1.0f);
    };

    // OPEN BUTTON
    addAndMakeVisible(openButton); 
    openButton.setClickingTogglesState(true);
    openButton.setToggleState(true, juce::dontSendNotification);
    openButton.setButtonText("Open"); 
    openButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xFF222222)); 
    openButton.setColour(juce::TextButton::textColourOffId, juce::Colours::white);
    openButton.setColour(juce::TextButton::buttonOnColourId, juce::Colour(0xFFFBB03B)); 
    openButton.setColour(juce::TextButton::textColourOnId, juce::Colours::black);
    
    openButton.onClick = [this] {
        bool isOpen = openButton.getToggleState();
        openButton.setButtonText(isOpen ? "Open" : "Closed");
        if (spriteWindow != nullptr) {
            spriteWindow->setVisible(isOpen);
        }
    };

    // RESET BUTTON
    addAndMakeVisible(resetButton); resetButton.setButtonText("Reset");
    resetButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xFF222222)); 
    resetButton.setColour(juce::TextButton::textColourOffId, juce::Colours::white);
    
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
    speedSlider.setLookAndFeel(nullptr);
    syncSlider.setLookAndFeel(nullptr);
    scaleSlider.setLookAndFeel(nullptr);
    spriteWindow = nullptr; 
}

void SquabDanceAudioProcessorEditor::timerCallback() {

    bool isSync = *audioProcessor.apvts.getRawParameterValue("sync_mode") > 0.5f;
    int syncIdx = (int)*audioProcessor.apvts.getRawParameterValue("sync_rate");
    
    const double syncBeatLengths[] = {
        4.0/2048.0, 4.0/1024.0, 4.0/512.0, 4.0/256.0, 4.0/128.0, 4.0/64.0, 4.0/48.0, 4.0/32.0,
        4.0/24.0, 4.0/16.0, 4.0/12.0, 4.0/8.0, 4.0/6.0, 12.0/16.0, 4.0/4.0, 20.0/16.0, 
        4.0/3.0, 12.0/8.0, 4.0/2.0, 12.0/4.0, 4.0/1.0
    };

    speedSlider.setVisible(!isSync);
    syncSlider.setVisible(isSync);
    
    if (spriteWindow == nullptr || spriteWindow->getContent() == nullptr) return;

    spriteWindow->getContent()->updateSync(isSync, syncBeatLengths[syncIdx], audioProcessor.currentPpq.load(), audioProcessor.isPlaying.load());

    float speed = *audioProcessor.apvts.getRawParameterValue("speed");
    bool mir = *audioProcessor.apvts.getRawParameterValue("mirror") > 0.5f;
    int style = animationBox.getSelectedId() - 1; 
    
    mirrorButton.setButtonText(mir ? "Reflection" : "No Reflection");

    float scaleVal = *audioProcessor.apvts.getRawParameterValue("scale") / 100.0f;
    spriteWindow->getContent()->setScale(scaleVal);

    int catIdx = categoryBox.getSelectedId() - 1;
    
    int frames = 8;
    int hRow = 9;    
    int hFrames = 8; 

    if (catIdx >= 0 && catIdx < (int)characterDB.size()) {
        auto& anims = characterDB[catIdx].anims;
        
        if (style >= 0 && style < (int)anims.size()) {
            frames = anims[style].frameCount;
        }

        bool foundHeld = false;
        for (int i = 0; i < (int)anims.size(); ++i) {
            juce::String name = anims[i].name.toLowerCase();
            if (name.contains("held") || name.contains("drag") || name.contains("pick")) {
                hRow = i;
                hFrames = anims[i].frameCount;
                foundHeld = true;
                break; 
            }
        }

        if (!foundHeld && anims.size() > 9) {
            hRow = 9;
            hFrames = anims[9].frameCount;
        }
    }

    spriteWindow->getContent()->setSpeed(speed);
    spriteWindow->getContent()->updateParams(style, frames, hRow, hFrames, mir); 
}

void SquabDanceAudioProcessorEditor::paint (juce::Graphics& g) { 
    g.fillAll (juce::Colour(0xFF424242)); 
}

void SquabDanceAudioProcessorEditor::resized() {
    // --- LEFT COLUMN ---
    titleLabel.setBounds(20, 15, 200, 30);
    birdLogo.setBounds(20, 50, 180, 130); 
    
    // Exact baseline match
    mirrorButton.setBounds(20, 205, 180, 20);

    // --- RIGHT COLUMN ---
    int rightCol = 230; 
    
    categoryBox.setBounds(rightCol, 35, 205, 22);
    animationBox.setBounds(rightCol, 85, 205, 22);

    // Scale Knob
    scaleLabel.setBounds(rightCol, 130, 60, 20); 
    scaleSlider.setBounds(rightCol, 150, 60, 75); 

    // Rate / Sync Knob
    speedLabel.setBounds(rightCol + 65, 130, 60, 20); 
    speedSlider.setBounds(rightCol + 65, 150, 60, 75); 
    syncSlider.setBounds(rightCol + 65, 150, 60, 75); 

    // Buttons
    int btnX = rightCol + 135;
    int btnY = 150;
    hzButton.setBounds(btnX, btnY, 35, 20);
    openButton.setBounds(btnX + 40, btnY, 50, 20);
    
    syncButton.setBounds(btnX, btnY + 25, 35, 20);
    resetButton.setBounds(btnX + 40, btnY + 25, 50, 20);

    // Exact baseline match
    squabDadLabel.setBounds(btnX, 205, 100, 20);
} 

void SquabDanceAudioProcessorEditor::preCacheImages() {
    cachedSprites.clear();

    // Launch a background thread so the UI doesn't freeze during startup
    juce::Thread::launch([this]() {
        DBG("--- Background Image Caching Started ---");
        
        for (int i = 0; i < BinaryData::namedResourceListSize; ++i) {
            int size = 0;
            const char* data = BinaryData::getNamedResource(BinaryData::namedResourceList[i], size);
            
            if (data != nullptr) {
                // 1. Decode the PNG data in the background (This is the heavy CPU math!)
                juce::Image img = juce::ImageCache::getFromMemory(data, size);
                
                if (img.isValid()) {
                    // 2. Safely push the decoded image to the main thread so it stays alive in RAM
                    juce::MessageManager::callAsync([this, img]() {
                        cachedSprites.push_back(img);
                    });
                }
            }
        }
        DBG("--- Background Image Caching Complete ---");
    });
}
void SquabDanceAudioProcessorEditor::loadCharacterImage(int index) {
    if (index < 0 || index >= (int)characterDB.size()) return;
    
    auto& charDef = characterDB[index];
    std::vector<juce::Image> loadedImages;
    
    for (const auto& fname : charDef.filenames) {
        int size = 0;
        juce::String searchTarget = fname.upToFirstOccurrenceOf(".", false, false)
                                         .toLowerCase()
                                         .replace("-", "");
                                       
        bool found = false;
        
        for (int i = 0; i < BinaryData::namedResourceListSize; ++i) {
            juce::String resName = juce::String(BinaryData::namedResourceList[i]).toLowerCase();
            
            if (resName.contains(searchTarget.replace(" ", "_")) || resName.contains(searchTarget.replace(" ", ""))) {
                const char* data = BinaryData::getNamedResource(BinaryData::namedResourceList[i], size);
                if (data != nullptr) {
                    juce::Image img = juce::ImageCache::getFromMemory(data, size);
                    if (img.isValid()) {
                        loadedImages.push_back(img);
                        found = true;
                    }
                }
                break;
            }
        }
    }
    
    if (spriteWindow != nullptr && spriteWindow->getContent() != nullptr) {
        auto* content = spriteWindow->getContent();
        content->setSpriteData(charDef.isGridSprite, charDef.anims, loadedImages);
    }
}