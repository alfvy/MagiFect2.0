#include "PluginProcessor.h"
#include "PluginEditor.h"

//getting current bounts of the slider
juce::Rectangle<int> LookAndFeel::getSliderBounds(juce::Slider& slider)
{
    //return getLocalBounds();

    auto bounds = slider.getLocalBounds();

    auto size = juce::jmin(bounds.getWidth(), bounds.getHeight());

    size -= 14 * 2;

    juce::Rectangle<int> r;

    r.setSize(size, size);
    r.setCentre(bounds.getCentreX(), 0);
    r.setY(2);

    return r;
}

//function for getting the current info of the slider/parameter
juce::String LookAndFeel::getDisplayString(juce::Slider& slider)
{
    //boolean to check if the slider is the distortion type slider
    juce::Range<double> typeRange = { 0 , 6 };
    bool checkIfDistTypeSlider = slider.getRange() == typeRange;

    juce::String str;
    bool addK = false;

    float val = slider.getValue();

    //divide the number if its greater than a 1000
    //and turn the addK value true to add the thousand at the end or not
    if (val > 999.9f)
    {
        val /= 1000.0f;
        addK = true;
    }

    switch (checkIfDistTypeSlider)
    {
        case true:
        {
            //str = juce::String(val + 1, (addK ? 2 : 0));
            switch ((int)(slider.getValue()))
            {
                case 0:
                {
                    str = "Hard Clip";
                    break;
                }
                case 1:
                {
                    str = "Soft Clip";
                    break;
                }
                case 2:
                {
                    str = "Overdrive";
                    break;
                }
                case 3:
                {
                    str = "AMP";
                    break;
                }
                case 4:
                {
                    str = "Saturation";
                    break;
                }
                case 5:
                {
                    str = "Wave Shaper";
                    break;
                }
                case 6:
                {
                    str = "Bypass";
                    break;
                }
            }
            break;
        }
    
        case false:
        {
            str = juce::String(val, (addK ? 2 : 0));

            if (addK)
            {
                str << "k";
            }
            break;
        }
    }

    return str;
}

//function for drawing the custom rotary sliders
void LookAndFeel::drawRotarySlider(juce::Graphics& g, float rotaryStartAngle, float rotaryEndAngle, juce::Slider& slider)
{
    //check if the slider is the distortion type slider
    juce::Range<double> typeRange = { 0, 6 };
    bool checkIfDistTypeSlider = slider.getRange() == typeRange;

    //turn the slider that juce draws invisible
    slider.setAlpha(0.f);

    //turn off the value texts
    slider.setSliderStyle(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag);
    slider.setTextBoxStyle(juce::Slider::NoTextBox, true, 0, 0);

    //set the bounds of the slider
    auto x = slider.getX();
    auto y = slider.getY();
    auto width = slider.getWidth();
    auto height = slider.getHeight();

    auto bounds = juce::Rectangle<float>(x, y, width, height);

    //set the color of the inside of the knob
    g.setColour(juce::Colour(80u, 80u, 80u));
    g.fillEllipse(bounds);

    //uint knobColor = juce::jlimit(0, 255, (int)(slider.getValue() * 2.55));
    uint knobColor = 255;
    g.setColour(juce::Colour(knobColor, knobColor, knobColor));
    g.drawEllipse(bounds, 3.f);

    if (auto* rswl = dynamic_cast<juce::Slider*>(&slider))
    {
        //setting the bounds of the desired path
        auto center = bounds.getCentre();

        juce::Path p;

        juce::Rectangle<float> r;

        r.setLeft(center.getX() - 2);
        r.setRight(center.getX() + 2);
        r.setTop(bounds.getY() + 5);
        r.setBottom(center.getY());// -testSlider.getTextBoxHeight() * 1.25f);
        r.setSize(7, 7);

        p.addEllipse(r);

        //getting the rotation from the slider
        float sliderPosProportional = juce::jmap(slider.getValue(), 
            slider.getRange().getStart(), slider.getRange().getEnd(), 0.0, 1.0);

        jassert(startAngle < endAngle);

        auto sliderAngleRad = juce::jmap(sliderPosProportional, 0.0f, 1.0f, startAngle, endAngle);

        //applying the rotation
        p.applyTransform(juce::AffineTransform().rotated(sliderAngleRad, center.getX(), center.getY()));

        g.fillPath(p);

        g.setFont(14);

        auto text = getDisplayString(slider);
        auto strWidth = g.getCurrentFont().getStringWidth(text);

        juce::Point<float> offset = { 0.0f, 20.0f };

        //change the location of the text to be drawn based on the type of slider
        switch(checkIfDistTypeSlider)
        {
            case true:
            {
                r.setSize(strWidth + 20, 14 + 2);
                r.setCentre(bounds.getCentre() + offset);
                r.setY(r.getY() + 25);

                g.setColour(juce::Colours::white);
                g.drawFittedText(text, r.toNearestInt(), juce::Justification::centred, 1);
                break;
            }
            case false:
            {
                r.setSize(100, 14 + 2);
                r.setCentre(bounds.getCentre() + offset);
                r.setY(r.getY() + 4);

                g.setColour(juce::Colours::white);
                g.drawFittedText(text, r.toNearestInt(), juce::Justification::centred, 1);
                break;
            }
        }
    }
}

