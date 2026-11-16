cmake_minimum_required(VERSION 3.22)
project(alias_audit VERSION 0.1.0 LANGUAGES C CXX)

# alias_audit: loads a built VST3 effect, feeds it sine waves with its drive-like knobs turned up and reports how much of the output
# energy is NOT a harmonic of the input (aliasing, plus any modulation sidebands). Report only, it never fails a build.
include(FetchContent)
FetchContent_Declare(juce GIT_REPOSITORY https://github.com/juce-framework/JUCE.git GIT_TAG 8.0.16-security GIT_SHALLOW TRUE)
FetchContent_MakeAvailable(juce)

juce_add_console_app(alias_audit PRODUCT_NAME "alias_audit")
target_sources(alias_audit PRIVATE alias_audit.cpp)
target_compile_features(alias_audit PUBLIC cxx_std_17)
target_compile_definitions(alias_audit PRIVATE JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0 JUCE_PLUGINHOST_VST3=1 JUCE_PLUGINHOST_LV2=0 JUCE_PLUGINHOST_AU=0 JUCE_STANDALONE_APPLICATION=1)
target_link_libraries(alias_audit PRIVATE juce::juce_audio_processors juce::juce_audio_formats juce::juce_dsp
    PUBLIC juce::juce_recommended_config_flags)
