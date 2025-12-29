#include "PluginEditor.h"

BasslineGeneratorEditor::BasslineGeneratorEditor(BasslineGeneratorProcessor& p)
    : AudioProcessorEditor(&p), processorRef(p)
{
    setSize(700, 630);

    // Setup sliders with labels
    auto setupSlider = [this](juce::Slider& slider, juce::Label& label,
                              const juce::String& name, const juce::String& paramId)
    {
        slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 20);
        addAndMakeVisible(slider);

        label.setText(name, juce::dontSendNotification);
        label.setJustificationType(juce::Justification::centred);
        addAndMakeVisible(label);

        sliderAttachments.push_back(
            std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
                processorRef.apvts, paramId, slider));
    };

    setupSlider(stepsSlider, stepsLabel, "Steps", "steps");
    setupSlider(hitsSlider, hitsLabel, "Hits", "hits");
    setupSlider(rotationSlider, rotationLabel, "Rotation", "rotation");
    setupSlider(rootNoteSlider, rootNoteLabel, "Root", "rootNote");
    setupSlider(octaveRangeSlider, octaveLabel, "Octaves", "octaveRange");
    setupSlider(noteLengthSlider, noteLengthLabel, "Length", "noteLength");
    setupSlider(velocitySlider, velocityLabel, "Velocity", "velocity");
    setupSlider(swingSlider, swingLabel, "Swing", "swing");
    setupSlider(humanizeSlider, humanizeLabel, "Humanize", "humanize");
    setupSlider(seedSlider, seedLabel, "Seed", "seed");

    // Scale selector
    scaleSelector.addItemList({"Minor Pent", "Major", "Minor", "Dorian", "Chromatic"}, 1);
    addAndMakeVisible(scaleSelector);
    scaleAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
        processorRef.apvts, "scale", scaleSelector);

    scaleLabel.setText("Scale", juce::dontSendNotification);
    scaleLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(scaleLabel);

    // Regenerate button
    regenerateButton.setButtonText("Regenerate");
    regenerateButton.onClick = [this]()
    {
        // Randomize seed
        auto* seedParam = processorRef.apvts.getParameter("seed");
        seedParam->setValueNotifyingHost(juce::Random::getSystemRandom().nextFloat());
    };
    addAndMakeVisible(regenerateButton);

    // Add visualizers
    addAndMakeVisible(circularViz);
    addAndMakeVisible(stepGrid);

    startTimerHz(30); // For UI updates
}

BasslineGeneratorEditor::~BasslineGeneratorEditor()
{
    stopTimer();
}

void BasslineGeneratorEditor::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::darkgrey);

    g.setColour(juce::Colours::white);
    g.setFont(20.0f);
    g.drawText("Bassline Generator", getLocalBounds().removeFromTop(40),
               juce::Justification::centred);
}

void BasslineGeneratorEditor::resized()
{
    auto area = getLocalBounds().reduced(10);
    area.removeFromTop(40); // Title space

    // Top section: Visualizers side by side
    auto vizArea = area.removeFromTop(180);
    int vizSize = 160;

    circularViz.setBounds(vizArea.removeFromLeft(vizSize).reduced(5));
    stepGrid.setBounds(vizArea.reduced(5));

    area.removeFromTop(10);

    int knobSize = 70;
    int labelHeight = 18;

    // Row 1: Rhythm controls
    auto row1 = area.removeFromTop(knobSize + labelHeight);
    auto placeKnob = [&](juce::Slider& slider, juce::Label& label, juce::Rectangle<int>& row)
    {
        auto knobArea = row.removeFromLeft(knobSize + 10);
        label.setBounds(knobArea.removeFromTop(labelHeight));
        slider.setBounds(knobArea);
    };

    placeKnob(stepsSlider, stepsLabel, row1);
    placeKnob(hitsSlider, hitsLabel, row1);
    placeKnob(rotationSlider, rotationLabel, row1);

    area.removeFromTop(10);

    // Row 2: Pitch controls
    auto row2 = area.removeFromTop(knobSize + labelHeight);
    placeKnob(rootNoteSlider, rootNoteLabel, row2);

    auto scaleArea = row2.removeFromLeft(90);
    scaleLabel.setBounds(scaleArea.removeFromTop(labelHeight));
    scaleSelector.setBounds(scaleArea.removeFromTop(24));

    placeKnob(octaveRangeSlider, octaveLabel, row2);

    area.removeFromTop(10);

    // Row 3: Note controls
    auto row3 = area.removeFromTop(knobSize + labelHeight);
    placeKnob(noteLengthSlider, noteLengthLabel, row3);
    placeKnob(velocitySlider, velocityLabel, row3);

    area.removeFromTop(10);

    // Row 4: Groove & randomization
    auto row4 = area.removeFromTop(knobSize + labelHeight);
    placeKnob(swingSlider, swingLabel, row4);
    placeKnob(humanizeSlider, humanizeLabel, row4);
    placeKnob(seedSlider, seedLabel, row4);

    regenerateButton.setBounds(row4.removeFromLeft(100).reduced(5));
}

void BasslineGeneratorEditor::timerCallback()
{
    // Update visualizers with current pattern and playback state
    int steps = processorRef.apvts.getRawParameterValue("steps")->load();
    int hits = processorRef.apvts.getRawParameterValue("hits")->load();
    int rotation = processorRef.apvts.getRawParameterValue("rotation")->load();

    int currentStep = processorRef.patternState.currentStep.load();
    bool isPlaying = processorRef.patternState.isPlaying.load();

    circularViz.setPattern(steps, hits, rotation);
    circularViz.setCurrentStep(currentStep, isPlaying);

    stepGrid.setPattern(steps, hits, rotation);
    stepGrid.setCurrentStep(currentStep, isPlaying);
}