//useless function
void LookAndFeel::drawTypeText(juce::Graphics& g, juce::Slider& slider)
{
    juce::Range<double> bruh = { 0, 6 };

    auto text = LookAndFeel::getDisplayString(slider);
    auto strWidth = g.getCurrentFont().getStringWidth(text);

    juce::Point<int> offset = { 0, 20 };

    juce::Rectangle<int> r, bounds = { 45, 405, 70, 70 };

    r.setSize(strWidth + 20, 14 + 2);
    r.setCentre(bounds.getCentre() + offset);

    g.drawRect(r);
    g.drawRect(bounds);

    g.setColour(juce::Colours::white);
    g.drawText(text, r.toNearestInt(), juce::Justification::centred, 1);
}

SpinningObject::SpinningObject(RealMagiVerbAudioProcessor& p) : audioProcessor(p)
{
    startTimerHz(24);
}

SpinningObject::~SpinningObject()
{
    stopTimer();
}

void SpinningObject::timerCallback()
{
    this->rotation++;
    repaint();
}

void SpinningObject::paint(juce::Graphics& g)
{
    
}

//draws the spinning rounded rectangles, seen on size, rate and bit depth
void SpinningObject::drawSpinningObject(juce::Graphics& g, juce::Slider& slider, float factor, juce::Colour colour)
{
    g.setColour(colour);

    juce::Path path;

    auto bounds = slider.getBounds();

    path.addRoundedRectangle(bounds.reduced(20.0f), 6.0f);

    float actualRotation = (((slider.getValue() * rotation * factor) / 33) * 45) / 180;

    auto transform = juce::AffineTransform().rotated(juce::degreesToRadians(actualRotation),
        bounds.reduced(20.0f).getCentreX(), bounds.reduced(20.0f).getCentreY());

    auto strokeType = juce::PathStrokeType(1.0f);

    path.applyTransform(transform);

    g.fillPath(path);
    path.clear();
}

//override of the above function, but using RGB values
void SpinningObject::drawSpinningObject(juce::Graphics& g, juce::Slider& slider, float factor, uint x, uint y, uint z)
{
    g.setColour(juce::Colour(x, y, z));

    juce::Path path;

    auto bounds = slider.getBounds();

    path.addRoundedRectangle(bounds.reduced(20.0f), 6.0f);

    float actualRotation = (((slider.getValue() * rotation * factor) / 33) * 45) / 180;

    auto transform = juce::AffineTransform().rotated(juce::degreesToRadians(actualRotation),
        bounds.reduced(20.0f).getCentreX(), bounds.reduced(20.0f).getCentreY());

    auto strokeType = juce::PathStrokeType(1.0f);

    path.applyTransform(transform);

    g.fillPath(path);
    path.clear();
}

void SpinningObject::resized()
{

}

