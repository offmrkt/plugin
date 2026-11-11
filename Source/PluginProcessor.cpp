#include "PluginProcessor.h"
#include "PluginEditor.h"

ThreeKnobAudioProcessor::ThreeKnobAudioProcessor()
    : AudioProcessor(BusesProperties()
          .withInput("Input", juce::AudioChannelSet::stereo(), true)
          .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      apvts(*this, nullptr, "PARAMS", createLayout())
{
    lowCutParam  = apvts.getRawParameterValue("lowcut");
    highCutParam = apvts.getRawParameterValue("highcut");
    midParam     = apvts.getRawParameterValue("mid");
}

juce::AudioProcessorValueTreeState::ParameterLayout
ThreeKnobAudioProcessor::createLayout()
{
    using namespace juce;
    AudioProcessorValueTreeState::ParameterLayout layout;

    auto onOff = [](float v, int) { return v > 0.f ? String("ON") : String("OFF"); };
    auto dbText = [](float v, int) { return String(v, 1) + " dB"; };

    layout.add(std::make_unique<AudioParameterFloat>(
        ParameterID{"lowcut", 1}, "Low",
        NormalisableRange<float>(0.f, 1.f, 0.01f), 0.f,
        AudioParameterFloatAttributes().withStringFromValueFunction(onOff)));

    layout.add(std::make_unique<AudioParameterFloat>(
        ParameterID{"highcut", 1}, "High",
        NormalisableRange<float>(0.f, 1.f, 0.01f), 0.f,
        AudioParameterFloatAttributes().withStringFromValueFunction(onOff)));

    layout.add(std::make_unique<AudioParameterFloat>(
        ParameterID{"mid", 1}, "Mid",
        NormalisableRange<float>(-12.f, 12.f, 0.1f), 0.f,
        AudioParameterFloatAttributes().withStringFromValueFunction(dbText)));

    return layout;
}

bool ThreeKnobAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    const auto out = layouts.getMainOutputChannelSet();
    if (out != juce::AudioChannelSet::mono() && out != juce::AudioChannelSet::stereo())
        return false;
    return out == layouts.getMainInputChannelSet();
}

void ThreeKnobAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    currentSampleRate = sampleRate;

    juce::dsp::ProcessSpec spec{ sampleRate,
                                 (juce::uint32) samplesPerBlock,
                                 (juce::uint32) getTotalNumOutputChannels() };

    for (auto& s : lowCut)  s.prepare(spec);
    for (auto& s : highCut) s.prepare(spec);
    mid.prepare(spec);

    // Fixed cutoffs, designed once
    auto lowCoeffs = juce::dsp::FilterDesign<float>::
        designIIRHighpassHighOrderButterworthMethod(30.f, sampleRate, order);
    for (int i = 0; i < numStages; ++i)
        *lowCut[(size_t) i].state = *lowCoeffs[i];

    const float highFreq = juce::jmin(15000.f, (float) (sampleRate * 0.45));
    auto highCoeffs = juce::dsp::FilterDesign<float>::
        designIIRLowpassHighOrderButterworthMethod(highFreq, sampleRate, order);
    for (int i = 0; i < numStages; ++i)
        *highCut[(size_t) i].state = *highCoeffs[i];

    lastMidDb = 1000.f;
    lowWasOn = highWasOn = false;
}

void ThreeKnobAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer,
                                           juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;

    for (int ch = getTotalNumInputChannels(); ch < getTotalNumOutputChannels(); ++ch)
        buffer.clear(ch, 0, buffer.getNumSamples());

    // Any twist above zero turns the hard cut fully on
    const bool lowOn  = lowCutParam->load()  > 0.f;
    const bool highOn = highCutParam->load() > 0.f;

    // Clear old filter memory when a cut switches on, to avoid a stale burst
    if (lowOn && !lowWasOn)   for (auto& s : lowCut)  s.reset();
    if (highOn && !highWasOn) for (auto& s : highCut) s.reset();
    lowWasOn = lowOn;
    highWasOn = highOn;

    // Mid boost or cut around 1 kHz
    const float midDb = midParam->load();
    if (midDb != lastMidDb)
    {
        *mid.state = *Coeffs::makePeakFilter(currentSampleRate, 1000.f, 0.7f,
                                             juce::Decibels::decibelsToGain(midDb));
        lastMidDb = midDb;
    }

    juce::dsp::AudioBlock<float> block(buffer);
    juce::dsp::ProcessContextReplacing<float> context(block);

    if (lowOn)  for (auto& s : lowCut)  s.process(context);
    if (highOn) for (auto& s : highCut) s.process(context);
    mid.process(context);
}

juce::AudioProcessorEditor* ThreeKnobAudioProcessor::createEditor()
{
    return new ThreeKnobAudioProcessorEditor(*this);
}

void ThreeKnobAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    if (auto xml = apvts.copyState().createXml())
        copyXmlToBinary(*xml, destData);
}

void ThreeKnobAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    if (auto xml = getXmlFromBinary(data, sizeInBytes))
        if (xml->hasTagName(apvts.state.getType()))
            apvts.replaceState(juce::ValueTree::fromXml(*xml));
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new ThreeKnobAudioProcessor();
}
