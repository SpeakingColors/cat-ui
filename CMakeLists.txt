cmake_minimum_required(VERSION 3.22)
project(cat_ui VERSION 0.1.0 LANGUAGES CXX)

# cat-ui: shared look and feel for the cat plugins (JUCE).
# Use from another project:
#   FetchContent_Declare(cat_ui GIT_REPOSITORY https://github.com/SpeakingColors/cat-ui GIT_TAG main)
#   FetchContent_MakeAvailable(cat_ui)   # after JUCE is available
#   target_link_libraries(MyApp PRIVATE cat_ui)

if(NOT TARGET juce::juce_gui_basics)
    include(FetchContent)
    FetchContent_Declare(juce GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
        GIT_TAG 8.0.16-security GIT_SHALLOW TRUE)
    FetchContent_MakeAvailable(juce)
endif()

add_library(cat_ui STATIC src/CatLookAndFeel.cpp src/CatFrame.cpp)
add_library(cat_ui::cat_ui ALIAS cat_ui)
target_include_directories(cat_ui PUBLIC include)
target_compile_features(cat_ui PUBLIC cxx_std_17)
target_link_libraries(cat_ui PUBLIC
    juce::juce_gui_basics
    juce::juce_graphics
    juce::juce_recommended_config_flags
    juce::juce_recommended_warning_flags)

if(CAT_UI_BUILD_DEMO)
    juce_add_gui_app(CatUiDemo PRODUCT_NAME "CatUiDemo" COMPANY_NAME "SpeakingColors")
    target_sources(CatUiDemo PRIVATE demo/Main.cpp)
    target_compile_definitions(CatUiDemo PRIVATE JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0
        JUCE_APPLICATION_NAME_STRING="CatUiDemo" JUCE_APPLICATION_VERSION_STRING="0.1.0")
    target_link_libraries(CatUiDemo PRIVATE cat_ui juce::juce_recommended_lto_flags)
endif()