//debug function
void drawElipse(juce::Graphics& g, juce::Slider& slider)
{
    auto bounds = slider.getBounds().toFloat();

    g.drawEllipse(bounds.reduced(15), 2);
}

//used to set the bounds of the label using the existing bounds of the slider
void setLabelBounds(juce::Rectangle<int> bounds, juce::Label& label)
{
    bounds.setY(bounds.getY() + 45);
    bounds.setX(bounds.getX() - 3);
    bounds.setWidth(bounds.getWidth() + label.getText().length());
    label.setBounds(bounds);
}

//used to draw the actual text
void RealMagiVerbAudioProcessorEditor::drawLabel(juce::String text, juce::Label& label)
{
    label.setText(text, juce::NotificationType::dontSendNotification);
    label.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(label);
}
    
//==============================================================================
RealMagiVerbAudioProcessorEditor::RealMagiVerbAudioProcessorEditor (RealMagiVerbAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p), spinningObject(audioProcessor)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be

    addAndMakeVisible(spinningObject);

    spinButton.setToggleState(true, juce::dontSendNotification);
    spinButton.onClick = [this]() { setSpinState(); };
    addAndMakeVisible(spinButton);

    //really wish I could somehow call these automatically
    drawLabel("Size", revSizeLabel);
    drawLabel("Damp", revDampLabel);
    drawLabel("Width", revWidthLabel);
    drawLabel("Dry/Wet", revDryWetLabel);
    drawLabel("Rate", modRateLabel);
    drawLabel("Amount", modAmountLabel);
    drawLabel("Low Cut Freq.", lowCutFreqLabel);
    drawLabel("High Cut Freq.", highCutFreqLabel);
    drawLabel("Pre-Gain", preGainLabel);
    drawLabel("Post-Gain", postGainLabel);
    drawLabel("Gain", distGainLabel);
    drawLabel("Bit Rate", rateDivLabel);
    drawLabel("Bit Depth", bitDepthLabel);
    drawLabel("Entropy", entropyLabel);

    //making every component visible
    for (auto* comp : getComps())
    {
        addAndMakeVisible(comp);
    }

    //these too, since I have to call each parameter with its specific name definied in the plugin processor layout function
    using Attachment = juce::SliderParameterAttachment;
    auto& apvts = audioProcessor.apvts;
    {
        revSizeSliderAtt        = std::make_unique<Attachment>(*apvts.getParameter("Reverb Size"), revSizeSlider);
        revDampSliderAtt        = std::make_unique<Attachment>(*apvts.getParameter("Reverb Damping"), revDampSlider);
        revWidthSliderAtt       = std::make_unique<Attachment>(*apvts.getParameter("Reverb Width"), revWidthSlider);
        revDryWetSliderAtt      = std::make_unique<Attachment>(*apvts.getParameter("Reverb Dry/Wet"), revDryWetSlider);
        modRateSliderAtt        = std::make_unique<Attachment>(*apvts.getParameter("Modulation Rate"), modRateSlider);
        modAmountSliderAtt      = std::make_unique<Attachment>(*apvts.getParameter("Modulation Amount"), modAmountSlider);
        distGainSliderAtt       = std::make_unique<Attachment>(*apvts.getParameter("Distortion Gain"), distGainSlider);
        distChoiceSliderAtt     = std::make_unique<Attachment>(*apvts.getParameter("Distortion Type"), distChoiceSlider);
        rateDivSliderAtt        = std::make_unique<Attachment>(*apvts.getParameter("Rate Divide"), rateDivSlider);
        bitDepthSliderAtt       = std::make_unique<Attachment>(*apvts.getParameter("Bit Depth"), bitDepthSlider);
        lowCutFreqSliderAtt     = std::make_unique<Attachment>(*apvts.getParameter("LowCut Frequency"), lowCutFreqSlider);
        highCutFreqSliderAtt    = std::make_unique<Attachment>(*apvts.getParameter("HighCut Frequency"), highCutFreqSlider);
        preGainSliderAtt        = std::make_unique<Attachment>(*apvts.getParameter("Pre-Gain"), preGainSlider);
        postGainSliderAtt       = std::make_unique<Attachment>(*apvts.getParameter("Post-Gain"), postGainSlider);
        entropySlAtt            = std::make_unique<Attachment>(*apvts.getParameter("Entropy"), entropySlider);
    }

    setResizable(false, false);

    setSize (400, 570);
}

