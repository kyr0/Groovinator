/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"


//==============================================================================
GroovinatorAudioProcessor::GroovinatorAudioProcessor() :
#ifndef JucePlugin_PreferredChannelConfigurations
    AudioProcessor (BusesProperties()
                 #if ! JucePlugin_IsMidiEffect
                  #if ! JucePlugin_IsSynth
                   .withInput  ("Input",  AudioChannelSet::stereo(), true)
                  #endif
                   .withOutput ("Output", AudioChannelSet::stereo(), true)
                 #endif
                   ),
#endif
    _soundTouch(),
    _playHead(NULL),
    _hasPlayHeadBeenSet(false),
    _hasMeasureBufferBeenSet(false)
{
}

GroovinatorAudioProcessor::~GroovinatorAudioProcessor()
{
}

//==============================================================================
const String GroovinatorAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool GroovinatorAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool GroovinatorAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

double GroovinatorAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int GroovinatorAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int GroovinatorAudioProcessor::getCurrentProgram()
{
    return 0;
}

void GroovinatorAudioProcessor::setCurrentProgram (int index)
{
}

const String GroovinatorAudioProcessor::getProgramName (int index)
{
    return {};
}

void GroovinatorAudioProcessor::changeProgramName (int index, const String& newName)
{
}

//==============================================================================
void GroovinatorAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
}

void GroovinatorAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool GroovinatorAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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

