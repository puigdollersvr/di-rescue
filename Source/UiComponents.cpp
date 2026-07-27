#include "UiComponents.h"

namespace ui {

SegmentedMeter::SegmentedMeter()
{
    setBufferedToImage(false);
}

void SegmentedMeter::setLevelDb(float db)
{
    targetDb = juce::jlimit(-60.0f, 0.0f, db);
    repaint();
}

void SegmentedMeter::setNumSegments(int num)
{
    numSegments = juce::jlimit(1, 64, num);
    repaint();
}

void SegmentedMeter::paint(juce::Graphics& g)
{
    displayedDb += (targetDb - displayedDb) * smoothing;

    auto bounds = getLocalBounds().toFloat().reduced(2.0f);
    const float segHeight = bounds.getHeight() / static_cast<float>(numSegments);
    const float gap = 1.5f;
    const float x = bounds.getX();
    const float width = bounds.getWidth();
    const float yTop = bounds.getY();

    for (int i = 0; i < numSegments; ++i)
    {
        const float segY = yTop + static_cast<float>(i) * segHeight;
        const float thresholdDb = juce::jmap(static_cast<float>(i), 0.0f, static_cast<float>(numSegments - 1), 0.0f, -60.0f);
        const bool isOn = displayedDb >= thresholdDb;

        juce::Rectangle<float> seg(x + 1.0f, segY + gap / 2.0f, width - 2.0f, segHeight - gap);
        g.setColour(isOn ? meterOnColour() : meterOffColour());
        g.fillRoundedRectangle(seg, 3.0f);
    }
}

ClipLed::ClipLed()
{
    setBufferedToImage(false);
    reset();
}

void ClipLed::refresh(bool isClippingNow)
{
    if (isClippingNow)
    {
        clipEndTime = juce::Time::getCurrentTime();
        clipEndTime += juce::RelativeTime(1.0); // 1 second hold
        wasClipping = true;
    }

    if (wasClipping && juce::Time::getCurrentTime() > clipEndTime)
        wasClipping = false;

    repaint();
}

void ClipLed::reset()
{
    clipEndTime = juce::Time();
    wasClipping = false;
    repaint();
}

void ClipLed::paint(juce::Graphics& g)
{
    const bool on = wasClipping;
    auto bounds = getLocalBounds().toFloat().reduced(2.0f);

    g.setColour(on ? activeColour() : inactiveColour());
    g.fillEllipse(bounds);

    if (on)
    {
        g.setColour(glowColour());
        g.fillEllipse(bounds.expanded(2.0f));
    }

    g.setColour(juce::Colour(0x20ffffff));
    g.drawEllipse(bounds, 1.0f);
}

InstrumentSelector::InstrumentSelector()
{
    setBufferedToImage(false);
}

void InstrumentSelector::setSelectedIndex(int index)
{
    setSelectedItem(index);
}

int InstrumentSelector::getSelectedIndex() const
{
    return selectedIndex;
}

void InstrumentSelector::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().reduced(2);
    const int itemH = bounds.getHeight() / 2;
    guitarRect = bounds.removeFromTop(itemH);
    bassRect = bounds;

    drawItem(g, guitarRect, 0, "GUITAR");
    drawItem(g, bassRect, 1, "BASS");
}

void InstrumentSelector::drawItem(juce::Graphics& g, const juce::Rectangle<int>& rect,
                                  int index, const juce::String& text)
{
    const bool selected = (index == selectedIndex);
    const auto textCol = selected ? accentColour() : textColour();

    g.setColour(panelColour());
    g.fillRoundedRectangle(rect.toFloat().reduced(2.0f), 6.0f);

    if (selected)
    {
        g.setColour(accentColour());
        g.drawRoundedRectangle(rect.toFloat().reduced(2.0f), 6.0f, 1.5f);
    }

    const int dotSize = 8;
    const auto textArea = rect.reduced(24, 0);
    const float dotX = static_cast<float>(textArea.getX());
    const float dotY = static_cast<float>(rect.getCentreY()) - dotSize / 2.0f;
    g.setColour(selected ? accentColour() : deepColour());
    g.fillEllipse(juce::Rectangle<float>(dotX, dotY, dotSize, dotSize));

    g.setColour(textCol);
    g.setFont(juce::FontOptions(14.0f, juce::Font::bold));
    g.drawText(text, textArea.withX(textArea.getX() + 18), juce::Justification::centredLeft);
}

void InstrumentSelector::mouseDown(const juce::MouseEvent& e)
{
    if (guitarRect.contains(e.getPosition()))
        setSelectedItem(0);
    else if (bassRect.contains(e.getPosition()))
        setSelectedItem(1);
}

