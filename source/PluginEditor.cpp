#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <unordered_map>

const int DEFAULT_CHARACTER_INDEX = 11; 

SquabDanceAudioProcessorEditor::SquabDanceAudioProcessorEditor (SquabDanceAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    setSize (880, 540);
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

    addAndMakeVisible(randomButton);
    randomButton.setButtonText("Click for Random Image");
    randomButton.setLookAndFeel(&customLookAndFeel); // Applies the Pill Shape!
    // Set a vibrant "Fire" base color (Vibrant Orange/Amber)
    randomButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xFFE67E22)); 
    randomButton.setColour(juce::TextButton::textColourOffId, juce::Colours::white);
    
    randomButton.onClick = [this] {
        if (characterDB.empty()) return;
        
        // 1. Pick a random category
        int randomCat = juce::Random::getSystemRandom().nextInt((int)characterDB.size());
        
        // Use sendNotificationSync so the animationBox populates *immediately* before step 2
        categoryBox.setSelectedId(randomCat + 1, juce::sendNotificationSync); 
        
        // 2. Pick a random animation within that new category
        int numAnims = characterDB[randomCat].anims.size();
        if (numAnims > 0) {
            int randomAnim = juce::Random::getSystemRandom().nextInt(numAnims);
            animationBox.setSelectedId(randomAnim + 1, juce::sendNotificationSync);
        }
    };

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

    auto setupReactKnob = [this](juce::Slider& s, juce::Label& l, juce::String text, juce::String paramID, std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>& attach) {
        addAndMakeVisible(l);
        l.setText(text, juce::dontSendNotification);
        l.setColour(juce::Label::textColourId, juce::Colours::lightgrey);
        l.setJustificationType(juce::Justification::centred);
        
        addAndMakeVisible(s);
        s.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        s.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 20);
        s.setColour(juce::Slider::textBoxTextColourId, juce::Colours::lightgrey);
        s.setNumDecimalPlacesToDisplay(0); 
        s.setTextValueSuffix(" %");        
        s.setLookAndFeel(&customLookAndFeel);
        s.setDoubleClickReturnValue(true, 100.0, juce::ModifierKeys::noModifiers);
        attach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts, paramID, s);
    };

    // Initialize Dry/Wet Knob
    setupReactKnob(dryWetSlider, dryWetLabel, "Dry / Wet", "dry_wet", dryWetAttachment);

    // Initialize Meter and Fader
    addAndMakeVisible(stereoMeter);
    
    addAndMakeVisible(outGainSlider);
    outGainSlider.setSliderStyle(juce::Slider::LinearVertical);
    outGainSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 20);
    outGainSlider.setColour(juce::Slider::textBoxTextColourId, juce::Colours::lightgrey);
    outGainSlider.setNumDecimalPlacesToDisplay(1);
    outGainSlider.setTextValueSuffix(" dB");
    outGainSlider.setLookAndFeel(&linearLookAndFeel);
    outGainAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts, "out_gain", outGainSlider);

// --- AUDIO REACTIVITY INITIALIZATION ---
    addAndMakeVisible(reactTitleLabel);
    reactTitleLabel.setText("Audio Reactivity", juce::dontSendNotification);
    reactTitleLabel.setColour(juce::Label::textColourId, juce::Colour(0xFFAAAAAA));

    addAndMakeVisible(reactButton);
    reactButton.setClickingTogglesState(true);
    reactButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xFF2A2A2A)); 
    reactButton.setColour(juce::TextButton::textColourOffId, juce::Colours::white);
    reactButton.setColour(juce::TextButton::buttonOnColourId, juce::Colour(0xFFFBB03B)); 
    reactButton.setColour(juce::TextButton::textColourOnId, juce::Colours::black);
    reactAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(audioProcessor.apvts, "audio_react", reactButton);
    
    reactButton.onClick = [this] {
        reactButton.setButtonText(reactButton.getToggleState() ? "Audio Reactive On" : "Audio Reactive Off");
    };
    reactButton.setButtonText(*audioProcessor.apvts.getRawParameterValue("audio_react") > 0.5f ? "Audio Reactive On" : "Audio Reactive Off");

    

    setupReactKnob(intensitySlider, intensityLabel, "Intensity", "react_intensity", intensityAttachment);
    setupReactKnob(colorSlider, colorLabel, "Color", "react_color", colorAttachment);
    setupReactKnob(pumpSlider, pumpLabel, "Pump", "react_pump", pumpAttachment);

   // --- AUDIO MANIPULATION INITIALIZATION ---
    addAndMakeVisible(manipTitleLabel);
    manipTitleLabel.setText("Audio Manipulation", juce::dontSendNotification);
    manipTitleLabel.setColour(juce::Label::textColourId, juce::Colour(0xFFAAAAAA));

    addAndMakeVisible(manipButton);
    manipButton.setClickingTogglesState(true);
    manipButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xFF2A2A2A)); 
    manipButton.setColour(juce::TextButton::textColourOffId, juce::Colours::white);
    
    // UPDATED: Brighter, punchy blue color
    manipButton.setColour(juce::TextButton::buttonOnColourId, juce::Colour(0xFF7CB9E8)); 
    manipButton.setColour(juce::TextButton::textColourOnId, juce::Colours::black);
    
    manipAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(audioProcessor.apvts, "audio_manip", manipButton);

    manipButton.onClick = [this] {
    manipButton.setButtonText(manipButton.getToggleState() ? "Audio Manipulation On" : "Audio Manipulation Off");
    };
    manipButton.setButtonText(*audioProcessor.apvts.getRawParameterValue("audio_manip") > 0.5f ? "Audio Manipulation On" : "Audio Manipulation Off");

    // Reuse the lambda to setup the new knobs
    setupReactKnob(dynamicSlider, dynamicLabel, "Dynamic", "manip_dynamic", dynamicAttachment);
    setupReactKnob(hueSlider, hueLabel, "Hue Analysis", "manip_hue", hueAttachment);
    setupReactKnob(panningSlider, panningLabel, "Panning", "manip_pan", panningAttachment);

    // --- INITIALIZE SATURATION DROPDOWN ---
    addAndMakeVisible(satTypeBox);
    satTypeBox.addItemList({ "Wavefolder", "Soft Clip", "Hard Clip", "Bitcrusher" }, 1);
    satTypeBox.setJustificationType(juce::Justification::centred);
    satTypeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(audioProcessor.apvts, "sat_type", satTypeBox);

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

    dynamicSlider.setLookAndFeel(nullptr);
    hueSlider.setLookAndFeel(nullptr);
    panningSlider.setLookAndFeel(nullptr);

    randomButton.setLookAndFeel(nullptr);

    spriteWindow = nullptr; 
}

