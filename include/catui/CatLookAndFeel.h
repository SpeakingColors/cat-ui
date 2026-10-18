#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include "SodaPop.h"
#include "Logo.h"

namespace catui
{
// Palette shared by every cat plugin. Backgrounds stay dark charcoal; only controls and graph lines use the neon soda pop colours (SodaPop.h).
struct Colours
{
    static juce::Colour wood()      { return juce::Colour(0xff5a3a24); }
    static juce::Colour woodDark()  { return juce::Colour(0xff3b2415); }
    static juce::Colour woodLight() { return juce::Colour(0xff7a5236); }
    static juce::Colour panel()     { return juce::Colour(0xff131519); }
    static juce::Colour panelLight(){ return juce::Colour(0xff1e2128); }
    static juce::Colour text()      { return juce::Colour(0xffe8e6e1); }
    static juce::Colour dim()       { return juce::Colour(0xff9a9690); }
    static juce::Colour accent()    { return SodaPop::amber(); }
};

class CatLookAndFeel : public juce::LookAndFeel_V4
{
public:
    CatLookAndFeel();
    void drawRotarySlider(juce::Graphics&, int x, int y, int w, int h, float pos,
                          float startAngle, float endAngle, juce::Slider&) override;
    void drawButtonBackground(juce::Graphics&, juce::Button&, const juce::Colour&,
                              bool highlighted, bool down) override;
};

// Draws the wood side cheeks. Call in paint() before children.
void drawWoodCheeks(juce::Graphics&, juce::Rectangle<int> bounds, int cheekWidth);
}
