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
   # --- SINGLE SHEET CATEGORIES ---
        "Assets/Frog.png"
        "Assets/Nyan Cat.png"
        "Assets/Link.png"
        "Assets/Fruity Chan.png"
        "Assets/Squab_Logo.png"

        # --- MULTI-SHEET CATEGORIES ---
        # Cars (8 Sheets)
        "Assets/Cars/Cars-0.png"
        "Assets/Cars/Cars-1.png"
        "Assets/Cars/Cars-2.png"
        "Assets/Cars/Cars-3.png"
        "Assets/Cars/Cars-4.png"
        "Assets/Cars/Cars-5.png"
        "Assets/Cars/Cars-6.png"
        "Assets/Cars/Cars-7.png"
        
        # Cat (8 Sheets: 543 total frames)
        "Assets/Cat/Cat-0.png"
        "Assets/Cat/Cat-1.png"
        "Assets/Cat/Cat-2.png"
        "Assets/Cat/Cat-3.png"
        "Assets/Cat/Cat-4.png"
        "Assets/Cat/Cat-5.png"
        "Assets/Cat/Cat-6.png"
        "Assets/Cat/Cat-7.png"

        # Dog (7 Sheets: 482 total frames)
        "Assets/Dog/Dog-0.png"
        "Assets/Dog/Dog-1.png"
        "Assets/Dog/Dog-2.png"
        "Assets/Dog/Dog-3.png"
        "Assets/Dog/Dog-4.png"
        "Assets/Dog/Dog-5.png"
        "Assets/Dog/Dog-6.png"

        # Other Animals (9 Sheets: 610 total frames)
        "Assets/Other Animals/OtherAnimals-0.png"
        "Assets/Other Animals/OtherAnimals-1.png"
        "Assets/Other Animals/OtherAnimals-2.png"
        "Assets/Other Animals/OtherAnimals-3.png"
        "Assets/Other Animals/OtherAnimals-4.png"
        "Assets/Other Animals/OtherAnimals-5.png"
        "Assets/Other Animals/OtherAnimals-6.png"
        "Assets/Other Animals/OtherAnimals-7.png"
        "Assets/Other Animals/OtherAnimals-8.png"

        # Video Game (4 Sheets: 224 total frames)
        "Assets/Video Game/VideoGame-0.png"
        "Assets/Video Game/VideoGame-1.png"
        "Assets/Video Game/VideoGame-2.png"
        "Assets/Video Game/VideoGame-3.png"

        # Twice (5 Sheets: 357 total frames)
        "Assets/Twice/Twice-0.png"
        "Assets/Twice/Twice-1.png"
        "Assets/Twice/Twice-2.png"
        "Assets/Twice/Twice-3.png"
        "Assets/Twice/Twice-4.png"

        # Anime (3 Sheets: 216 total frames)
        "Assets/Anime/Anime-0.png"
        "Assets/Anime/Anime-1.png"
        "Assets/Anime/Anime-2.png"
        
        # Anime 2 (5 Sheets: 352 total frames)
        "Assets/Anime 2/Anime2-0.png"
        "Assets/Anime 2/Anime2-1.png"
        "Assets/Anime 2/Anime2-2.png"
        "Assets/Anime 2/Anime2-3.png"
        "Assets/Anime 2/Anime2-4.png"

        # Other Dance (6 Sheets: 400 total frames)
        "Assets/Other Dance/OtherDance-0.png"
        "Assets/Other Dance/OtherDance-1.png"
        "Assets/Other Dance/OtherDance-2.png"
        "Assets/Other Dance/OtherDance-3.png"
        "Assets/Other Dance/OtherDance-4.png"
        "Assets/Other Dance/OtherDance-5.png"
)

# 6. Link the Assets to your Plugin
target_link_libraries(SquabDance PRIVATE SquabAssets)

# 7. Generate Header (Must be last)
juce_generate_juce_header(SquabDance)