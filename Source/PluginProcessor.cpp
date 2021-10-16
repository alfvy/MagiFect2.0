#include "PluginProcessor.h"
#include "PluginEditor.h"

const float PI = 3.1415926535f;
const float oneOverSQ2 = 1 / sqrt(2);

//==============================================================================
RealMagiVerbAudioProcessor::RealMagiVerbAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       ), apvts(*this, nullptr, "Parameters", createParameters())
                       , lowCutFilter(1), highCutFilter(2)
#endif
{
    apvts.addParameterListener("Reverb Size", this);
    apvts.addParameterListener("Reverb Damping", this);
    apvts.addParameterListener("Reverb Width", this);
    apvts.addParameterListener("Reverb Dry/Wet", this);
    apvts.addParameterListener("Modulation Rate", this);
    apvts.addParameterListener("Modulation Amount", this);
    apvts.addParameterListener("Distortion Gain", this);
    apvts.addParameterListener("Distortion Type", this);
    apvts.addParameterListener("Rate Divide", this);
    apvts.addParameterListener("Bit Depth", this);
    apvts.addParameterListener("Pre-Gain", this);
    apvts.addParameterListener("Post-Gain", this);
}

RealMagiVerbAudioProcessor::~RealMagiVerbAudioProcessor()
{
    apvts.removeParameterListener("Reverb Size", this);
    apvts.removeParameterListener("Reverb Damping", this);
    apvts.removeParameterListener("Reverb Width", this);
    apvts.removeParameterListener("Reverb Dry/Wet", this);
    apvts.removeParameterListener("Modulation Rate", this);
    apvts.removeParameterListener("Modulation Amount", this);
    apvts.removeParameterListener("Distortion Gain", this);
    apvts.removeParameterListener("Distortion Choice", this);
    apvts.removeParameterListener("Rate Divide", this);
    apvts.removeParameterListener("Bit Depth", this);
    apvts.removeParameterListener("Pre-Gain", this);
    apvts.removeParameterListener("Post-Gain", this);
}

//==============================================================================
const juce::String RealMagiVerbAudioProcessor::getName() const
{
    //return JucePlugin_Name;
    return "MagiFect";
}

bool RealMagiVerbAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool RealMagiVerbAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool RealMagiVerbAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double RealMagiVerbAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int RealMagiVerbAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int RealMagiVerbAudioProcessor::getCurrentProgram()
{
    return 0;
}

void RealMagiVerbAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String RealMagiVerbAudioProcessor::getProgramName (int index)
{
    return {};
}

void RealMagiVerbAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void RealMagiVerbAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
    juce::dsp::ProcessSpec spec;
    juce::dsp::ProcessSpec filterSpec;

    spec.sampleRate         = sampleRate;
    spec.maximumBlockSize   = samplesPerBlock;
    spec.numChannels        = 1;

    filterSpec.sampleRate       = sampleRate;
    filterSpec.maximumBlockSize = samplesPerBlock;
    filterSpec.numChannels      = 2;

    leftReverb.prepare(spec);
    rightReverb.prepare(spec);
    leftChorus.prepare(spec);
    rightChorus.prepare(spec);  

    lowCutFilter.prepare(filterSpec);
    highCutFilter.prepare(filterSpec);
}

void RealMagiVerbAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool RealMagiVerbAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

//enhanced modulation function
float mod(float n, float d)
{
    n = fmod(n, d);
    if (n < 0) n += d;
    return n;
}

//fast hyperbolic tanget function because std::tanh sucks
//juce::dsp has a fast math namespace witha all the good shit
float fast_tanh(float x) {
    float x2 = x * x;
    float a = x * (135135.0f + x2 * (17325.0f + x2 * (378.0f + x2)));
    float b = 135135.0f + x2 * (62370.0f + x2 * (3150.0f + x2 * 28.0f));
    return a / b;
}

//fast approximation of arc tangent fucntion
float fastAtan(float z)
{
    const float n1 = 0.97239411f;
    const float n2 = -0.19194795f;
    return (n1 + n2 * z * z) * z;
}

void RealMagiVerbAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    //sample block of the audio buffer
    juce::dsp::AudioBlock<float> sampleBlock(buffer);

    /*I really wanted to make the random component more natrual, but this is the right now solution, nothing is in stone
    first you get the vlaue of the slider, make that value the limit of a range
    then generate a random number in that range, use that random number as you will
    knowing that, that number will never surpass the limit of randomInput*/
    int randomInput = *apvts.getRawParameterValue("Entropy");

    //we can't manipulate the range in nextInt() but we can in a range
    juce::Range<int> randRange = {0, randomInput};

    //generate a random number between 0 and the knob value
    int randNum = random.nextInt(randRange);

    //all the values used in the distortion algorithms
    auto pre        = apvts.getRawParameterValue("Pre-Gain");
    auto post       = apvts.getRawParameterValue("Post-Gain");
    auto bitDepth   = apvts.getRawParameterValue("Bit Depth");
    auto rateDivide = apvts.getRawParameterValue("Rate Divide");
    auto gain       = apvts.getRawParameterValue("Distortion Gain");

    distChoices choice = (distChoices)(int)apvts.getRawParameterValue("Distortion Type")->load();

    //all the values we got from the slider, and pass them to the reverb::parameters object
    reverbParameters.roomSize     = *apvts.getRawParameterValue("Reverb Size") / 100;
    reverbParameters.damping      = *apvts.getRawParameterValue("Reverb Damping") / 100;
    reverbParameters.width        = *apvts.getRawParameterValue("Reverb Width") / 100;
    reverbParameters.wetLevel     = *apvts.getRawParameterValue("Reverb Dry/Wet") / 100;
    reverbParameters.dryLevel     = 1.f - *apvts.getRawParameterValue("Reverb Dry/Wet") / 100;

    //pass those parameters to the reverb object
    leftReverb.setParameters(reverbParameters);
    rightReverb.setParameters(reverbParameters);

    //make two different sample blocks, one responsible for the left channel and one for the right channel
    auto leftBlock = sampleBlock.getSingleChannelBlock(0);
    auto rightBlock = sampleBlock.getSingleChannelBlock(1);

    //make processing contexes for the left and right sample blocks
    juce::dsp::ProcessContextReplacing<float> leftContext(leftBlock);
    juce::dsp::ProcessContextReplacing<float> rightContext(rightBlock);
    juce::dsp::ProcessContextReplacing<float> monoContext(sampleBlock);

    //the chorus object have self containted method to set their parameters
    {
        leftChorus.setFeedback(0.2f);
        leftChorus.setCentreDelay(5.0f);
        leftChorus.setMix((*apvts.getRawParameterValue("Modulation Amount")) / 100);
        leftChorus.setRate(((*apvts.getRawParameterValue("Modulation Rate") + randNum) / 100));// +(randNum / 100));
        leftChorus.setDepth(((*apvts.getRawParameterValue("Modulation Amount") + randNum) / 100 * 0.25f));// +(randNum / 100));

        rightChorus.setFeedback(0.2f);
        rightChorus.setCentreDelay(5.0f);
        rightChorus.setMix((*apvts.getRawParameterValue("Modulation Amount")) / 100);
        rightChorus.setRate(((*apvts.getRawParameterValue("Modulation Rate") + randNum) / 100) * 5);
        rightChorus.setDepth(((*apvts.getRawParameterValue("Modulation Amount") + randNum) / 100 * 0.25f));

        leftChorus.process(leftContext);
        rightChorus.process(rightContext);
    }

    //the raw values of the lowcut and highcut frequency
    auto lowcut = apvts.getRawParameterValue("LowCut Frequency");
    auto highcut = apvts.getRawParameterValue("HighCut Frequency");

    buffer.applyGain(*pre);
    for (auto channel = 0; channel < buffer.getNumChannels(); ++channel)
    {
        auto* writePointer = buffer.getWritePointer(channel);
        for (auto sample = 0; sample < buffer.getNumSamples(); ++sample)
        {
            switch (choice)
            {
                //hard clip
                case 0:
                {
                    writePointer[sample] = juce::jlimit(-1.0f, 1.0f,
                        writePointer[sample] * (*gain + (randNum / 100)));
                }
                //soft clip
                case 1:
                {
                    if (writePointer[sample] < -1.f) {
                        writePointer[sample] = -0.9f;
                    }
                    else if (writePointer[sample] > 1.f) {
                        writePointer[sample] = 0.9f;
                    }
                    else {
                        writePointer[sample] = (*gain + (randNum / 100)) *
                            (writePointer[sample] - ((writePointer[sample] 
                                * writePointer[sample] * writePointer[sample]) / 3.f));
                    }
                }
                //overdrive
                case 2:
                {
                    writePointer[sample] = mod((*gain + (randNum / 100)) * writePointer[sample] + 1, 2) - 1;
                }
                //ZAP
                case 3:
                {
                    writePointer[sample] = juce::dsp::FastMathApproximations::sin((*gain + (randNum / 100)) 
                        * writePointer[sample] * (PI / 2));
                }
                //saturation
                case 4:
                {
                     writePointer[sample] = juce::dsp::FastMathApproximations::tanh((*gain + (randNum / 100)) 
                        * writePointer[sample]);
                }
                //wave shapper
                case 5:
                {
                    writePointer[sample] = (*gain + (randNum / 100)) * (writePointer[sample] + *gain 
                        * writePointer[sample] * writePointer[sample]);
                }
                //simple bypass
                case 6:
                {
                   writePointer[sample] =  writePointer[sample];
                }
            }
            //bit crush is always applied
            float totalQLevels = powf(2, *bitDepth);
            float val = writePointer[sample];
            float remainder = fmodf(val, 1 / totalQLevels);

            writePointer[sample] = val - remainder;

            if (*rateDivide > 1)
            {
                int rD = *rateDivide / 2;
                if (rD != 0)
                {
                    if (sample % rD != 0)
                        writePointer[sample] = writePointer[sample - sample % rD];
                }
                else
                {
                    rD++;
                    if (sample % rD != 0)
                        writePointer[sample] = writePointer[sample - sample % rD];
                }
            }
        }
    }

    leftReverb.process(leftContext);
    rightReverb.process(rightContext);

    lowCutFilter.setFilterCutoff(*lowcut, monoContext);
    highCutFilter.setFilterCutoff(*highcut, monoContext);

    buffer.applyGain(*post);
}

