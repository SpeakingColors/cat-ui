#include <juce_gui_basics/juce_gui_basics.h>
#include "catui/CatFrame.h"

class DemoContent : public juce::Component
{
public:
    DemoContent()
    {
        for (auto& k : knobs)
        {
            k.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
            k.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
            k.setRange(0.0, 1.0);
            addAndMakeVisible(k);
        }
    }
    void resized() override
    {
        auto r = getLocalBounds().reduced(16);
        const int w = r.getWidth() / 4;
        for (auto& k : knobs) k.setBounds(r.removeFromLeft(w).withHeight(w));
    }
private:
    juce::Slider knobs[4];
};

class DemoWindow : public juce::DocumentWindow
{
public:
    DemoWindow() : juce::DocumentWindow("cat-ui demo", catui::Colours::panel(), juce::DocumentWindow::allButtons)
    {
        setUsingNativeTitleBar(true);
        frame = new catui::CatFrame("cat-ui demo", 640, 360);
        content = new DemoContent();
        frame->content().addAndMakeVisible(content);
        setContentOwned(frame, true);
        frame->attachTo(*this);
        centreWithSize(640, 360);
        setVisible(true);
    }
    void closeButtonPressed() override { juce::JUCEApplication::getInstance()->systemRequestedQuit(); }
private:
    catui::CatFrame* frame = nullptr;
    juce::Component* content = nullptr;
};

class DemoApp : public juce::JUCEApplication
{
public:
    const juce::String getApplicationName() override    { return "CatUiDemo"; }
    const juce::String getApplicationVersion() override { return "0.1.0"; }
    void initialise(const juce::String&) override       { win = std::make_unique<DemoWindow>(); }
    void shutdown() override                            { win.reset(); }
private:
    std::unique_ptr<DemoWindow> win;
};

START_JUCE_APPLICATION(DemoApp)
