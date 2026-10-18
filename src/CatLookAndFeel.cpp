#include "catui/CatLookAndFeel.h"

namespace catui
{
CatLookAndFeel::CatLookAndFeel()
{
    setColour(juce::ResizableWindow::backgroundColourId, Colours::panel());
    setColour(juce::Label::textColourId, Colours::text());
    setColour(juce::TextButton::buttonColourId, Colours::panelLight());
    setColour(juce::TextButton::textColourOffId, Colours::text());
    setColour(juce::TextButton::buttonOnColourId, SodaPop::violet());   // switched-on buttons are purple
    setColour(juce::TextButton::textColourOnId, juce::Colours::white);
    setColour(juce::Slider::rotarySliderFillColourId, Colours::accent());
    setColour(juce::Slider::rotarySliderOutlineColourId, Colours::woodDark());
}

void CatLookAndFeel::drawRotarySlider(juce::Graphics& g, int x, int y, int w, int h,
                                      float pos, float a0, float a1, juce::Slider&)
{
    auto r = juce::Rectangle<float>((float) x, (float) y, (float) w, (float) h).reduced(4.0f);
    const float d = juce::jmin(r.getWidth(), r.getHeight());
    r = r.withSizeKeepingCentre(d, d);
    const float angle = a0 + pos * (a1 - a0);

    g.setColour(Colours::woodDark());
    g.fillEllipse(r);
    g.setColour(Colours::woodLight().withAlpha(0.5f));
    g.drawEllipse(r, 1.5f);

    juce::Path arc;
    arc.addCentredArc(r.getCentreX(), r.getCentreY(), d * 0.5f - 3, d * 0.5f - 3, 0.0f, a0, angle, true);
    g.setColour(Colours::accent());
    g.strokePath(arc, juce::PathStrokeType(3.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    juce::Path tick;
    tick.addRoundedRectangle(-1.5f, -d * 0.5f + 5, 3.0f, d * 0.22f, 1.0f);
    g.setColour(Colours::text());
    g.fillPath(tick, juce::AffineTransform::rotation(angle).translated(r.getCentre()));
}

void CatLookAndFeel::drawButtonBackground(juce::Graphics& g, juce::Button& b, const juce::Colour& base,
                                          bool highlighted, bool down)
{
    auto r = b.getLocalBounds().toFloat().reduced(0.5f);
    auto c = base;
    if (down) c = c.brighter(0.2f); else if (highlighted) c = c.brighter(0.1f);
    g.setColour(c);
    g.fillRoundedRectangle(r, 4.0f);
    g.setColour(Colours::woodLight().withAlpha(0.6f));
    g.drawRoundedRectangle(r, 4.0f, 1.0f);
}

void drawWoodCheeks(juce::Graphics& g, juce::Rectangle<int> bounds, int cw)
{
    for (int side = 0; side < 2; ++side)
    {
        auto r = side == 0 ? bounds.removeFromLeft(cw) : bounds.removeFromRight(cw);
        g.setGradientFill(juce::ColourGradient(Colours::woodLight(), (float) r.getX(), 0.0f,
                                               Colours::woodDark(), (float) r.getRight(), 0.0f, false));
        g.fillRect(r);
        g.setColour(Colours::woodDark().withAlpha(0.5f));
        for (int y = r.getY() + 5; y < r.getBottom(); y += 9)
            g.drawHorizontalLine(y, (float) r.getX(), (float) r.getRight());
    }
}
}
