#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include "PluginProcessor.h"
#include "ui/CircularVisualizer.h"
#include "ui/StepSequencerGrid.h"
#include "ui/MidiDragComponent.h"
#include "ui/ComicBookLookAndFeel.h"

class BasslineGeneratorEditor : public juce::AudioProcessorEditor,
                                 private juce::Timer,
                                 public juce::DragAndDropContainer
{
public:
    explicit BasslineGeneratorEditor(BasslineGeneratorProcessor&);
    ~BasslineGeneratorEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    void timerCallback() override;

    BasslineGeneratorProcessor& processorRef;

    // Rhythm controls
    juce::Slider stepsSlider, hitsSlider, rotationSlider;
    juce::Label stepsLabel, hitsLabel, rotationLabel;

    // Pitch controls
    juce::Slider rootNoteSlider, octaveRangeSlider;
    juce::ComboBox scaleSelector;
    juce::Label rootNoteLabel, scaleLabel, octaveLabel;

    // Note controls
    juce::Slider noteLengthSlider, velocitySlider;
    juce::Label noteLengthLabel, velocityLabel;

    // Groove controls
    juce::Slider swingSlider, humanizeSlider;
    juce::Label swingLabel, humanizeLabel;

    // Pattern display
    juce::Slider seedSlider;
    juce::Label seedLabel;
    juce::TextButton regenerateButton;

    // Visualizers
    CircularVisualizer circularViz;
    StepSequencerGrid stepGrid;

    // MIDI Export
    MidiDragComponent midiDragArea;
    juce::ComboBox barLengthSelector;
    juce::Label barLengthLabel;

    // Attachments
    std::vector<std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>> sliderAttachments;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> scaleAttachment;

    // Custom look and feel
    ComicBookLookAndFeel comicLookAndFeel;

    juce::MemoryBlock createMidiPattern();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BasslineGeneratorEditor)
};
