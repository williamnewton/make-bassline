#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include "generator/EuclideanRhythm.h"
#include "generator/PitchGenerator.h"
#include "generator/PatternState.h"

class BasslineGeneratorProcessor : public juce::AudioProcessor
{
public:
    BasslineGeneratorProcessor();
    ~BasslineGeneratorProcessor() override;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return JucePlugin_Name; }

    bool acceptsMidi() const override { return true; }
    bool producesMidi() const override { return true; }
    bool isMidiEffect() const override { return true; }
    double getTailLengthSeconds() const override { return 0.0; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override { return {}; }
    void changeProgramName(int, const juce::String&) override {}

    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    juce::AudioProcessorValueTreeState apvts;

    // Pattern state for UI access (lock-free)
    PatternState patternState;

private:
    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    // Generator components
    EuclideanRhythm euclidean;
    PitchGenerator pitchGen;

    // Timing state
    double currentSampleRate = 44100.0;
    double ppqPerSample = 0.0;
    int64_t lastPpqPosition = -1;
    int currentStep = 0;

    // Note tracking for note-offs
    int activeNote = -1;
    int noteDurationSamples = 0;
    int samplesUntilNoteOff = 0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BasslineGeneratorProcessor)
};
