#include <juce_gui_basics/juce_gui_basics.h>
#include "catui/CatFrame.h"
#include "catui/RenderGate.h"

class DemoContent : public juce::Component
{
public:
    DemoContent()
    {
        // Moving scope line: only animates while the window is visible and in front.
        gate.onFrame = [this] { phase += 0.08f; repaint(scopeArea); };
        gate.start(30);
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
        auto top = r.removeFromTop(w);
        for (auto& k : knobs) k.setBounds(top.removeFromLeft(w).withHeight(w));
        scopeArea = r.reduced(0, 8);
    }
    void paint(juce::Graphics& g) override
    {
        g.setColour(juce::Colour(0xff050607));
        g.fillRect(scopeArea);
        juce::Path p;
        const auto a = scopeArea.toFloat();
        for (int x = 0; x < scopeArea.getWidth(); ++x)
        {
            const float y = a.getCentreY() - std::sin(phase + (float) x * 0.05f) * a.getHeight() * 0.4f;
            if (x == 0) p.startNewSubPath(a.getX(), y); else p.lineTo(a.getX() + (float) x, y);
        }
        g.setColour(juce::Colour(0xffcfe3ff));
        g.strokePath(p, juce::PathStrokeType(1.6f));
    }
private:
    catui::RenderGate gate{ *this };
    juce::Rectangle<int> scopeArea;
    float phase = 0.0f;
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
