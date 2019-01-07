/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "PluginProcessor.h"

//==============================================================================
/**
*/
class Vas_binauralrirAudioProcessorEditor  : public AudioProcessorEditor
{
public:
    Vas_binauralrirAudioProcessorEditor (Vas_binauralrirAudioProcessor&);
    ~Vas_binauralrirAudioProcessorEditor();

    //==============================================================================
    void paint (Graphics&) override;
    void resized() override;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    Vas_binauralrirAudioProcessor& processor;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Vas_binauralrirAudioProcessorEditor)
};
