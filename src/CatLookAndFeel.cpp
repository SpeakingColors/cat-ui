#include "catui/CatLookAndFeel.h"

namespace catui
{
CatLookAndFeel::CatLookAndFeel()
{
    setColour(juce::ResizableWindow::backgroundColourId, Colours::panel());
    setColour(juce::Label::textColourId, Colours::text());
    setColour(juce::TextButton::buttonColourId, Colours::panelLight());
    setColour(juce::TextButton::textColourOffId, Colours::text());
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

void drawWood(juce::Graphics& g, juce::Rectangle<int> r)
{
    // The PurrOne wood: dark to warm to mid brown across the width, plus fixed-seed vertical grain (no shimmer on repaint).
    juce::ColourGradient cg(juce::Colour(0xff3d2210), (float) r.getX(), 0.0f, juce::Colour(0xff3d2210), (float) r.getRight(), 0.0f, false);
    cg.addColour(0.4, juce::Colour(0xff7b4a26)); cg.addColour(0.75, juce::Colour(0xff5a3419));
    g.setGradientFill(cg); g.fillRect(r);
    juce::Random rnd(7);
    for (int x = r.getX(); x < r.getRight(); x += 2)
    {
        g.setColour(juce::Colour::fromFloatRGBA(0.f, 0.f, 0.f, 0.05f + 0.20f * rnd.nextFloat()));
        const float wob = rnd.nextFloat() * 2.0f;
        g.drawLine((float) x + wob, (float) r.getY(), (float) x - wob, (float) r.getBottom(), 1.0f);
    }
    g.setColour(juce::Colours::black.withAlpha(0.45f)); g.drawRect(r, 1);
}

void drawWoodCheeks(juce::Graphics& g, juce::Rectangle<int> bounds, int cw)
{
    drawWood(g, bounds.removeFromLeft(cw));
    drawWood(g, bounds.removeFromRight(cw));
}
}
