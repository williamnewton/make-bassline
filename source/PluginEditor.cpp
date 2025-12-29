#include "PluginEditor.h"
#include "utils/MidiPatternExporter.h"
#include "BinaryData.h"

BasslineGeneratorEditor::BasslineGeneratorEditor(BasslineGeneratorProcessor& p)
    : AudioProcessorEditor(&p), processorRef(p)
{
    setSize(800, 480);  // Wider for logo, taller for bigger knobs

    // Make window resizable with constraints
    setResizable(true, true);
    setResizeLimits(700, 420, 1100, 650);

    // Load logo from binary data
    logoImage = juce::ImageCache::getFromMemory(BinaryData::makebasslogo_png, BinaryData::makebasslogo_pngSize);
    logoComponent.setImage(logoImage);
    addAndMakeVisible(logoComponent);

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

    // Essential visible controls (5)
    setupSlider(stepsSlider, stepsLabel, "Steps", "steps");
    setupSlider(densitySlider, densityLabel, "Density", "hits");
    setupSlider(swingSlider, swingLabel, "Swing", "swing");
    setupSlider(rootNoteSlider, rootNoteLabel, "Root", "rootNote");

    // Scale selector
    scaleSelector.addItemList({"Minor Pent", "Major", "Minor", "Dorian", "Chromatic"}, 1);
    addAndMakeVisible(scaleSelector);
    scaleAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
        processorRef.apvts, "scale", scaleSelector);

    scaleLabel.setText("Scale", juce::dontSendNotification);
    scaleLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(scaleLabel);

    // Hidden advanced controls (still functional, just not visible)
    setupSlider(rotationSlider, rotationLabel, "Rotation", "rotation");
    setupSlider(octaveRangeSlider, octaveLabel, "Octaves", "octaveRange");
    setupSlider(noteLengthSlider, noteLengthLabel, "Length", "noteLength");
    setupSlider(velocitySlider, velocityLabel, "Velocity", "velocity");
    setupSlider(humanizeSlider, humanizeLabel, "Humanize", "humanize");
    setupSlider(seedSlider, seedLabel, "Seed", "seed");

    // Regenerate button (hidden)
    regenerateButton.setButtonText("Regenerate");
    regenerateButton.onClick = [this]()
    {
        auto* seedParam = processorRef.apvts.getParameter("seed");
        seedParam->setValueNotifyingHost(juce::Random::getSystemRandom().nextFloat());
    };
    addAndMakeVisible(regenerateButton);

    // Randomize button
    randomizeButton.setButtonText("Randomize");
    randomizeButton.onClick = [this]()
    {
        auto& random = juce::Random::getSystemRandom();

        // Clear any manual step edits when randomizing
        processorRef.clearManualToggles();

        // Get current steps to calculate hits range (but don't change steps)
        int currentSteps = static_cast<int>(processorRef.apvts.getRawParameterValue("steps")->load());

        // Randomize rhythm parameters (excluding steps/length)
        auto* hitsParam = processorRef.apvts.getParameter("hits");
        auto* rotationParam = processorRef.apvts.getParameter("rotation");

        // Hits: 1 to currentSteps (favor 30-70% density)
        int minHits = juce::jmax(1, currentSteps / 3);
        int maxHits = juce::jmin(currentSteps, (currentSteps * 2) / 3);
        int newHits = random.nextInt(juce::Range<int>(minHits, maxHits + 1));
        hitsParam->setValueNotifyingHost((newHits - 1) / 15.0f);

        // Rotation: 0-15
        int newRotation = random.nextInt(16);
        rotationParam->setValueNotifyingHost(newRotation / 15.0f);

        // Randomize scale (sometimes)
        if (random.nextFloat() > 0.5f)
        {
            auto* scaleParam = processorRef.apvts.getParameter("scale");
            int newScale = random.nextInt(5);
            scaleParam->setValueNotifyingHost(newScale / 4.0f);
        }

        // Randomize swing (sometimes, keep it subtle)
        if (random.nextFloat() > 0.6f)
        {
            auto* swingParam = processorRef.apvts.getParameter("swing");
            float newSwing = random.nextFloat() * 0.4f; // 0-40% swing
            swingParam->setValueNotifyingHost(newSwing / 0.75f);
        }

        // Always randomize seed for new pitch variations
        auto* seedParam = processorRef.apvts.getParameter("seed");
        seedParam->setValueNotifyingHost(random.nextFloat());
    };
    addAndMakeVisible(randomizeButton);

    // Add step sequencer grid
    addAndMakeVisible(stepGrid);

    // Setup step click callback for manual editing
    stepGrid.onStepClicked = [this](int step)
    {
        processorRef.toggleStep(step);
    };

    // Setup callback to check if step is manually toggled
    stepGrid.isStepManuallyToggled = [this](int step)
    {
        return processorRef.isStepManuallyToggled(step);
    };

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

    barLengthLabel.setText("MIDI Export", juce::dontSendNotification);
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
    auto bounds = getLocalBounds().toFloat();

    // WHITE BACKGROUND - pop-art/comic style
    g.setColour(juce::Colours::white);
    g.fillAll();

    // Bold comic book frame - thick black outline
    g.setColour(juce::Colours::black);
    g.drawRect(bounds, 8.0f);

    // Inner bold outline (double-line comic style)
    g.drawRect(bounds.reduced(12), 4.0f);

    // Pop-art accent dots in corners - graffiti style
    auto drawAccentDot = [&](float x, float y)
    {
        // Yellow dot with black outline
        g.setColour(juce::Colours::black);
        g.fillEllipse(x - 5, y - 5, 10, 10);

        g.setColour(juce::Colour(0xffffdd00));
        g.fillEllipse(x - 4, y - 4, 8, 8);
    };

    float dotInset = 20.0f;
    drawAccentDot(dotInset, dotInset);                                      // Top-left
    drawAccentDot(bounds.getWidth() - dotInset, dotInset);                 // Top-right
    drawAccentDot(dotInset, bounds.getHeight() - dotInset);                // Bottom-left
    drawAccentDot(bounds.getWidth() - dotInset, bounds.getHeight() - dotInset); // Bottom-right
}

