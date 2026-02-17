cmake_minimum_required(VERSION 3.15)

project(SquabDance VERSION 0.0.1)

# --- BYPASS VST2/VST3 CONFLICT ERROR ---
add_definitions(-DJUCE_VST3_CAN_REPLACE_VST2=0)

# 1. Download JUCE automatically
include(FetchContent)
FetchContent_Declare(
    JUCE
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG        master
)
FetchContent_MakeAvailable(JUCE)

# 2. Create the Plugin (VST3 and Standalone only)
juce_add_plugin(SquabDance
    COMPANY_NAME "Squab Dad"
    BUNDLE_ID "com.SquabDad.SquabDance" 
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT TRUE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS TRUE
    FORMATS VST3 Standalone   # No VST2 here
    PLUGIN_CODE "SqDa" 
    VST2_ID "SqDa"            # Still needed for the UID math
)

# 3. Link the Source Files
target_sources(SquabDance
    PRIVATE
    Source/PluginProcessor.cpp
    Source/PluginProcessor.h
    Source/PluginEditor.cpp
    Source/PluginEditor.h
)

# 4. Link Libraries
target_link_libraries(SquabDance
    PRIVATE
    juce::juce_audio_utils
    juce::juce_recommended_config_flags
)

# . EMBED ASSETS (Add all your PNGs here)
juce_add_binary_data(SquabAssets
    SOURCES
        "Assets/Cat.png"
        "Assets/Dog.png"
        "Assets/Frog.png"
        "Assets/Other Animals.png"
        "Assets/Nyan Cat.png"
        "Assets/Link.png"
        "Assets/Video Game.png"
        "Assets/Twice.png"
        "Assets/Anime.png"
        "Assets/Anime 2.png"
        "Assets/Fruity Chan.png"
        "Assets/Cars.png"
        "Assets/Other Dance.png"
)

# 6. Link the Assets to your Plugin
target_link_libraries(SquabDance PRIVATE SquabAssets)

# 7. Generate Header (Must be last)
juce_generate_juce_header(SquabDance)