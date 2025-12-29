#include "PluginEditor.h"
#include "utils/MidiPatternExporter.h"

BasslineGeneratorEditor::BasslineGeneratorEditor(BasslineGeneratorProcessor& p)
    : AudioProcessorEditor(&p), processorRef(p)
{
    setSize(850, 620);

    // Apply comic book look and feel to all components
    setLookAndFeel(&comicLookAndFeel);

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

    // Setup MIDI drag area
    addAndMakeVisible(midiDragArea);
    midiDragArea.onCreatePattern = [this]() { return createMidiPattern(); };

    // Setup bar length selector
    barLengthSelector.addItemList({"1 Bar", "2 Bars", "4 Bars", "8 Bars"}, 1);
    barLengthSelector.setSelectedId(1, juce::dontSendNotification);
    barLengthSelector.onChange = [this]()
    {
        int bars = 1;
        switch (barLengthSelector.getSelectedId())
        {
            case 1: bars = 1; break;
            case 2: bars = 2; break;
            case 3: bars = 4; break;
            case 4: bars = 8; break;
        }
        midiDragArea.setNumBars(bars);
    };
    addAndMakeVisible(barLengthSelector);

    barLengthLabel.setText("Export", juce::dontSendNotification);
    barLengthLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(barLengthLabel);

    startTimerHz(30); // For UI updates
}

BasslineGeneratorEditor::~BasslineGeneratorEditor()
{
    setLookAndFeel(nullptr);
    stopTimer();
}

void BasslineGeneratorEditor::paint(juce::Graphics& g)
{
    // Comic book gradient background
    auto gradient = juce::ColourGradient(juce::Colour(0xff0f3460), 0, 0,
                                         juce::Colour(0xff1a1a2e), 0, static_cast<float>(getHeight()),
                                         false);
    g.setGradientFill(gradient);
    g.fillAll();

    auto titleBounds = getLocalBounds().removeFromTop(50).reduced(10, 5);

    // Comic book title with bold outline
    g.setFont(juce::Font(32.0f, juce::Font::bold | juce::Font::italic));

    // Black outline
    g.setColour(juce::Colours::black);
    for (int x = -2; x <= 2; ++x)
        for (int y = -2; y <= 2; ++y)
            g.drawText("BASSLINE GENERATOR", titleBounds.translated(x, y),
                      juce::Justification::centred, true);

    // Yellow fill
    g.setColour(juce::Colour(0xffffea00));
    g.drawText("BASSLINE GENERATOR", titleBounds,
               juce::Justification::centred, true);

    // White highlight
    g.setColour(juce::Colours::white.withAlpha(0.4f));
    g.drawText("BASSLINE GENERATOR", titleBounds.translated(-1, -1),
               juce::Justification::centred, true);
}