void GroovinatorAudioProcessor::processBlock (AudioSampleBuffer& buffer, MidiBuffer& midiMessages)
{
    // Update sample rate
    _sampleRate = getSampleRate();
    
    // Getting host info from playhead (we can only do this from within processBlock,
    // which is why we keep track of playhead info so we can use it elsewhere)
    _playHead = getPlayHead();
    if (_playHead != nullptr)
    {
        _playHead->getCurrentPosition(_playHeadInfo);
        _hasPlayHeadBeenSet = true;
        //_hostBpm = curPosInfo.bpm;
    }
    else
    {
        _hasPlayHeadBeenSet = false;
        return;
    }
    
    // For now, no need to process if playhead isn't moving
    // Eventually a host-independent "live mode" would be nice
    if (_playHeadInfo.timeInSamples == 0)
        return;
    
    // Audio reading and writing
    //
    const int totalNumInputChannels  = getTotalNumInputChannels();
    const int totalNumOutputChannels = getTotalNumOutputChannels();
    
    const int numSamples = buffer.getNumSamples();

    // In case we have more outputs than inputs, this code clears any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    // This is here to avoid people getting screaming feedback
    // when they first compile a plugin, but obviously you don't need to keep
    // this code if your algorithm always overwrites all the output channels.
    for (int i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());
    
    
    // Create measure buffer to store time-stretched data
    //if (/*_playHeadInfo.timeInSamples == 0 ||*/ calculatePlayHeadRelativePositionInSamples() == 0)
    if (calculatePlayHeadRelativePositionInSamples() <= numSamples || _playHeadInfo.ppqPositionOfLastBarStart != _mostRecentMeasureStartPpq)
    {
//        if (!_hasMeasureBufferBeenSet)
//        {
            _measureBuffer = AudioSampleBuffer(totalNumInputChannels, calculateNumSamplesPerMeasure());
            _hasMeasureBufferBeenSet = true;
//        }
    
        for (int i=0; i<_measureBuffer.getNumChannels(); i++)
            _measureBuffer.clear(i, 0, _measureBuffer.getNumSamples()); // Make sure to clear measure buffer so we don't get a nasty pop when first playing
        
        _mostRecentMeasureBufferSample = std::max(calculatePlayHeadRelativePositionInSamples(), 0); // Reset this value
        //_mostRecentMeasureBufferSample = 0;
    }
    
    //if (_mostRecentMeasureBufferSample >= _measureBuffer.getNumSamples())
    //    return;
    
    _mostRecentMeasureStartPpq = _playHeadInfo.ppqPositionOfLastBarStart;

    // This is the place where you'd normally do the guts of your plugin's
    // audio processing...
    for (int channel = 0; channel < totalNumInputChannels; ++channel)
    {
        float* channelData = buffer.getWritePointer (channel);

        // ..do something to the data...

        // Pitch shift
        /*
        if (numSamples > 0)
        {
            _soundTouch.setSampleRate(getSampleRate());
            //_soundTouch.setChannels(totalNumInputChannels);
            _soundTouch.setChannels(1);
            _soundTouch.putSamples(channelData, numSamples);
            
            int numReceivedSamples = 0;
            int receiveIterationNum = 0;
            
            //if (numReceivedSamples != numSamples)
            //{
            //    printf("put %d samples but only received %d samples\n", numSamples, numReceivedSamples);
            //}
            
            do
            {
                receiveIterationNum++;
                numReceivedSamples = _soundTouch.receiveSamples(channelData, numSamples);
                //printf("%d.%d: received %d of %d samples\n", channel, receiveIterationNum, numReceivedSamples, numSamples);
            }
            while (numReceivedSamples != 0 && numReceivedSamples != numSamples);
        }
        */
        
        // Tempo stretch and preserve buffer
        if (numSamples > 0)
        {
            // Setup SoundTouch
            _soundTouch.setSampleRate(getSampleRate());
            //_soundTouch.setChannels(totalNumInputChannels);
            _soundTouch.setChannels(1);
            _soundTouch.setRate(1.0);
            _soundTouchTempo = 0.5; // Hard-code this to test (doesn't work yet for >1.0)
            _soundTouch.setTempo(_soundTouchTempo);
            
            // Get input samples
            _soundTouch.putSamples(channelData, numSamples);
            
            // Process and write measure buffer samples, if we can
            float* measureChannelData = _measureBuffer.getWritePointer (channel, _mostRecentMeasureBufferSample);
            int totalNumReceivedSamples = 0;
            int numReceivedSamples = 0;
            int receiveIterationNum = 0;
            int numOutputSamples = (int) (numSamples / _soundTouchTempo);
            bool canWriteToMeasureBuffer = (_measureBuffer.getNumSamples() > _mostRecentMeasureBufferSample+numOutputSamples); // && (_mostRecentMeasureBufferSample+numOutputSamples < calculateNumSamplesPerMeasure())
            if (canWriteToMeasureBuffer)
            {
                do
                {
                    receiveIterationNum++;
                    numReceivedSamples = _soundTouch.receiveSamples(measureChannelData, numOutputSamples);
                    //printf("%d.%d: received %d of %d samples\n", channel, receiveIterationNum, numReceivedSamples, numSamples);
                    totalNumReceivedSamples += numReceivedSamples;
                }
                while (numReceivedSamples != 0);
                //while (numReceivedSamples != 0 && numOutputSamples-totalNumReceivedSamples > 0);
                //while (numReceivedSamples != 0 && numReceivedSamples != numOutputSamples);
            
                // Update most recent sample index
                _mostRecentMeasureBufferSample = std::min(_mostRecentMeasureBufferSample + numOutputSamples, _measureBuffer.getNumSamples()-1);
                //_mostRecentMeasureBufferSample = std::min(_mostRecentMeasureBufferSample + totalNumReceivedSamples, _measureBuffer.getNumSamples()-1);
            }
        }
        
        // Write output samples from measure buffer
        int posInSamples = calculatePlayHeadRelativePositionInSamples();
        int endPosInSamples = posInSamples + numSamples;
        if (endPosInSamples < _mostRecentMeasureBufferSample)
        {
            // Clear output buffer
            buffer.clear(channel, 0, buffer.getNumSamples());
            
            const float* measureChannelOutputData = _measureBuffer.getReadPointer(channel, posInSamples);
            
            // Write using SoundTouch object
//            st.setSampleRate(_sampleRate);
//            st.setChannels(1);
//            st.putSamples(measureChannelOutputData, numSamples);
//            int numReceivedSamples = 0;
//            do
//            {
//                numReceivedSamples = st.receiveSamples(channelData, numSamples);
//            }
//            while (numReceivedSamples != 0 && numReceivedSamples != numSamples);
//            
            
            // Write manually
            for (int sampleIdx=0; sampleIdx<numSamples; sampleIdx++)
            {
                channelData[sampleIdx] = measureChannelOutputData[sampleIdx];
            }
        }
    }
}

//==============================================================================
bool GroovinatorAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

AudioProcessorEditor* GroovinatorAudioProcessor::createEditor()
{
    return new GroovinatorAudioProcessorEditor (*this);
}