void SquabDanceAudioProcessorEditor::timerCallback() {

    bool isSync = *audioProcessor.apvts.getRawParameterValue("sync_mode") > 0.5f;
    int syncIdx = (int)*audioProcessor.apvts.getRawParameterValue("sync_rate");
    bool reactOn = *audioProcessor.apvts.getRawParameterValue("audio_react") > 0.5f;
    float reactInt = *audioProcessor.apvts.getRawParameterValue("react_intensity");
    float reactCol = *audioProcessor.apvts.getRawParameterValue("react_color");
    float reactPumpAmount = *audioProcessor.apvts.getRawParameterValue("react_pump");
   
    // Grab the peaks from the DSP
    float peakL = audioProcessor.outputLevelL.load();
    float peakR = audioProcessor.outputLevelR.load();
    
    stereoMeter.setLevels(peakL, peakR);

    // Fade the peaks out slightly so the meter falls smoothly like a real LED
    audioProcessor.outputLevelL.store(peakL * 0.85f, std::memory_order_relaxed);
    audioProcessor.outputLevelR.store(peakR * 0.85f, std::memory_order_relaxed);


    spriteWindow->getContent()->updateAudioReact(reactOn, reactInt, reactCol, reactPumpAmount, audioProcessor.currentAudioLevel.load());
    
    const double syncBeatLengths[] = {
        4.0/2048.0, 4.0/1024.0, 4.0/512.0, 4.0/256.0, 4.0/128.0, 4.0/64.0, 4.0/48.0, 4.0/32.0,
        4.0/24.0, 4.0/16.0, 4.0/12.0, 4.0/8.0, 4.0/6.0, 12.0/16.0, 4.0/4.0, 20.0/16.0, 
        4.0/3.0, 12.0/8.0, 4.0/2.0, 12.0/4.0, 4.0/1.0
    };

    speedSlider.setVisible(!isSync);
    syncSlider.setVisible(isSync);
    
    if (spriteWindow == nullptr || spriteWindow->getContent() == nullptr) return;

    spriteWindow->getContent()->updateSync(isSync, syncBeatLengths[syncIdx], audioProcessor.currentPpq.load(), audioProcessor.isPlaying.load());

    audioProcessor.visualMotion.store(spriteWindow->getContent()->getMotion(), std::memory_order_relaxed);
    audioProcessor.visualHue.store(spriteWindow->getContent()->getHue(), std::memory_order_relaxed);
    audioProcessor.visualPan.store(spriteWindow->getContent()->getPan(), std::memory_order_relaxed);

    // --- DEBUG: PRINT ONCE PER SECOND ---
    static int debugTick = 0;
    if (++debugTick % 30 == 0) {
        float dyn = *audioProcessor.apvts.getRawParameterValue("manip_dynamic");
        float mot = audioProcessor.visualMotion.load();
       // DBG("1. BRIDGE Check | Dynamic Knob: " << dyn << " | Visual Motion: " << mot);
    }

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
    // --- EXPANDED GRID LAYOUT ---
    int margin = 30;
    int colWidth = 350; // Increased width for the columns
    
    int leftColX = margin;
    int rightColX = leftColX + colWidth + 40; // Starts precisely at x=420
    
    int topRowY = margin;
    int botRowY = 270; 

    // ==========================================
    // TOP LEFT: Branding & Sprite
    // ==========================================
    titleLabel.setBounds(leftColX, topRowY, colWidth, 30);
    birdLogo.setBounds(leftColX, topRowY + 30, colWidth, 130); 
    mirrorButton.setBounds(leftColX, topRowY + 170, colWidth, 25);

    // ==========================================
    // TOP RIGHT: Settings, Random Button & Speed
    // ==========================================
    categoryBox.setBounds(rightColX, topRowY + 15, colWidth, 24);
    animationBox.setBounds(rightColX, topRowY + 65, colWidth, 24);
    randomButton.setBounds(rightColX, topRowY + 105, colWidth, 24); 

    int topKnobY = topRowY + 135;
    
    scaleLabel.setBounds(rightColX + 20, topKnobY, 60, 20); 
    scaleSlider.setBounds(rightColX + 20, topKnobY + 20, 60, 75); 

    speedLabel.setBounds(rightColX + 110, topKnobY, 60, 20); 
    speedSlider.setBounds(rightColX + 110, topKnobY + 20, 60, 75); 
    syncSlider.setBounds(rightColX + 110, topKnobY + 20, 60, 75); 

    int btnX = rightColX + 200;
    int btnY = topKnobY + 25;
    hzButton.setBounds(btnX, btnY, 40, 20);
    openButton.setBounds(btnX + 55, btnY, 60, 20); 
    syncButton.setBounds(btnX, btnY + 25, 40, 20);
    resetButton.setBounds(btnX + 55, btnY + 25, 60, 20);

    // ==========================================
    // BOTTOM LEFT: Audio Reactivity
    // ==========================================
    reactTitleLabel.setBounds(leftColX, botRowY, colWidth, 20);
    reactButton.setBounds(leftColX, botRowY + 25, colWidth, 70);
    
    int botKnobY = botRowY + 110;
    int spacing = 95; // Wide, beautiful spacing for 3 knobs
    
    intensityLabel.setBounds(leftColX + 15, botKnobY, 60, 20);
    intensitySlider.setBounds(leftColX + 15, botKnobY + 20, 60, 75);
    
    colorLabel.setBounds(leftColX + 15 + spacing, botKnobY, 60, 20);
    colorSlider.setBounds(leftColX + 15 + spacing, botKnobY + 20, 60, 75);
    
    pumpLabel.setBounds(leftColX + 15 + (spacing * 2), botKnobY, 60, 20);
    pumpSlider.setBounds(leftColX + 15 + (spacing * 2), botKnobY + 20, 60, 75);

    // ==========================================
    // BOTTOM RIGHT: Audio Manipulation
    // ==========================================
    manipTitleLabel.setBounds(rightColX, botRowY, colWidth, 20);
    manipButton.setBounds(rightColX, botRowY + 25, colWidth, 70);
    
    int rSpacing = 85; // Perfect mathematical spacing to fit 4 knobs smoothly
    
    dynamicLabel.setBounds(rightColX + 5, botKnobY, 60, 20);
    dynamicSlider.setBounds(rightColX + 5, botKnobY + 20, 60, 75);
    satTypeBox.setBounds(rightColX - 5, botKnobY + 95, 80, 20); 
    
    hueLabel.setBounds(rightColX + 5 + rSpacing, botKnobY, 60, 20);
    hueSlider.setBounds(rightColX + 5 + rSpacing, botKnobY + 20, 60, 75);
    
    panningLabel.setBounds(rightColX + 5 + (rSpacing * 2), botKnobY, 60, 20);
    panningSlider.setBounds(rightColX + 5 + (rSpacing * 2), botKnobY + 20, 60, 75);

    dryWetLabel.setBounds(rightColX + 5 + (rSpacing * 3), botKnobY, 60, 20);
    dryWetSlider.setBounds(rightColX + 5 + (rSpacing * 3), botKnobY + 20, 60, 75);

    // ==========================================
    // OUTPUT STRIP (Far Right)
    // ==========================================
    int meterX = getWidth() - 55; // Pushed to the far right wall
    int meterY = 50; 
    int meterWidth = 24;
    int meterHeight = 400; 

    // The LED Meter sits visually in the background
    stereoMeter.setBounds(meterX, meterY, meterWidth, meterHeight);
    
    // The Slider is intentionally 54 pixels wide so the Text Box can render "-12.5 dB"
    // We center it directly over the 24-pixel meter using (meterX - 15)
    outGainSlider.setBounds(meterX - 15, meterY, 54, meterHeight + 35); 

    squabDadLabel.setBounds(rightColX + colWidth - 100, botKnobY + 100, 100, 20);
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