#include "catui/CatFrame.h"

namespace catui
{
CatHeader::CatHeader(juce::String t) : title(std::move(t))
{
    for (auto* b : { &info, &gear, &reset })
        addAndMakeVisible(*b);
    info.onClick  = [this] { if (onInfo) onInfo(); };
    gear.onClick  = [this] { if (onSettings) onSettings(); };
    reset.onClick = [this] { if (onReset100) onReset100(); };
}

void CatHeader::paint(juce::Graphics& g)
{
    g.fillAll(Colours::panelLight());
    g.setColour(Colours::text());
    g.setFont(juce::FontOptions(16.0f, juce::Font::bold));
    g.drawText(title, getLocalBounds().withTrimmedLeft(12), juce::Justification::centredLeft);
}

void CatHeader::resized()
{
    auto r = getLocalBounds().reduced(4);
    reset.setBounds(r.removeFromRight(52)); r.removeFromRight(4);
    gear.setBounds(r.removeFromRight(52));  r.removeFromRight(4);
    info.setBounds(r.removeFromRight(32));
}

CatFrame::CatFrame(juce::String title, int bw, int bh)
    : baseW(bw), baseH(bh), head(std::move(title))
{
    setLookAndFeel(&laf);
    addAndMakeVisible(head);
    addAndMakeVisible(contentHolder);
    head.onReset100 = [this] { resetTo100(); };
    setSize(bw, bh);
}

CatFrame::~CatFrame() { setLookAndFeel(nullptr); }

void CatFrame::attachTo(juce::ResizableWindow& w)
{
    window = &w;
    w.setResizable(true, true);   // corner + edge resizing
    w.setResizeLimits(baseW / 2, baseH / 2, baseW * 3, baseH * 3);
    if (auto* c = w.getConstrainer())
        c->setFixedAspectRatio((double) baseW / (double) baseH);
}

void CatFrame::resetTo100()
{
    if (window != nullptr) window->setSize(baseW, baseH);
}

void CatFrame::paint(juce::Graphics& g)
{
    g.fillAll(Colours::panel());
    drawWoodCheeks(g, getLocalBounds(), cheek);
}

void CatFrame::resized()
{
    auto r = getLocalBounds().reduced(cheek, 0);
    head.setBounds(r.removeFromTop(34));
    contentHolder.setBounds(r);
    for (auto* c : contentHolder.getChildren())
        c->setBounds(contentHolder.getLocalBounds());
}
}