void BasslineGeneratorEditor::resized()
{
    auto area = getLocalBounds().reduced(12);
    area.removeFromTop(50); // Title space

    // Step sequencer at top (full width)
    auto stepArea = area.removeFromTop(100);
    stepGrid.setBounds(stepArea);

    area.removeFromTop(15);

    // Two rows of perfectly aligned knobs
    int knobSize = 90;
    int labelHeight = 22;
    int knobSpacing = (area.getWidth() - (6 * knobSize)) / 7; // Space evenly across width

    auto placeKnob = [&](juce::Slider& slider, juce::Label& label, juce::Rectangle<int>& row, int index)
    {
        int xPos = knobSpacing + index * (knobSize + knobSpacing);
        auto knobArea = juce::Rectangle<int>(row.getX() + xPos, row.getY(), knobSize, row.getHeight());
        label.setBounds(knobArea.removeFromTop(labelHeight));
        slider.setBounds(knobArea.removeFromBottom(knobSize));
    };

    // ROW 1: Euclidean Rhythm + Pitch
    auto row1 = area.removeFromTop(knobSize + labelHeight);
    placeKnob(stepsSlider, stepsLabel, row1, 0);
    placeKnob(hitsSlider, hitsLabel, row1, 1);
    placeKnob(rotationSlider, rotationLabel, row1, 2);
    placeKnob(rootNoteSlider, rootNoteLabel, row1, 3);

    // Scale selector (special case)
    int xPos = knobSpacing + 4 * (knobSize + knobSpacing);
    auto scaleArea = juce::Rectangle<int>(row1.getX() + xPos, row1.getY(), knobSize, row1.getHeight());
    scaleLabel.setBounds(scaleArea.removeFromTop(labelHeight));
    scaleSelector.setBounds(scaleArea.removeFromTop(28).reduced(5, 0));

    placeKnob(octaveRangeSlider, octaveLabel, row1, 5);

    area.removeFromTop(15);

    // ROW 2: Note controls + Groove
    auto row2 = area.removeFromTop(knobSize + labelHeight);
    placeKnob(noteLengthSlider, noteLengthLabel, row2, 0);
    placeKnob(velocitySlider, velocityLabel, row2, 1);
    placeKnob(swingSlider, swingLabel, row2, 2);
    placeKnob(humanizeSlider, humanizeLabel, row2, 3);
    placeKnob(seedSlider, seedLabel, row2, 4);

    // Regenerate button
    xPos = knobSpacing + 5 * (knobSize + knobSpacing);
    auto regenArea = juce::Rectangle<int>(row2.getX() + xPos, row2.getY() + labelHeight, knobSize, knobSize);
    regenerateButton.setBounds(regenArea.reduced(5));

    area.removeFromTop(15);

    // MIDI Export at bottom
    auto exportArea = area.removeFromTop(60);
    int totalExportWidth = 400;
    int exportX = (area.getWidth() - totalExportWidth) / 2 + area.getX();

    midiDragArea.setBounds(exportX, exportArea.getY(), 200, 60);
    barLengthLabel.setBounds(exportX + 210, exportArea.getY() + 10, 70, 20);
    barLengthSelector.setBounds(exportX + 210, exportArea.getY() + 32, 120, 24);

    // Hide circular viz for cleaner layout
    circularViz.setBounds(0, 0, 0, 0);
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

juce::MemoryBlock BasslineGeneratorEditor::createMidiPattern()
{
    MidiPatternExporter::PatternParams params;

    // Get all current parameters
    params.steps = static_cast<int>(processorRef.apvts.getRawParameterValue("steps")->load());
    params.hits = static_cast<int>(processorRef.apvts.getRawParameterValue("hits")->load());
    params.rotation = static_cast<int>(processorRef.apvts.getRawParameterValue("rotation")->load());
    params.rootNote = static_cast<int>(processorRef.apvts.getRawParameterValue("rootNote")->load());
    params.scaleIndex = static_cast<int>(processorRef.apvts.getRawParameterValue("scale")->load());
    params.octaveRange = static_cast<int>(processorRef.apvts.getRawParameterValue("octaveRange")->load());
    params.velocity = static_cast<int>(processorRef.apvts.getRawParameterValue("velocity")->load());
    params.noteLength = processorRef.apvts.getRawParameterValue("noteLength")->load();
    params.swing = processorRef.apvts.getRawParameterValue("swing")->load();
    params.humanize = static_cast<int>(processorRef.apvts.getRawParameterValue("humanize")->load());
    params.seed = static_cast<int>(processorRef.apvts.getRawParameterValue("seed")->load());

    // Get number of bars from selector
    switch (barLengthSelector.getSelectedId())
    {
        case 1: params.numBars = 1; break;
        case 2: params.numBars = 2; break;
        case 3: params.numBars = 4; break;
        case 4: params.numBars = 8; break;
        default: params.numBars = 1; break;
    }

    // Get tempo and time signature from host if available
    if (auto* playHead = processorRef.getPlayHead())
    {
        if (auto posInfo = playHead->getPosition())
        {
            params.bpm = posInfo->getBpm().orFallback(120.0);
            if (auto timeSig = posInfo->getTimeSignature())
            {
                params.timeSignatureNumerator = timeSig->numerator;
            }
        }
    }

    return MidiPatternExporter::exportToMemory(params);
}
