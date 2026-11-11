#pragma once

#include "PluginProcessor.h"
#include <cmath>

// Draws each rotary knob as a sushi roll: seaweed ring, rice, colored filling.
class SushiLookAndFeel : public juce::LookAndFeel_V4
{
public:
    explicit SushiLookAndFeel(juce::Colour fillingColour) : filling(fillingColour) {}

    void drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
                          float sliderPos, float startAngle, float endAngle,
                          juce::Slider&) override
    {
        const juce::Rectangle<float> bounds((float) x, (float) y, (float) width, (float) height);
        const float r = juce::jmin(juce::jmin(bounds.getWidth(), bounds.getHeight()) * 0.5f - 6.0f, 52.0f);
        const auto c = bounds.getCentre();

        auto disc = [&](float radius, juce::Colour colour)
        {
            g.setColour(colour);
            g.fillEllipse(c.x - radius, c.y - radius, radius * 2.0f, radius * 2.0f);
        };

        disc(r,         juce::Colour(0xff1c1f1d));   // seaweed
        disc(r * 0.80f, juce::Colour(0xfff1efe8));   // rice
        disc(r * 0.30f, filling);                    // filling
        disc(r * 0.12f, filling.brighter(0.5f));     // highlight

        // The white dot on the seaweed ring shows the knob position
        const float angle = startAngle + sliderPos * (endAngle - startAngle);
        const float ringR = r * 0.90f;
        const float dotR  = r * 0.10f;
        const float dx = c.x + ringR * std::sin(angle);
        const float dy = c.y - ringR * std::cos(angle);

        g.setColour(juce::Colour(0xfff1efe8));
        g.fillEllipse(dx - dotR, dy - dotR, dotR * 2.0f, dotR * 2.0f);
    }

private:
    juce::Colour filling;
};

class ThreeKnobAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    explicit ThreeKnobAudioProcessorEditor(ThreeKnobAudioProcessor&);
    ~ThreeKnobAudioProcessorEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;

    struct Knob
    {
        juce::Slider slider;
        juce::Label label;
        std::unique_ptr<SliderAttachment> attachment;
    };

    void setupKnob(Knob& knob, const juce::String& paramId, const juce::String& text,
                   SushiLookAndFeel& lnf);
    juce::Rectangle<int> platterBounds() const;

    ThreeKnobAudioProcessor& audioProcessor;

    SushiLookAndFeel salmonLnf   { juce::Colour(0xfff09070) };
    SushiLookAndFeel avocadoLnf  { juce::Colour(0xff97c459) };
    SushiLookAndFeel tunaLnf     { juce::Colour(0xffe24b4a) };

    Knob lowCutKnob, midKnob, highCutKnob;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ThreeKnobAudioProcessorEditor)
};
