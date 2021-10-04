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

//struct for individual filters making them high order by stacking them
struct BetterFilter
{
	juce::dsp::StateVariableTPTFilter<float> Filter1;
    juce::dsp::StateVariableTPTFilter<float> Filter2;
    juce::dsp::StateVariableTPTFilter<float> Filter3;
    juce::dsp::StateVariableTPTFilter<float> Filter4;

	BetterFilter(int type);

	~BetterFilter();

	void setType(int type);
	void prepareFilter(juce::dsp::ProcessSpec spec);
	void resetFilter();
	void setFilterCutoff(float cut, juce::dsp::ProcessContextReplacing<float> context);
};

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

    //the reverb parameters
    juce::dsp::Reverb::Parameters reverbParameters;
    juce::dsp::Reverb leftReverb, rightReverb;

    //the chorus parameters
    juce::dsp::Chorus<float> leftChorus, rightChorus;

    //functions used to create layouts that are passed back to the editor and attched to sliders
    juce::AudioProcessorValueTreeState::ParameterLayout createParameters();

    //virtual function needed to be overriden
    void parameterChanged(const juce::String& parameterID, float newValue) override {};
	
	//independant high order filters
	BetterFilter lowCutFilter;
	BetterFilter highCutFilter;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(RealMagiVerbAudioProcessor);
};