//==============================================================================
void GroovinatorAudioProcessor::getStateInformation (MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void GroovinatorAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new GroovinatorAudioProcessor();
}

// Getters and setters

void GroovinatorAudioProcessor::setTestSliderValue(float v)
{
    //_freq = v;
    //_soundTouch.setPitchSemiTones(v);
    
    //_soundTouch.setRate(v);
    
    _soundTouchTempo = v;
    //_soundTouch.setTempo(v);
}

//float GroovinatorAudioProcessor::getFreq()
//{
//    return _freq;
//}

double GroovinatorAudioProcessor::getPlayHeadBpm()
{
    return _playHeadInfo.bpm;
}

AudioPlayHead::CurrentPositionInfo GroovinatorAudioProcessor::getPlayHeadInfo()
{
    return _playHeadInfo;
}

bool GroovinatorAudioProcessor::getHasPlayHeadBeenSet()
{
    return _hasPlayHeadBeenSet;
}

int GroovinatorAudioProcessor::getPlayHeadBarNum()
{
    // TODO:
    // - Handle cases where denominator != 4
    // - Handle time signature changes (do we have access to enough info for this to work?)
    return _playHeadInfo.ppqPositionOfLastBarStart / _playHeadInfo.timeSigNumerator;
}

int GroovinatorAudioProcessor::getPlayHeadRelativePulseNum()
{
    return _playHeadInfo.ppqPosition - _playHeadInfo.ppqPositionOfLastBarStart;
}

int GroovinatorAudioProcessor::getMostRecentMeasureBufferSample()
{
    return _mostRecentMeasureBufferSample;
}

int GroovinatorAudioProcessor::getMeasureBufferSize()
{
    return _measureBuffer.getNumSamples();
}

// Utility methods

int GroovinatorAudioProcessor::calculatePlayHeadRelativePositionInSamples()
{
    if (!_hasPlayHeadBeenSet)
        return 0;
    
    int samplesPerMeasure = calculateNumSamplesPerMeasure();
    int pulsesPerMeasure = calculateNumPulsesPerMeasure();
    double relativePositionInPulses = (_playHeadInfo.ppqPosition - _playHeadInfo.ppqPositionOfLastBarStart) / (double) pulsesPerMeasure;
    
    //int relativePositionInSamples = (int) (_sampleRate * calculateSecondsPerBeat() * relativePositionInPulses);
    int relativePositionInSamples = (int) (relativePositionInPulses * samplesPerMeasure);
    relativePositionInSamples = relativePositionInSamples >= samplesPerMeasure ? 0 : relativePositionInSamples;
    return std::max(relativePositionInSamples, 0);
}

int GroovinatorAudioProcessor::calculateNumPulsesPerMeasure()
{
    if (!_hasPlayHeadBeenSet)
        return 0;
    
    return (int) (4.0 * _playHeadInfo.timeSigNumerator / _playHeadInfo.timeSigDenominator);
}

int GroovinatorAudioProcessor::calculateNumSamplesPerMeasure()
{
    if (!_hasPlayHeadBeenSet || _sampleRate == 0)
        return 0;
    
    // BPM
    double secondsPerBeat = calculateSecondsPerBeat();

    // Pulses (beats)
    //double ppqLastBarStart = _playHeadInfo.ppqPositionOfLastBarStart;
    int pulsesPerMeasure = calculateNumPulsesPerMeasure();
    
    // Time since last edit
    //double timeInSeconds = _playHeadInfo.timeInSeconds;
    //int timeInSamples = _playHeadInfo.timeInSamples;
    //double samplesPerSecond = timeInSamples / timeInSeconds;
    
    // ... Or just use sample rate (same result)
    double samplesPerSecond = _sampleRate;
    
    // Calculate number of samples
    int numSamples = (int) (samplesPerSecond * secondsPerBeat * pulsesPerMeasure);
    
    return std::max(numSamples, 0);
}

double GroovinatorAudioProcessor::calculateSecondsPerBeat()
{
    if (!_hasPlayHeadBeenSet)
        return 0.0;

    return 60.0 / _playHeadInfo.bpm;
}

//void GroovinatorAudioProcessor::updateValuesFromPlayHead()
//{
//    _hostBpm = _playHeadInfo.bpm;
//}