//==============================================================================
bool RealMagiVerbAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* RealMagiVerbAudioProcessor::createEditor()
{
    return new RealMagiVerbAudioProcessorEditor (*this);

    //return new juce::GenericAudioProcessorEditor(*this);
}

//==============================================================================
void RealMagiVerbAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.

    juce::ValueTree copyState = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml = copyState.createXml();
    copyXmlToBinary(*xml.get(), destData);
}

void RealMagiVerbAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.

    std::unique_ptr<juce::XmlElement> xml = getXmlFromBinary(data, sizeInBytes);
    juce::ValueTree copyState = juce::ValueTree::fromXml(*xml.get());
    apvts.replaceState(copyState);
}

void RealMagiVerbAudioProcessor::reset()
{
    leftReverb.reset();
    rightReverb.reset();
    leftChorus.reset();
    rightChorus.reset();
    lowCutFilter.reset();
    highCutFilter.reset();
}

juce::AudioProcessorValueTreeState::ParameterLayout RealMagiVerbAudioProcessor::createParameters()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;
    
    //the layout of the reverb stuff
    layout.add(std::make_unique<juce::AudioParameterFloat>("Reverb Size", "Reverb Size",
        juce::NormalisableRange<float>(0.0f, 100.0f, 1.0f, 1.0f),
        10.0f));

    layout.add(std::make_unique<juce::AudioParameterFloat>("Reverb Damping", "Reverb Damping",
        juce::NormalisableRange<float>(0.0f, 100.0f, 1.0f, 1.0f),
        20.0f));

    layout.add(std::make_unique<juce::AudioParameterFloat>("Reverb Width", "Reverb Width",
        juce::NormalisableRange<float>(0.0f, 100.0f, 1.0f, 1.0f),
        50.0f));

    layout.add(std::make_unique<juce::AudioParameterFloat>("Reverb Dry/Wet", "Reverb Dry/Wet",
        juce::NormalisableRange<float>(0.0f, 100.0f, 1.0f, 1.0f),
        0.0f));


    //the layout and parameters for the chorus stuff
    layout.add(std::make_unique<juce::AudioParameterFloat>("Modulation Rate", " Modulation Rate",
        juce::NormalisableRange<float>(0.0f, 100.0f, 0.01f), 0.0f));

    layout.add(std::make_unique<juce::AudioParameterFloat>("Modulation Amount", "Modulation Amount",
        juce::NormalisableRange<float>(0.0f, 100.0f, 0.01f), 0.0f));

    //these are the layouts of bruh sliders
    layout.add(std::make_unique<juce::AudioParameterFloat>("LowCut Frequency",
        "LowCut Frequency",
        juce::NormalisableRange<float>(20.0f, 20000.0f, 1.0f, 0.5f),
        20.0f));

    layout.add(std::make_unique<juce::AudioParameterFloat>("HighCut Frequency",
        "HighCut Frequency",
        juce::NormalisableRange<float>(20.0f, 20000.0f, 1.0f, 1.5f),
        20000.0f));

    //distortion layout
    layout.add(std::make_unique<juce::AudioParameterFloat>("Pre-Gain", "Pre-Gain",
        juce::NormalisableRange<float>(0.1f, 2.0f, 0.01f), 0.5f));

    layout.add(std::make_unique<juce::AudioParameterFloat>("Post-Gain", "Post-Gain",
        juce::NormalisableRange<float>(0.1f, 2.0f, 0.01f), 1.0f));

    layout.add(std::make_unique<juce::AudioParameterFloat>("Distortion Gain", "Distortion Gain",
        juce::NormalisableRange<float>(0.1f, 2.0f, 0.001f), 1.0f));

    layout.add(std::make_unique<juce::AudioParameterFloat>("Rate Divide", "Rate Divide",
        juce::NormalisableRange<float>(1.0f, 100.0f, 0.01f), 1.0f));

    layout.add(std::make_unique<juce::AudioParameterFloat>("Bit Depth", "Bit Depth",
        juce::NormalisableRange<float>(1.0f, 32.0f, 1.0f), 32.0f));

    //parameter for the entropy value
    layout.add(std::make_unique<juce::AudioParameterInt>("Entropy", "Entropy", 1, 100, 10));

    //array for the distortion choices
    juce::StringArray array;
    array.add("Hard Clip");
    array.add("Soft Clip");
    array.add("Over Drive");
    array.add("AMP");
    array.add("Saturation");
    array.add("Wave Shaper");
    array.add("Bypass");

    layout.add(std::make_unique<juce::AudioParameterChoice>("Distortion Type", "Distortion Type", array, 6));

    return layout;
}

