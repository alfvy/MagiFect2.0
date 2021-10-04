/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

enum distChoices {
    Clipping,
    Overdrive,
    GuitarAmp,
    ValveSat,
    SoftClip,
    WaveShapper,
    Bypass
};

enum Slope
{
    Slope_12,
    Slope_24,
    Slope_36,
    Slope_48
};

struct ChainSettings
{
    float lowCutFreq{ 0 }, highCutFreq{ 0 };
    Slope lowCutSlope{ Slope::Slope_48 }, highCutSlope{ Slope::Slope_48 };
    //bool lowCutBypassed { false }, highCutBypassed { false };
};

using Filter = juce::dsp::IIR::Filter<float>;

using CutFilter = juce::dsp::ProcessorChain<Filter, Filter, Filter, Filter>;

using Coefficients = Filter::CoefficientsPtr;

//==============================================================================
class RealMagiVerbAudioProcessor  : public juce::AudioProcessor, public juce::AudioProcessorValueTreeState::Listener
{
public:
    //==============================================================================
    RealMagiVerbAudioProcessor();
    ~RealMagiVerbAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;
    void reset() override;

    juce::AudioProcessorValueTreeState apvts;

private:
    
    //random number generator
    juce::Random random;

    //filter params
    using Filter = juce::dsp::IIR::Filter<float>;
    using MonoChain = juce::dsp::ProcessorChain<Filter, Filter>;

    MonoChain leftChain, rightChain;

    //the reverb parameters
    juce::dsp::Reverb::Parameters reverbParameters;
    juce::dsp::Reverb leftReverb, rightReverb;

    //the chorus parameters
    juce::dsp::Chorus<float> leftChorus, rightChorus;

    //functions used to create layouts that are passed back to the editor and attched to sliders
    juce::AudioProcessorValueTreeState::ParameterLayout createParameters();

    //virtual function needed to be overriden
    void parameterChanged(const juce::String& parameterID, float newValue) override;

    enum ChainPositions
    {
        LowCut,
        HighCut
    };
    
    ChainSettings getChainSettings(juce::AudioProcessorValueTreeState& apvts);

    void updateLowCutFilter(const ChainSettings& chainSettings);
    void updateHighCutFilter(const ChainSettings& chainSettings);
    void updateFilters();

    //void updateLowCutFilter(const ChainSettings& chainSettings);
    //void updateHighCutFilter(const ChainSettings& chainSettings);

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(RealMagiVerbAudioProcessor);
};