RealMagiVerbAudioProcessorEditor::~RealMagiVerbAudioProcessorEditor()
{
}

//==============================================================================
void RealMagiVerbAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour

    int entropyGrad = entropySlider.getValue() * 0.3;

    juce::Colour topColour;
    juce::Colour bottomColour;

    if (spinStateBool == true)
    {
        topColour = juce::Colour(90 - entropyGrad, 90 - entropyGrad, 90 - entropyGrad);
        bottomColour = juce::Colour(50 - entropyGrad, 50 - entropyGrad, 50 - entropyGrad);
    }
    else
    {
        topColour = juce::Colour(90 , 90 , 90);
        bottomColour = juce::Colour(50 , 50 , 50);
    }

    juce::ColourGradient gradient = juce::ColourGradient::vertical(topColour, bottomColour, getBounds());
    g.setGradientFill(gradient);
    g.fillAll();

    //the section rectangles
    {
        juce::Path reverbRectPath, modRectPath, distRectPath, filterRectPath;

        juce::Rectangle<float> reverbRect   = { 30, 30, 200, 200 };
        juce::Rectangle<float> modRect      = { 270, 30, 110, 200 };
        juce::Rectangle<float> distRect     = { 30, 270, 200, 200 };
        juce::Rectangle<float> filterRect   = { 270, 270, 110, 200 };

        g.setColour(juce::Colours::white);
        g.drawRoundedRectangle(reverbRect, 8.0f, 4.0f);
        reverbRectPath.addRoundedRectangle(reverbRect.reduced(2), 8.0);
        g.setColour(juce::Colour(140u, 140u, 140u));
        g.fillPath(reverbRectPath);

        g.setColour(juce::Colours::white);
        g.drawRoundedRectangle(modRect, 8.0f, 4.0f);
        modRectPath.addRoundedRectangle(modRect.reduced(2), 8.0);
        g.setColour(juce::Colour(122u, 122u, 122u));
        g.fillPath(modRectPath);

        g.setColour(juce::Colours::white);
        g.drawRoundedRectangle(distRect, 8.0f, 4.0f);
        distRectPath.addRoundedRectangle(distRect.reduced(2), 8.0);
        g.setColour(juce::Colour(99u, 99u, 99u));
        g.fillPath(distRectPath);

        g.setColour(juce::Colours::white);
        g.drawRoundedRectangle(filterRect, 8.0f, 4.0f);
        filterRectPath.addRoundedRectangle(filterRect.reduced(2), 8.0);
        g.setColour(juce::Colour(80u, 80u, 80u));
        g.fillPath(filterRectPath);
    }
    
    //more manual code because I still can't use god damn templates
    {
        LookAndFeel::drawRotarySlider(g, startAngle, endAngle, revSizeSlider);

        LookAndFeel::drawRotarySlider(g, startAngle, endAngle, revDampSlider);

        LookAndFeel::drawRotarySlider(g, startAngle, endAngle, revWidthSlider);

        LookAndFeel::drawRotarySlider(g, startAngle, endAngle, revDryWetSlider);

        LookAndFeel::drawRotarySlider(g, startAngle, endAngle, modRateSlider);

        LookAndFeel::drawRotarySlider(g, startAngle, endAngle, modAmountSlider);

        LookAndFeel::drawRotarySlider(g, startAngle, endAngle, lowCutFreqSlider);

        LookAndFeel::drawRotarySlider(g, startAngle, endAngle, highCutFreqSlider);

        LookAndFeel::drawRotarySlider(g, startAngle, endAngle, distGainSlider);

        LookAndFeel::drawRotarySlider(g, startAngle, endAngle, distChoiceSlider);

        LookAndFeel::drawRotarySlider(g, startAngle, endAngle, rateDivSlider);

        LookAndFeel::drawRotarySlider(g, startAngle, endAngle, bitDepthSlider);

        LookAndFeel::drawRotarySlider(g, startAngle, endAngle, preGainSlider);

        LookAndFeel::drawRotarySlider(g, startAngle, endAngle, postGainSlider);

        LookAndFeel::drawRotarySlider(g, startAngle, endAngle, entropySlider);

        if (spinStateBool == true)
        {
            spinningObject.drawSpinningObject(g, revSizeSlider, 20.0f, 127u, 127u, 127u);
            spinningObject.drawSpinningObject(g, modRateSlider, 20.0f, 255, 255, 255);
            spinningObject.drawSpinningObject(g, bitDepthSlider, 50.f, juce::Colours::black);
        }
    }

    //big text headers, because DUHH
    {
        juce::Rectangle<float> reverbText   = {70, -3, 110, 35};
        juce::Rectangle<float> modText      = {265, -3, 120, 35};
        juce::Rectangle<float> distText     = {50, 235, 170, 35};
        juce::Rectangle<float> filterText   = {275, 235, 100, 35};

        g.setFont(juce::Font::italic);
        g.setFont(30);
        g.setColour(juce::Colours::white);
        g.drawText("REVERB", reverbText, juce::Justification::centred, false);
        g.setColour(juce::Colours::white);
        g.drawText("MOD", modText, juce::Justification::centred, false);
        g.setColour(juce::Colours::white);
        g.drawText("DISTORTION", distText, juce::Justification::centred, false);
        g.setColour(juce::Colours::white);
        g.drawText("FILTER", filterText, juce::Justification::centred, false);
    }
}

void RealMagiVerbAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
    spinningObject.setBounds(getBounds());

    revSizeSlider.setBounds(revSizeBounds);
    revDampSlider.setBounds(revDampBounds);
    revWidthSlider.setBounds(revWidthBounds);
    revDryWetSlider.setBounds(revDryWetBounds);
    modRateSlider.setBounds(modRateBounds);
    modAmountSlider.setBounds(modAmountBounds);
    lowCutFreqSlider.setBounds(lowCutBounds);
    highCutFreqSlider.setBounds(highCutBounds);
    preGainSlider.setBounds(preGainBounds);
    postGainSlider.setBounds(postGainBounds);
    distGainSlider.setBounds(distGainBounds);
    bitDepthSlider.setBounds(bitDepthBounds);
    distChoiceSlider.setBounds(distChoiceBounds);
    rateDivSlider.setBounds(rateDivBounds);
    entropySlider.setBounds(entropyBounds);

    setLabelBounds(revSizeBounds, revSizeLabel);
    setLabelBounds(revDampBounds, revDampLabel);
    setLabelBounds(revWidthBounds, revWidthLabel);
    setLabelBounds(revDryWetBounds, revDryWetLabel);
    setLabelBounds(modRateBounds, modRateLabel);
    setLabelBounds(modAmountBounds, modAmountLabel);
    setLabelBounds(lowCutBounds, lowCutFreqLabel);
    setLabelBounds(highCutBounds, highCutFreqLabel);
    setLabelBounds(preGainBounds, preGainLabel);
    setLabelBounds(postGainBounds, postGainLabel);
    setLabelBounds(distGainBounds, distGainLabel);
    setLabelBounds(bitDepthBounds, bitDepthLabel);
    setLabelBounds(rateDivBounds, rateDivLabel);
    setLabelBounds(entropyBounds, entropyLabel);

    spinButton.setBounds(5, 0, 35, 35);
}

std::vector<juce::Component*> RealMagiVerbAudioProcessorEditor::getComps()
{
    return
    {
        &revSizeSlider, &revDampSlider, &revWidthSlider,
        &revDryWetSlider, &modRateSlider, &modAmountSlider,
        &lowCutFreqSlider, &highCutFreqSlider, &preGainSlider,
        &postGainSlider, &distGainSlider, &rateDivSlider,
        &bitDepthSlider, &distChoiceSlider, &entropySlider
    };
}

void RealMagiVerbAudioProcessorEditor::buttonClicked(juce::Button* button)
{

}