BetterFilter::BetterFilter(int type)
{
    setType(type);
}

BetterFilter::~BetterFilter()
{
    
}

void BetterFilter::setType(int type)
{
    if(type == 1)
    {
        Filter1.setType(juce::dsp::StateVariableTPTFilterType::highpass);
        Filter2.setType(juce::dsp::StateVariableTPTFilterType::highpass);
        Filter3.setType(juce::dsp::StateVariableTPTFilterType::highpass);
        Filter4.setType(juce::dsp::StateVariableTPTFilterType::highpass);
    }

    if(type == 2)
    {
        Filter1.setType(juce::dsp::StateVariableTPTFilterType::lowpass);
        Filter2.setType(juce::dsp::StateVariableTPTFilterType::lowpass);
        Filter3.setType(juce::dsp::StateVariableTPTFilterType::lowpass);
        Filter4.setType(juce::dsp::StateVariableTPTFilterType::lowpass);
    }
}

void BetterFilter::prepare(juce::dsp::ProcessSpec spec)
{
    Filter1.prepare(spec);
    Filter2.prepare(spec);
    Filter3.prepare(spec);
    Filter4.prepare(spec);

    Filter1.setResonance(0.99f);
    Filter2.setResonance(0.99f);
    Filter3.setResonance(0.99f);
    Filter4.setResonance(0.99f);
}

void BetterFilter::reset()
{
    Filter1.reset();
    Filter2.reset();
    Filter3.reset();
    Filter4.reset();
}

void BetterFilter::setFilterCutoff(float cut, juce::dsp::ProcessContextReplacing<float> context)
{
    Filter1.setCutoffFrequency(cut);
    Filter2.setCutoffFrequency(cut);
    Filter3.setCutoffFrequency(cut);
    Filter4.setCutoffFrequency(cut);

    Filter1.process(context);
    Filter2.process(context);
    Filter3.process(context);
    Filter4.process(context);
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new RealMagiVerbAudioProcessor();
}