void InstrumentSelector::setSelectedItem(int index)
{
    index = juce::jlimit(0, 1, index);
    if (index == selectedIndex)
        return;

    selectedIndex = index;
    repaint();

    if (onSelectionChanged)
        onSelectionChanged(selectedIndex);
}

DiRescueLookAndFeel::DiRescueLookAndFeel()
{
    setColour(juce::Slider::thumbColourId, juce::Colour(0xffd7903f));
    setColour(juce::Slider::rotarySliderFillColourId, juce::Colour(0xffd7903f));
    setColour(juce::Slider::rotarySliderOutlineColourId, juce::Colour(0xff0d0e0f));
    setColour(juce::ToggleButton::tickColourId, juce::Colour(0xffd7903f));
    setColour(juce::ToggleButton::tickDisabledColourId, juce::Colour(0xff3a3a3a));
}

void DiRescueLookAndFeel::drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
                                           float sliderPosProportional, float rotaryStartAngle,
                                           float rotaryEndAngle, juce::Slider&)
{
    constexpr float halfPi = juce::MathConstants<float>::pi * 0.5f;

    const float radius = juce::jmin(static_cast<float>(width), static_cast<float>(height)) * 0.5f - 16.0f;
    const float centreX = static_cast<float>(x) + static_cast<float>(width) * 0.5f;
    const float centreY = static_cast<float>(y) + static_cast<float>(height) * 0.5f;
    const float angle = rotaryStartAngle + sliderPosProportional * (rotaryEndAngle - rotaryStartAngle);
    const float outerR = radius + 10.0f;

    // Outer dark ring
    g.setColour(juce::Colour(0xff0d0e0f));
    g.fillEllipse(centreX - outerR, centreY - outerR, outerR * 2.0f, outerR * 2.0f);

    // Tick arc
    const int numTicks = 48;
    for (int i = 0; i <= numTicks; ++i)
    {
        const float t = static_cast<float>(i) / static_cast<float>(numTicks);
        const float a = rotaryStartAngle + t * (rotaryEndAngle - rotaryStartAngle);
        const float x1 = centreX + std::cos(a - halfPi) * (outerR - 5.0f);
        const float y1 = centreY + std::sin(a - halfPi) * (outerR - 5.0f);
        const float x2 = centreX + std::cos(a - halfPi) * outerR;
        const float y2 = centreY + std::sin(a - halfPi) * outerR;
        const bool active = t <= sliderPosProportional;
        g.setColour(active ? juce::Colour(0xffd7903f) : juce::Colour(0xff2a2b2c));
        g.drawLine(x1, y1, x2, y2, active ? 2.0f : 1.0f);
    }

    // Inner knob face
    const float innerR = radius - 8.0f;
    g.setColour(juce::Colour(0xff171819));
    g.fillEllipse(centreX - innerR, centreY - innerR, innerR * 2.0f, innerR * 2.0f);

    g.setColour(juce::Colour(0x15ffffff));
    g.drawEllipse(centreX - innerR, centreY - innerR, innerR * 2.0f, innerR * 2.0f, 1.0f);

    // Thumb
    const float thumbR = innerR * 0.75f;
    const float thumbX = centreX + std::cos(angle - halfPi) * thumbR;
    const float thumbY = centreY + std::sin(angle - halfPi) * thumbR;
    g.setColour(juce::Colour(0xffd7903f));
    g.drawLine(centreX, centreY, thumbX, thumbY, 3.0f);
}

void DiRescueLookAndFeel::drawToggleButton(juce::Graphics& g, juce::ToggleButton& button,
                                           bool, bool)
{
    auto bounds = button.getLocalBounds().toFloat().reduced(2.0f);
    const bool isOn = button.getToggleState();
    const float radius = bounds.getHeight() * 0.5f;

    g.setColour(juce::Colour(0xff0d0e0f));
    g.fillRoundedRectangle(bounds, radius);

    if (isOn)
    {
        juce::Path fill;
        fill.addRoundedRectangle(bounds.reduced(2.0f), radius);
        g.setColour(juce::Colour(0xffd7903f));
        g.fillPath(fill);
    }

    const float knobSize = bounds.getHeight() - 8.0f;
    const float knobY = bounds.getCentreY() - knobSize * 0.5f;
    const float knobX = isOn ? (bounds.getRight() - knobSize - 4.0f) : (bounds.getX() + 4.0f);
    g.setColour(juce::Colour(0xffd8d1c4));
    g.fillEllipse(knobX, knobY, knobSize, knobSize);
    g.setColour(juce::Colour(0x40ffffff));
    g.drawEllipse(knobX, knobY, knobSize, knobSize, 1.0f);
}

} // namespace ui
