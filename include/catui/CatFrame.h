#pragma once
#include "CatLookAndFeel.h"
#include <functional>

namespace catui
{
// Top bar with title, info, settings and "reset size to 100%" buttons.
class CatHeader : public juce::Component
{
public:
    explicit CatHeader(juce::String title);
    std::function<void()> onInfo, onSettings, onReset100;
    void paint(juce::Graphics&) override;
    void resized() override;
private:
    juce::String title;
    juce::TextButton info{ "i" }, gear{ "gear" }, reset{ "100%" };
};

// Base for plugin and app windows: wood cheeks, header, content area,
// resizable from all four corners and edges with locked aspect ratio.
class CatFrame : public juce::Component
{
public:
    CatFrame(juce::String title, int baseWidth, int baseHeight);
    ~CatFrame() override;

    // Put your UI in here.
    juce::Component& content() { return contentHolder; }
    CatHeader& header() { return head; }

    // Hook for the owning window/editor so it can lock aspect ratio.
    void attachTo(juce::ResizableWindow& window);
    void resetTo100();

    void paint(juce::Graphics&) override;
    void resized() override;

    static constexpr int cheek = 22;

private:
    int baseW, baseH;
    CatLookAndFeel laf;
    CatHeader head;
    juce::Component contentHolder;
    juce::ResizableWindow* window = nullptr;
};
}
