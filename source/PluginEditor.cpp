#include "PluginProcessor.h"
#include "PluginEditor.h"

SquabDanceAudioProcessorEditor::SquabDanceAudioProcessorEditor (SquabDanceAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    setSize (300, 300); 
    characterDB = SpriteDatabase::getDatabase();

    addAndMakeVisible(catLabel); catLabel.setText("Category", juce::dontSendNotification);
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

    addAndMakeVisible(animLabel); animLabel.setText("Animation", juce::dontSendNotification);
    addAndMakeVisible(animationBox);

    addAndMakeVisible(speedLabel); speedLabel.setText("Speed (Hz)", juce::dontSendNotification);
    addAndMakeVisible(speedSlider);
    speedSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    speedSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 20);
    speedSlider.setNumDecimalPlacesToDisplay(0); // Hides the ".0"
    speedAttachment = std::make_unique<SliderAttachment>(audioProcessor.apvts, "speed", speedSlider);

    addAndMakeVisible(mirrorButton);
    mirrorButton.setButtonText("Mirror Reflection");
    mirrorAttachment = std::make_unique<ButtonAttachment>(audioProcessor.apvts, "mirror", mirrorButton);

    spriteWindow = std::make_unique<SpriteWindow>("Squab Visuals");
    
    categoryBox.setSelectedId(11); 
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
    } else {
        DBG("FAIL: Could not load " + filename);
    }
}

void SquabDanceAudioProcessorEditor::timerCallback() {
    if (spriteWindow != nullptr) {
        float speed = *audioProcessor.apvts.getRawParameterValue("speed");
        bool mir = *audioProcessor.apvts.getRawParameterValue("mirror") > 0.5f;
        int style = animationBox.getSelectedId() - 1; 
        
        int catIdx = categoryBox.getSelectedId() - 1;
        int frames = 8;

        if (catIdx >= 0 && catIdx < (int)characterDB.size() && style >= 0 && style < (int)characterDB[catIdx].anims.size()) {
            frames = characterDB[catIdx].anims[style].frameCount;
        }

        spriteWindow->getContent()->setSpeed(speed);
        spriteWindow->getContent()->updateParams(style, frames, mir); 
    }
}

void SquabDanceAudioProcessorEditor::paint (juce::Graphics& g) { g.fillAll (juce::Colours::darkgrey); }

void SquabDanceAudioProcessorEditor::resized() {
    int margin = 20;
    catLabel.setBounds(margin, 20, 100, 20);
    categoryBox.setBounds(margin, 40, getWidth()-(margin*2), 30);
    animLabel.setBounds(margin, 80, 100, 20);
    animationBox.setBounds(margin, 100, getWidth()-(margin*2), 30);
    speedLabel.setBounds(margin, 150, 100, 20);
    speedSlider.setBounds(margin, 170, 100, 100);
    mirrorButton.setBounds(150, 200, 140, 30);
}