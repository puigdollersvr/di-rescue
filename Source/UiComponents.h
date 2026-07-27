#pragma once

#include <JuceHeader.h>

namespace ui {

class SegmentedMeter : public juce::Component
{
public:
    SegmentedMeter();

    void setLevelDb(float db);
    void setNumSegments(int num);

    void paint(juce::Graphics& g) override;

private:
    float targetDb = -60.0f;
    float displayedDb = -60.0f;
    int numSegments = 14;
    float smoothing = 0.35f;

    static juce::Colour meterOnColour() { return juce::Colour(0xff39d42f); }
    static juce::Colour meterOffColour() { return juce::Colour(0xff0d0e0f); }
};

class ClipLed : public juce::Component
{
public:
    ClipLed();

    void refresh(bool isClippingNow);
    void reset();

    void paint(juce::Graphics& g) override;

private:
    juce::Time clipEndTime;
    bool wasClipping = false;

    static juce::Colour activeColour() { return juce::Colour(0xffd62b2b); }
    static juce::Colour inactiveColour() { return juce::Colour(0xff0d0e0f); }
    static juce::Colour glowColour() { return juce::Colour(0x60d62b2b); }
};

class InstrumentSelector : public juce::Component
{
public:
    InstrumentSelector();

    std::function<void(int)> onSelectionChanged;

    void setSelectedIndex(int index);
    int getSelectedIndex() const;

    void paint(juce::Graphics& g) override;
    void mouseDown(const juce::MouseEvent& e) override;

private:
    int selectedIndex = 0;
    juce::Rectangle<int> guitarRect;
    juce::Rectangle<int> bassRect;

    void setSelectedItem(int index);
    void drawItem(juce::Graphics& g, const juce::Rectangle<int>& rect,
                  int index, const juce::String& text);

    static juce::Colour textColour() { return juce::Colour(0xffd8d1c4); }
    static juce::Colour accentColour() { return juce::Colour(0xffd7903f); }
    static juce::Colour deepColour() { return juce::Colour(0xff0d0e0f); }
    static juce::Colour panelColour() { return juce::Colour(0xff171819); }
};

class DiRescueLookAndFeel : public juce::LookAndFeel_V4
{
public:
    DiRescueLookAndFeel();

    void drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
                          float sliderPosProportional, float rotaryStartAngle,
                          float rotaryEndAngle, juce::Slider& slider) override;

    void drawToggleButton(juce::Graphics& g, juce::ToggleButton& button,
                          bool shouldDrawButtonAsHighlighted,
                          bool shouldDrawButtonAsDown) override;
};

} // namespace ui