void BasslineGeneratorEditor::resized()
{
    auto area = getLocalBounds().reduced(12);

    // Title and export area at top
    auto topArea = area.removeFromTop(45);

    // Top-right controls: Randomize, Bar selector, Export
    auto topRightArea = topArea.removeFromRight(340);
    midiDragArea.setBounds(topRightArea.removeFromRight(110).reduced(8, 8));
    barLengthSelector.setBounds(topRightArea.removeFromRight(100).reduced(5, 12));
    randomizeButton.setBounds(topRightArea.removeFromRight(100).reduced(5, 10));

    // Hide the label
    barLengthLabel.setBounds(0, 0, 0, 0);

    // Sequencer and logo area - same height, side by side
    auto vizArea = area.removeFromTop(160);

    // Logo on the left, same height as sequencer
    auto logoArea = vizArea.removeFromLeft(200);
    logoComponent.setBounds(logoArea.reduced(8, 4));

    // Sequencer on the right, takes remaining width
    stepGrid.setBounds(vizArea.reduced(8, 4));

    area.removeFromTop(20);

    // Controls area - 5 essential controls in single row
    int knobSize = 90;  // Bigger knobs
    int labelHeight = 24;
    int knobSpacing = (area.getWidth() - (5 * knobSize)) / 6; // 5 controls evenly spaced

    auto placeKnob = [&](juce::Slider& slider, juce::Label& label, juce::Rectangle<int>& row, int index)
    {
        int xPos = knobSpacing + index * (knobSize + knobSpacing);
        auto knobArea = juce::Rectangle<int>(row.getX() + xPos, row.getY(), knobSize, row.getHeight());
        label.setBounds(knobArea.removeFromTop(labelHeight));
        slider.setBounds(knobArea.removeFromBottom(knobSize));
    };

    // Row 1: Steps, Density, Swing
    auto row1 = area.removeFromTop(knobSize + labelHeight);
    placeKnob(stepsSlider, stepsLabel, row1, 0);
    placeKnob(densitySlider, densityLabel, row1, 1);
    placeKnob(swingSlider, swingLabel, row1, 2);

    // Root note
    placeKnob(rootNoteSlider, rootNoteLabel, row1, 3);

    // Scale selector (special case - dropdown instead of knob)
    int xPos = knobSpacing + 4 * (knobSize + knobSpacing);
    auto scaleArea = juce::Rectangle<int>(row1.getX() + xPos, row1.getY(), knobSize, row1.getHeight());
    scaleLabel.setBounds(scaleArea.removeFromTop(labelHeight));
    scaleSelector.setBounds(scaleArea.removeFromTop(26).reduced(3, 0));

    // Hide all advanced controls (still functional, just not visible)
    rotationSlider.setBounds(0, 0, 0, 0);
    rotationLabel.setBounds(0, 0, 0, 0);
    octaveRangeSlider.setBounds(0, 0, 0, 0);
    octaveLabel.setBounds(0, 0, 0, 0);
    noteLengthSlider.setBounds(0, 0, 0, 0);
    noteLengthLabel.setBounds(0, 0, 0, 0);
    velocitySlider.setBounds(0, 0, 0, 0);
    velocityLabel.setBounds(0, 0, 0, 0);
    humanizeSlider.setBounds(0, 0, 0, 0);
    humanizeLabel.setBounds(0, 0, 0, 0);
    seedSlider.setBounds(0, 0, 0, 0);
    seedLabel.setBounds(0, 0, 0, 0);
    regenerateButton.setBounds(0, 0, 0, 0);
}

void BasslineGeneratorEditor::timerCallback()
{
    // Update step grid with current pattern and playback state
    int steps = processorRef.apvts.getRawParameterValue("steps")->load();
    int hits = processorRef.apvts.getRawParameterValue("hits")->load();
    int rotation = processorRef.apvts.getRawParameterValue("rotation")->load();

    int currentStep = processorRef.patternState.currentStep.load();
    bool isPlaying = processorRef.patternState.isPlaying.load();

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
