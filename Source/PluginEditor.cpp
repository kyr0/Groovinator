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
    
    // Test slider (pitch-shift, rate, etc. depending on the need...)
    _testSlider.setSliderStyle(Slider::LinearVertical);
    //_testSlider.setRange(-24.0, 24.0); // Semitones
    _testSlider.setRange(0.1, 3.0); // Rate
    _testSlider.setTextBoxStyle(Slider::TextBoxBelow, false, 50, 20);
    //_testSlider.showTextBox();
    _testSlider.setPopupDisplayEnabled(true, this);
    //_testSlider.setTextValueSuffix(" (test)");
    //_testSlider.setValue(0.0); // Semitones
    _testSlider.setValue(1.0); // Rate
    _testSlider.addListener(this);
    
    // Playhead info label
    _playHeadInfoLabel.setText("", dontSendNotification);
    _playHeadInfoLabel.setJustificationType(Justification::centred);
    
    // Debug label
    _debugLabel.setFont(10.0f);
    _debugLabel.setText("", dontSendNotification);
    
    
    //==============================================================================
    //Rhythm
    
    // Num steps
    _originalNumStepsSlider.setSliderStyle(Slider::IncDecButtons);
    _originalNumStepsSlider.setRange(1, 99, 1);
    _originalNumStepsSlider.setTextBoxStyle(Slider::TextBoxLeft, false, 30, 20);
    _originalNumStepsSlider.setIncDecButtonsMode(Slider::incDecButtonsDraggable_AutoDirection);
    _originalNumStepsSlider.setValue(8);
    _originalNumStepsSlider.addListener(this);
    _originalNumStepsLabel.setText("# steps", dontSendNotification);
    
    _targetNumStepsSlider.setSliderStyle(Slider::IncDecButtons);
    _targetNumStepsSlider.setRange(1, 99, 1);
    _targetNumStepsSlider.setTextBoxStyle(Slider::TextBoxLeft, false, 30, 20);
    _targetNumStepsSlider.setIncDecButtonsMode(Slider::incDecButtonsDraggable_AutoDirection);
    _targetNumStepsSlider.setValue(8);
    _targetNumStepsSlider.addListener(this);
    _targetNumStepsLabel.setText("# steps", dontSendNotification);
    
    // Add components to editor
    addAndMakeVisible(&_testSlider);
    addAndMakeVisible(&_playHeadInfoLabel);
    addAndMakeVisible(&_debugLabel);
    addAndMakeVisible(&_originalNumStepsSlider);
    addAndMakeVisible(&_originalNumStepsLabel);
    addAndMakeVisible(&_targetNumStepsSlider);
    addAndMakeVisible(&_targetNumStepsLabel);

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
    _testSlider.setBounds(30, 80, 50, (getHeight()/2));
    _playHeadInfoLabel.setBounds(0, 40, getWidth(), 20);
    _debugLabel.setBounds(0, getHeight()-60, getWidth(), 60);
    
    
    // Rhythm components
    double originalRhythmY = 60;
    double targetRhythmY = originalRhythmY + 120;
    
    double numStepsX = getWidth()-80;
    
    double numStepsLabelYOffset = 0;
    double numStepsLabelWidth = 60;
    double numStepsLabelHeight = 20;
    
    double numStepsSliderYOffset = 20;
    double numStepsSliderWidth = 60;
    double numStepsSliderHeight = 50;
    
    _originalNumStepsLabel.setBounds(numStepsX, originalRhythmY + numStepsLabelYOffset, numStepsLabelWidth, numStepsLabelHeight);
    _originalNumStepsSlider.setBounds(numStepsX, originalRhythmY + numStepsSliderYOffset, numStepsSliderWidth, numStepsSliderHeight);
    _targetNumStepsLabel.setBounds(numStepsX, targetRhythmY + numStepsLabelYOffset, numStepsLabelWidth, numStepsLabelHeight);
    _targetNumStepsSlider.setBounds(numStepsX, targetRhythmY + numStepsSliderYOffset, numStepsSliderWidth, numStepsSliderHeight);
}

//
void GroovinatorAudioProcessorEditor::sliderValueChanged(Slider* slider)
{
    double v = slider->getValue();
    if (slider == &_testSlider)
    {
        processor.setTestSliderValue(v);
        printf("test slider changed\n");
    }
    else if (slider == &_originalNumStepsSlider)
    {
        processor.setOriginalNumSteps(v);
        printf("original num steps slider changed\n");
    }
    else if (slider == &_targetNumStepsSlider)
    {
        processor.setOriginalNumSteps(v);
        printf("target num steps slider changed\n");
    }
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
    debugLabelText  << "/* "
                    << "sampsPerMeasure=" << String(processor.calculateNumSamplesPerMeasure())
                    //<< ", pulses per measure: " << String(processor.calculateNumPulsesPerMeasure())
                    << ", posSamps=" << String(processor.calculatePlayHeadRelativePositionInSamples())
                    << ", mostRecentSamp=" << String(processor.getMostRecentMeasureBufferSample())
                    << ", diff=" << String(processor.getMostRecentMeasureBufferSample()-processor.calculatePlayHeadRelativePositionInSamples())
                    //<< ", soundTouchTempo=" << String(processor.getSoundTouchTempo())
                    << ", measureBufSz=" << String(processor.getMeasureBufferSize())
                    << "\n"
                    << ", measureNum=" << String(processor.getMeasuresElapsed())
                    << ", ioRatio=" << String(processor.getSoundTouchInputOutputRatio(), 2)
                    << " */";
    _debugLabel.setText(debugLabelText, sendNotification);
}
