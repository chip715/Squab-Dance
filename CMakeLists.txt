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

# 5. Generate Header
juce_generate_juce_header(SquabDance)