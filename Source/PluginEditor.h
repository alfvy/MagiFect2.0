#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

const float startAngle = juce::degreesToRadians(180.0f + 60.0f);
const float endAngle = juce::degreesToRadians(180.f - 60.0f) + juce::MathConstants<float>::twoPi;

using uint = unsigned int;

struct LookAndFeel : juce::LookAndFeel_V4
{
    //static void drawRotarySlider(juce::Graphics&, int x, int y, int width, int height,
    //    float sliderPosProportional, float rotaryStartAngle, float rotaryEndAngle, juce::Slider&);

    static void drawRotarySlider(juce::Graphics& g, float rotaryStartAngle, float rotaryEndAngle, juce::Slider& slider);
    static juce::String getDisplayString(juce::Slider& slider);
    void drawTypeText(juce::Graphics& g, juce::Slider& slider);
    juce::Rectangle<int> getSliderBounds(juce::Slider& slider);
};

struct SpinningObject : public juce::Component, public juce::Timer
{
    double rotation = {0.0f};

    SpinningObject(RealMagiVerbAudioProcessor& p);
    ~SpinningObject();

    void timerCallback() override;
    void paint(juce::Graphics&) override;
    void resized() override;
    void drawSpinningObject(juce::Graphics& g, juce::Slider& slider, float factor, juce::Colour colour);
    void drawSpinningObject(juce::Graphics& g, juce::Slider& slider, float factor, uint x, uint y, uint z);

    RealMagiVerbAudioProcessor& audioProcessor;
};

//==============================================================================
class RealMagiVerbAudioProcessorEditor  : public juce::AudioProcessorEditor, juce::Button::Listener
{
public:
    RealMagiVerbAudioProcessorEditor (RealMagiVerbAudioProcessor&);
    ~RealMagiVerbAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

    bool spinStateBool = true;
    int spinInt = 0;

private:

    //object of the spinning object struct to be able to call it in the constructor
    SpinningObject spinningObject;

    //a toggle button for the option to draw the spinning rectangles and dynamic background or not
    juce::ToggleButton spinButton{""};

    //useless virtual function
    void buttonClicked(juce::Button* button) override;

    //on click function, get called only when the button is pressed
    void setSpinState() {
        if (spinInt % 2 == 0)
        {
            spinningObject.stopTimer();
            spinStateBool = false;
            spinInt++;
        }
        else
        {
            spinningObject.startTimerHz(24);
            spinStateBool = true;
            spinInt++;
        }
        repaint();
    }

    //definitions for all the sliders, thier labels and their processor attachments
    juce::Slider revSizeSlider, revDampSlider, revWidthSlider,
        revDryWetSlider, modRateSlider, modAmountSlider,
        lowCutFreqSlider, highCutFreqSlider, preGainSlider,
        postGainSlider, distGainSlider, rateDivSlider, 
        bitDepthSlider, distChoiceSlider, entropySlider;

    juce::Label revSizeLabel, revDampLabel, revWidthLabel,
        revDryWetLabel, modRateLabel, modAmountLabel,
        lowCutFreqLabel, highCutFreqLabel, preGainLabel,
        postGainLabel, distGainLabel, rateDivLabel, 
        bitDepthLabel, entropyLabel;

    std::unique_ptr<juce::SliderParameterAttachment> revSizeSliderAtt, revDampSliderAtt, revWidthSliderAtt,
        revDryWetSliderAtt, modRateSliderAtt, modAmountSliderAtt,
        lowCutFreqSliderAtt, highCutFreqSliderAtt, preGainSliderAtt,
        postGainSliderAtt, distGainSliderAtt, rateDivSliderAtt, 
        bitDepthSliderAtt, distChoiceSliderAtt, entropySlAtt;

    //all the bounds that are used by the sliders and thier labels
    juce::Rectangle<int> revSizeBounds      = { 45, 40, 70, 70 };
    juce::Rectangle<int> revDampBounds      = { 145, 40, 70, 70 };
    juce::Rectangle<int> revWidthBounds     = { 45, 140, 70, 70 };
    juce::Rectangle<int> revDryWetBounds    = { 145, 140, 70, 70 };
    juce::Rectangle<int> modRateBounds      = { 290, 40, 70, 70 };
    juce::Rectangle<int> modAmountBounds    = { 290, 140, 70, 70 };
    juce::Rectangle<int> distGainBounds     = { 45, 280, 70, 70 };
    juce::Rectangle<int> distChoiceBounds   = { 45, 380, 70, 70 };
    juce::Rectangle<int> rateDivBounds      = { 145, 280, 70, 70 };
    juce::Rectangle<int> bitDepthBounds     = { 145, 380, 70, 70 };
    juce::Rectangle<int> lowCutBounds       = { 290, 280, 70, 70 };
    juce::Rectangle<int> highCutBounds      = { 290, 380, 70, 70 };
    juce::Rectangle<int> preGainBounds      = { 45, 480, 70, 70 };
    juce::Rectangle<int> postGainBounds     = { 145, 480, 70, 70 };
    juce::Rectangle<int> entropyBounds      = { 290, 480, 70, 70 };


    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    RealMagiVerbAudioProcessor& audioProcessor;

    void drawLabel(juce::String text, juce::Label& label);

    std::vector<juce::Component*> getComps();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RealMagiVerbAudioProcessorEditor)
};