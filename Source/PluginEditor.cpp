/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"


//==============================================================================
GroovinatorAudioProcessorEditor::GroovinatorAudioProcessorEditor (GroovinatorAudioProcessor& p)
    : AudioProcessorEditor (&p), processor (p)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize (400, 300);
    
    // Frequency slider
    _freqSlider.setSliderStyle(Slider::LinearBarVertical);
    _freqSlider.setRange(-24.0, 24.0); // Semitones
    _freqSlider.setTextBoxStyle(Slider::NoTextBox, false, 90, 0);
    _freqSlider.setPopupDisplayEnabled(true, this);
    _freqSlider.setTextValueSuffix(" semitones to pitch-shift");
    _freqSlider.setValue(0.0);
    _freqSlider.addListener(this);
    
    // Playhead info label
    _playHeadInfoLabel.setText("", dontSendNotification);
    _playHeadInfoLabel.setJustificationType(Justification::centred);
    
    // Debug label
    _debugLabel.setFont(10.0f);
    _debugLabel.setText("", dontSendNotification);
    
    // Add components to editor
    addAndMakeVisible(&_freqSlider);
    addAndMakeVisible(&_playHeadInfoLabel);
    addAndMakeVisible(&_debugLabel);

    // Start timer
    startTimer(50);
}

GroovinatorAudioProcessorEditor::~GroovinatorAudioProcessorEditor()
{
}

//==============================================================================
void GroovinatorAudioProcessorEditor::paint (Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (ResizableWindow::backgroundColourId));

    g.setColour (Colours::white);
    g.setFont (24.0f);
    g.drawFittedText ("G R O O V I N A T O R", 0, 0, getWidth(), 30, Justification::centred, 1);
}

void GroovinatorAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
    _freqSlider.setBounds(30, 80, 20, getHeight()/2);
    _playHeadInfoLabel.setBounds(0, 40, getWidth(), 20);
    _debugLabel.setBounds(0, getHeight()-20, getWidth(), 20);
}

//
void GroovinatorAudioProcessorEditor::sliderValueChanged(Slider* slider)
{
    processor.setFreq(slider->getValue());
}

void GroovinatorAudioProcessorEditor::timerCallback()
{
    // Set playhead info label text
    //_playHeadInfoLabel.setText(String(processor.getHostBpm(), 0), sendNotification);
    String playHeadLabelText;
    if (processor.getHasPlayHeadBeenSet())
    {
        AudioPlayHead::CurrentPositionInfo playHeadInfo = processor.getPlayHeadInfo();

        playHeadLabelText << "BPM: "                  << String((int)playHeadInfo.bpm)
                          << " | Meter: "             << String(playHeadInfo.timeSigNumerator)
                                                      << "/" << String(playHeadInfo.timeSigDenominator)
        
                          //<< " | Position: "          << String(processor.getPlayHeadBarNum())
                          //                            << "." << String(processor.getPlayHeadRelativePulseNum());
        
                          << " | Position: "          << String(playHeadInfo.ppqPosition, 2)
                          << " | Last Bar Start: "    << String((int)playHeadInfo.ppqPositionOfLastBarStart);
        
                          //<< " | Time: " << String((int)playHeadInfo.timeInSeconds)
                          //<< "s (" << String(playHeadInfo.timeInSamples) << " samples)";
    }
    else
    {
        playHeadLabelText = "Start playback to make playhead info available";
    }
    _playHeadInfoLabel.setText(playHeadLabelText, sendNotification);
    
    // Set debug label text
    String debugLabelText;
    debugLabelText << "/* "
                   << "samples per measure: " << String(processor.calculateNumSamplesPerMeasure())
                   << ", pulses per measure: " << String(processor.calculateNumPulsesPerMeasure())
                   << " */";
    _debugLabel.setText(debugLabelText, sendNotification);
}
