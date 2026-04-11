cmake_minimum_required(VERSION 3.22)

project(MassiveSynth VERSION 1.0.1)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ============================================================
# macOS — universal binary (Apple Silicon + Intel) + min target
# ============================================================
if(APPLE)
    if(NOT DEFINED CMAKE_OSX_ARCHITECTURES)
        set(CMAKE_OSX_ARCHITECTURES "arm64;x86_64" CACHE STRING "Build architectures" FORCE)
    endif()
    if(NOT DEFINED CMAKE_OSX_DEPLOYMENT_TARGET)
        set(CMAKE_OSX_DEPLOYMENT_TARGET "10.15" CACHE STRING "Min macOS version" FORCE)
    endif()
endif()

# ============================================================
# JUCE — point to the local copy already in the repo
# ============================================================
add_subdirectory(JUCE)

# ============================================================
# Plugin target
# ============================================================
juce_add_plugin(MassiveSynth
    # --- Identity ---
    COMPANY_NAME            "MyStudio"
    PLUGIN_MANUFACTURER_CODE MySt
    PLUGIN_CODE             DkTn
    PRODUCT_NAME            "Ducktane"
    VERSION                 "1.0.1"

    # --- Formats ---
    FORMATS                 VST3 AU Standalone
    # Add AU on macOS by appending AU above (requires Xcode SDK)

    # --- Plugin type flags ---
    IS_SYNTH                TRUE
    NEEDS_MIDI_INPUT        TRUE
    NEEDS_MIDI_OUTPUT       FALSE
    IS_MIDI_EFFECT          FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE

    # --- VST3 category ---
    VST3_CATEGORIES         "Instrument|Synth"

    # --- Bundle ID (macOS AU/AUv3) ---
    BUNDLE_ID               "com.mystudio.ducktane"

    # --- AU (macOS) ---
    AU_MAIN_TYPE            "kAudioUnitType_MusicDevice"
    PLUGIN_AU_EXPORT_PREFIX DucktaneAU

    # --- Icons (leave empty; we draw everything in code) ---
    ICON_BIG                ""
    ICON_SMALL              ""
)

# ============================================================
# Binary assets (logo.png embedded at compile time)
# ============================================================
juce_add_binary_data(DucktaneAssets
    NAMESPACE   DucktaneAssets
    HEADER_NAME DucktaneAssets.h
    SOURCES
        logo.png
)

# ============================================================
# Source files
# ============================================================
target_sources(MassiveSynth
    PRIVATE
        # Core plugin
        Source/PluginProcessor.cpp
        Source/PluginProcessor.h
        Source/PluginEditor.cpp
        Source/PluginEditor.h

        # DSP layer
        Source/DSP/Oscillator.cpp
        Source/DSP/Oscillator.h
        Source/DSP/SynthVoice.cpp
        Source/DSP/SynthVoice.h
        Source/DSP/SynthSound.h
        Source/DSP/StateVariableFilter.cpp
        Source/DSP/StateVariableFilter.h
        Source/DSP/AdsrEnvelope.h
        Source/DSP/Lfo.h
        Source/DSP/FxChain.cpp
        Source/DSP/FxChain.h

        # UI layer
        Source/UI/WaveformDisplay.cpp
        Source/UI/WaveformDisplay.h
        Source/UI/CustomLookAndFeel.cpp
        Source/UI/CustomLookAndFeel.h
        Source/UI/OscillatorPanel.cpp
        Source/UI/OscillatorPanel.h
        Source/UI/FilterPanel.cpp
        Source/UI/FilterPanel.h
        Source/UI/EnvelopePanel.cpp
        Source/UI/EnvelopePanel.h
        Source/UI/LfoPanel.cpp
        Source/UI/LfoPanel.h
        Source/UI/FxPanel.cpp
        Source/UI/FxPanel.h
        Source/UI/PresetPanel.cpp
        Source/UI/PresetPanel.h

        # Data / Preset management
        Source/Data/PresetManager.cpp
        Source/Data/PresetManager.h

        # Browser
        Source/UI/BrowserPanel.cpp
        Source/UI/BrowserPanel.h
)

# ============================================================
# Generate JuceHeader.h (gives us #include <JuceHeader.h> in all source files)
# ============================================================
juce_generate_juce_header(MassiveSynth)

# ============================================================
# Include paths
# ============================================================
target_include_directories(MassiveSynth
    PRIVATE
        Source
        Source/DSP
        Source/UI
        Source/Data
)

# ============================================================
# Compile definitions
# ============================================================
target_compile_definitions(MassiveSynth
    PUBLIC
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
        JUCE_GLOBAL_MODULE_SETTINGS_INCLUDED=1
)

# Silence "deprecated" warnings from JUCE's own Font API change
if(MSVC)
    target_compile_options(MassiveSynth PRIVATE /wd4996)
endif()

# ============================================================
# JUCE modules
# ============================================================
target_link_libraries(MassiveSynth
    PRIVATE
        DucktaneAssets
        juce::juce_audio_basics
        juce::juce_audio_devices
        juce::juce_audio_formats
        juce::juce_audio_plugin_client
        juce::juce_audio_processors
        juce::juce_audio_utils
        juce::juce_core
        juce::juce_data_structures
        juce::juce_dsp
        juce::juce_events
        juce::juce_graphics
        juce::juce_gui_basics
        juce::juce_gui_extra
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)
