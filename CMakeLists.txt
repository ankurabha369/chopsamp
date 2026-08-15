cmake_minimum_required(VERSION 3.20)

project(ChopSamp VERSION 1.0.0)

# Fetch JUCE
include(FetchContent)
FetchContent_Declare(
    juce
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG        8.0.0
)
FetchContent_MakeAvailable(juce)

# Create the VST3 and AU plugin target
juce_add_plugin(ChopSamp
    COMPANY_NAME "Ankur Rabha"
    BUNDLE_ID "com.ankurrabha.chopsamp"
    PLUGIN_NAME "ChopSamp"
    PLUGIN_MANUFACTURER_CODE "Anku"
    PLUGIN_CODE "Chps"
    FORMATS VST3 Standalone
    PRODUCT_NAME "ChopSamp"
    IS_SYNTH TRUE
    NEEDS_MIDI_INPUT TRUE
    NEEDS_MIDI_OUTPUT TRUE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS TRUE
    COPY_PLUGIN_AFTER_BUILD TRUE
    ICON_BIG Assets/icon.png
    ICON_SMALL Assets/icon.png
)

juce_generate_juce_header(ChopSamp)

# Add our source files
target_sources(ChopSamp
    PRIVATE
        Source/PluginProcessor.cpp
        Source/PluginProcessor.h
        Source/PluginEditor.cpp
        Source/PluginEditor.h
        Source/WaveformComponent.cpp
        Source/WaveformComponent.h
        Source/SliceControlsComponent.cpp
        Source/SliceControlsComponent.h
        Source/CustomLookAndFeel.cpp
)

# Compile settings
target_compile_features(ChopSamp PUBLIC cxx_std_20)

target_compile_definitions(ChopSamp
    PUBLIC
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
)

# Add binary assets
juce_add_binary_data(ChopSampAssets
    SOURCES
        Assets/logo.png
        Assets/slice_off.png
        Assets/slice_on.png
        Assets/bg_texture.jpg
        Assets/Minecraft.ttf
        Assets/reverse_off.png
        Assets/reverse_on.png
        Assets/dragmidi_off.png
        Assets/dragmidi_on.png
        Assets/dragaudio_off.png
        Assets/dragaudio_on.png
        Assets/record_off.png
        Assets/record_on.png
        Assets/skrew.png
        Assets/knob_base.png
        Assets/knob_rotating_part.png
        Assets/applytoall_off.png
        Assets/applytoall_on.png
        Assets/chromatic_keys.png
        Assets/only_white_keys.png
        Assets/Roboto-Medium.ttf
        Assets/Roboto-Bold.ttf
        Assets/icon.png
)

# Link to JUCE modules we need
target_link_libraries(ChopSamp
    PRIVATE
        juce::juce_audio_utils
        juce::juce_dsp
        ChopSampAssets
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)
