#include "PluginEditor.h"

ThreeKnobAudioProcessorEditor::ThreeKnobAudioProcessorEditor(ThreeKnobAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p)
{
    setupKnob(lowCutKnob,  "lowcut",  "LOW",  salmonLnf);
    setupKnob(midKnob,     "mid",     "MID",  avocadoLnf);
    setupKnob(highCutKnob, "highcut", "HIGH", tunaLnf);

    setSize(480, 290);
}

ThreeKnobAudioProcessorEditor::~ThreeKnobAudioProcessorEditor()
{
    lowCutKnob.slider.setLookAndFeel(nullptr);
    midKnob.slider.setLookAndFeel(nullptr);
    highCutKnob.slider.setLookAndFeel(nullptr);
}

void ThreeKnobAudioProcessorEditor::setupKnob(Knob& knob,
                                              const juce::String& paramId,
                                              const juce::String& text,
                                              SushiLookAndFeel& lnf)
{
    knob.slider.setLookAndFeel(&lnf);
    knob.slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    knob.slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 90, 22);
    knob.slider.setTextBoxIsEditable(false);
    knob.slider.setColour(juce::Slider::textBoxTextColourId, juce::Colour(0xffe6d3b8));
    knob.slider.setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    knob.slider.setColour(juce::Slider::textBoxBackgroundColourId, juce::Colours::transparentBlack);
    addAndMakeVisible(knob.slider);

    knob.label.setText(text, juce::dontSendNotification);
    knob.label.setJustificationType(juce::Justification::centred);
    knob.label.setColour(juce::Label::textColourId, juce::Colours::white);
    addAndMakeVisible(knob.label);

    knob.attachment = std::make_unique<SliderAttachment>(audioProcessor.apvts, paramId, knob.slider);
}

juce::Rectangle<int> ThreeKnobAudioProcessorEditor::platterBounds() const
{
    return { 20, 42, getWidth() - 40, getHeight() - 85 };
}

void ThreeKnobAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(0xff15171a));

    g.setColour(juce::Colours::white);
    g.setFont(juce::FontOptions(18.0f, juce::Font::bold));
    g.drawText("sushi", getLocalBounds().removeFromTop(40),
               juce::Justification::centred);

    // Wooden platter
    const auto platter = platterBounds().toFloat();
    g.setColour(juce::Colour(0xff4a3426));
    g.fillRoundedRectangle(platter, 10.0f);
    g.setColour(juce::Colour(0xff6b4a35));
    g.drawRoundedRectangle(platter.reduced(1.0f), 10.0f, 2.0f);
    g.setColour(juce::Colour(0xff5c4130));
    g.drawRoundedRectangle(platter.reduced(10.0f), 6.0f, 1.0f);

    g.setColour(juce::Colours::white.withAlpha(0.7f));
    g.setFont(juce::FontOptions(13.0f));
    g.drawText("@sadboisdub", 0, getHeight() - 28, getWidth(), 20,
               juce::Justification::centred);
}

void ThreeKnobAudioProcessorEditor::resized()
{
    auto area = platterBounds().reduced(10);
    const int colWidth = area.getWidth() / 3;

    for (auto* knob : { &lowCutKnob, &midKnob, &highCutKnob })
    {
        auto col = area.removeFromLeft(colWidth);
        knob->label.setBounds(col.removeFromTop(24));
        knob->slider.setBounds(col);
    }
}
