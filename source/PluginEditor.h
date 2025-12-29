#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include "PluginProcessor.h"
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

    // Essential controls (5 only)
    juce::Slider stepsSlider, densitySlider, swingSlider, rootNoteSlider;
    juce::Label stepsLabel, densityLabel, swingLabel, rootNoteLabel;

    juce::ComboBox scaleSelector;
    juce::Label scaleLabel;

    // Hidden advanced controls (still functional, not visible)
    juce::Slider rotationSlider, octaveRangeSlider, noteLengthSlider;
    juce::Slider velocitySlider, humanizeSlider, seedSlider;
    juce::Label rotationLabel, octaveLabel, noteLengthLabel;
    juce::Label velocityLabel, humanizeLabel, seedLabel;
    juce::TextButton regenerateButton;

    // Randomization button
    juce::TextButton randomizeButton;

    // Logo
    juce::Image logoImage;

    // Logo component
    class LogoComponent : public juce::Component
    {
    public:
        void setImage(const juce::Image& img) { image = img; repaint(); }

        void paint(juce::Graphics& g) override
        {
            if (image.isValid())
                g.drawImage(image, getLocalBounds().toFloat(),
                           juce::RectanglePlacement::centred | juce::RectanglePlacement::onlyReduceInSize);
        }
    private:
        juce::Image image;
    };

    LogoComponent logoComponent;

    // Visualizer - big step grid
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
