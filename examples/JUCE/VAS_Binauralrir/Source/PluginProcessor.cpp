/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"


//==============================================================================
Vas_binauralrirAudioProcessor::Vas_binauralrirAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                   
                       .withInput  ("Input",  AudioChannelSet::stereo(), true)
                     
                       .withOutput ("Output", AudioChannelSet::stereo(), true)
                     
                       )

#endif
{
   /* mUndoManager = new UndoManager();
    mState = new AudioProcessorValueTreeState(*this, mUndoManager);
    mState->createAndAddParameter(paramGain, "Gain", TRANS ("Input Gain"), NormalisableRange<float> (0.0, 2.0, 0.1), 1.0, nullptr, nullptr);
    mState->addParameterListener (paramGain, this);
    mState->state = ValueTree ("stp_gain");*/
    x = vas_fir_binaural_new(VAS_VDSP | VAS_BINAURALSETUP_NOELEVATION | VAS_LOCALFILTER, 512, NULL);
    char* path = NULL;
    int length, dirname_length;
    fpath = new char[1024];
    
    length = wai_getModulePath(NULL, 0, &dirname_length);
    if (length > 0)
    {
        path = (char*)malloc(length + 1);
        wai_getModulePath(path, length, &dirname_length);
        path[dirname_length] = '\0';
        char irName[512];
        sprintf(irName, "%s/fr_pos1.txt", path);
        sprintf(fpath, "%s", path);
        vas_fir_readText_1IrPerLine((vas_fir *)x, irName);
        vas_fir_setInitFlag((vas_fir *)x);
    }
    
    inm = new float[8192];
    outputL = new float[8192];
    outputR = new float[8192];
    
}

Vas_binauralrirAudioProcessor::~Vas_binauralrirAudioProcessor()
{
    vas_fir_binaural_free(x);
    delete inm;
    delete outputR;
    delete outputL;
}

//==============================================================================
const String Vas_binauralrirAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool Vas_binauralrirAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}



bool Vas_binauralrirAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool Vas_binauralrirAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double Vas_binauralrirAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int Vas_binauralrirAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int Vas_binauralrirAudioProcessor::getCurrentProgram()
{
    return 0;
}

void Vas_binauralrirAudioProcessor::setCurrentProgram (int index)
{
}

const String Vas_binauralrirAudioProcessor::getProgramName (int index)
{
    return {};
}

void Vas_binauralrirAudioProcessor::changeProgramName (int index, const String& newName)
{
}

//==============================================================================
void Vas_binauralrirAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
}

void Vas_binauralrirAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool Vas_binauralrirAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    if (layouts.getMainOutputChannelSet() != AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != AudioChannelSet::stereo())
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

void Vas_binauralrirAudioProcessor::processBlock (AudioBuffer<float>& buffer, MidiBuffer& midiMessages)
{
    ScopedNoDenormals noDenormals;
    int vectorSize = buffer.getNumSamples();
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();
    
    //for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
     //   buffer.clear (i, 0, buffer.getNumSamples());

    auto* channelData = buffer.getReadPointer (0);
    float *inputPtr = inm;
    for(int i = 0; i < vectorSize; i++)
        *inputPtr++ = *channelData++;
        

    
    vas_fir_binaural_process(x, inm, outputL, outputR, vectorSize);
    outputs = totalNumOutputChannels;
    
    auto *channelData1 = buffer.getWritePointer (0);
    float *outputLptr = outputL;
    for(int i = 0; i < vectorSize; i++)
    {
        *outputLptr *= 10.f;
        *channelData1 = *outputLptr;
        outputLptr++;
        channelData1++;
    }
    
    if(totalNumOutputChannels >= 2)
    {
        channelData1 = buffer.getWritePointer (1);
        float *outputRptr = outputR;
        for(int i = 0; i < vectorSize; i++)
            *channelData1++ = *outputRptr++ * 10.f;
    }
 }

//==============================================================================
bool Vas_binauralrirAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

AudioProcessorEditor* Vas_binauralrirAudioProcessor::createEditor()
{
    return new Vas_binauralrirAudioProcessorEditor (*this);
}

//==============================================================================
void Vas_binauralrirAudioProcessor::getStateInformation (MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void Vas_binauralrirAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new Vas_binauralrirAudioProcessor();
}
