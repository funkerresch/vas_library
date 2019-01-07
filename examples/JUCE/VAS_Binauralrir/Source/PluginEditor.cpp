/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
Vas_binauralrirAudioProcessorEditor::Vas_binauralrirAudioProcessorEditor (Vas_binauralrirAudioProcessor& p)
    : AudioProcessorEditor (&p), processor (p)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize (800, 800);
}

Vas_binauralrirAudioProcessorEditor::~Vas_binauralrirAudioProcessorEditor()
{
}

//==============================================================================
void Vas_binauralrirAudioProcessorEditor::paint (Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (ResizableWindow::backgroundColourId));
    char bums[512];
    sprintf(bums, "%f", processor.outputL[32]);
    g.setColour (Colours::white);
    g.setFont (15.0f);
    g.drawFittedText (bums, getLocalBounds(), Justification::centred, 1);
}

void Vas_binauralrirAudioProcessorEditor::resized()
{
    
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
}
