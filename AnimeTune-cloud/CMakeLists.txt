cmake_minimum_required(VERSION 3.24)
project(AnimeTune VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

include(FetchContent)
FetchContent_Declare(
    JUCE
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG 9.0.5
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(JUCE)

juce_add_plugin(AnimeTune
    COMPANY_NAME "AnimeTune"
    BUNDLE_ID com.animetune.plugin
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    COPY_PLUGIN_AFTER_BUILD TRUE
    PLUGIN_MANUFACTURER_CODE AnTu
    PLUGIN_CODE At01
    FORMATS VST3
    PRODUCT_NAME "AnimeTune"
    VST3_CATEGORIES "Fx" "Pitch Correction"
)

target_sources(AnimeTune PRIVATE
    Source/PluginProcessor.cpp
    Source/PluginEditor.cpp
    Source/DSP/YinPitchDetector.cpp
    Source/DSP/LowLatencyVoiceEngine.cpp
)

target_include_directories(AnimeTune PRIVATE Source)

target_link_libraries(AnimeTune PRIVATE
    juce::juce_audio_utils
    juce::juce_dsp
    juce::juce_recommended_config_flags
    juce::juce_recommended_lto_flags
    juce::juce_recommended_warning_flags
)

juce_generate_juce_header(AnimeTune)

if(MSVC)
    target_compile_options(AnimeTune PRIVATE /W4 /permissive-)
endif()
