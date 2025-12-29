#include "PluginEditor.h"
#include "utils/MidiPatternExporter.h"

BasslineGeneratorEditor::BasslineGeneratorEditor(BasslineGeneratorProcessor& p)
    : AudioProcessorEditor(&p), processorRef(p)
{
    setSize(650, 450);

    // Make window resizable with constraints
    setResizable(true, true);
    setResizeLimits(550, 380, 900, 600);

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

        // Randomize rhythm parameters
        auto* stepsParam = processorRef.apvts.getParameter("steps");
        auto* hitsParam = processorRef.apvts.getParameter("hits");
        auto* rotationParam = processorRef.apvts.getParameter("rotation");

        // Steps: 4-16
        int newSteps = random.nextInt(juce::Range<int>(4, 17));
        stepsParam->setValueNotifyingHost((newSteps - 4) / 12.0f);

        // Hits: 1 to newSteps (favor 30-70% density)
        int minHits = juce::jmax(1, newSteps / 3);
        int maxHits = juce::jmin(newSteps, (newSteps * 2) / 3);
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

    // Outer decorative frame - metallic dark look
    auto frameBounds = bounds;

    // Outer black border
    g.setColour(juce::Colours::black);
    g.fillRoundedRectangle(frameBounds, 8.0f);

    // Metallic dark grey gradient for frame
    auto frameGradient = juce::ColourGradient(juce::Colour(0xff2a2a2a), 0, 0,
                                              juce::Colour(0xff1a1a1a), 0, static_cast<float>(getHeight()),
                                              false);
    g.setGradientFill(frameGradient);
    g.fillRoundedRectangle(frameBounds.reduced(2), 7.0f);

    // Inner decorative ridge - top highlight
    g.setColour(juce::Colour(0xff444444));
    g.drawRoundedRectangle(frameBounds.reduced(6), 6.0f, 1.5f);

    // Inner shadow
    g.setColour(juce::Colours::black.withAlpha(0.6f));
    g.drawRoundedRectangle(frameBounds.reduced(8), 5.0f, 1.0f);

    // Red accent line
    g.setColour(juce::Colour(0xffff0000).withAlpha(0.3f));
    g.drawRoundedRectangle(frameBounds.reduced(7), 5.5f, 0.5f);

    // Content area - black to dark red gradient background
    auto contentBounds = frameBounds.reduced(10);
    auto gradient = juce::ColourGradient(juce::Colour(0xff000000), 0, 0,
                                         juce::Colour(0xff1a0000), 0, static_cast<float>(getHeight()),
                                         false);
    g.setGradientFill(gradient);
    g.fillRoundedRectangle(contentBounds, 4.0f);

    auto titleBounds = getLocalBounds().removeFromTop(50).reduced(10, 5);

    // Comic book title with bold outline
    g.setFont(juce::Font(36.0f, juce::Font::bold | juce::Font::italic));

    // Black outline
    g.setColour(juce::Colours::black);
    for (int x = -3; x <= 3; ++x)
        for (int y = -3; y <= 3; ++y)
            if (x != 0 || y != 0)
                g.drawText("MAKE BASSLINE", titleBounds.translated(x, y),
                          juce::Justification::centred, true);

    // Red fill
    g.setColour(juce::Colour(0xffff0000));
    g.drawText("MAKE BASSLINE", titleBounds,
               juce::Justification::centred, true);

    // Yellow highlight
    g.setColour(juce::Colour(0xffffdd00).withAlpha(0.6f));
    g.drawText("MAKE BASSLINE", titleBounds.translated(-1, -1),
               juce::Justification::centred, true);

    // Corner embellishments - decorative bolts/screws
    auto drawCornerBolt = [&](float x, float y)
    {
        juce::Rectangle<float> bolt(x - 4, y - 4, 8, 8);

        // Outer ring
        g.setColour(juce::Colour(0xff1a1a1a));
        g.fillEllipse(bolt);

        // Inner circle
        g.setColour(juce::Colour(0xff333333));
        g.fillEllipse(bolt.reduced(1.5f));

        // Highlight
        g.setColour(juce::Colour(0xff555555));
        g.fillEllipse(bolt.reduced(2.5f).translated(-0.5f, -0.5f));

        // Cross screw detail
        g.setColour(juce::Colours::black.withAlpha(0.5f));
        g.drawLine(x - 2, y, x + 2, y, 0.8f);
        g.drawLine(x, y - 2, x, y + 2, 0.8f);
    };

    float boltInset = 12.0f;
    drawCornerBolt(boltInset, boltInset);                                    // Top-left
    drawCornerBolt(bounds.getWidth() - boltInset, boltInset);               // Top-right
    drawCornerBolt(boltInset, bounds.getHeight() - boltInset);              // Bottom-left
    drawCornerBolt(bounds.getWidth() - boltInset, bounds.getHeight() - boltInset); // Bottom-right

    // Resize grip indicator in bottom-right corner
    auto gripArea = juce::Rectangle<float>(bounds.getWidth() - 20, bounds.getHeight() - 20, 15, 15);
    g.setColour(juce::Colour(0xff333333));
    for (int i = 0; i < 3; ++i)
    {
        float offset = i * 4.0f;
        g.drawLine(gripArea.getX() + 12 - offset, gripArea.getY() + offset,
                   gripArea.getX() + offset, gripArea.getY() + 12 - offset, 1.5f);
    }
}

void BasslineGeneratorEditor::resized()
{
    auto area = getLocalBounds().reduced(12);

    // Title and export area at top
    auto topArea = area.removeFromTop(45);

    // Top-right controls: Randomize, Bar selector, Export
    auto topRightArea = topArea.removeFromRight(360);
    midiDragArea.setBounds(topRightArea.removeFromRight(100).reduced(0, 8));
    barLengthSelector.setBounds(topRightArea.removeFromRight(110).reduced(5, 12));
    randomizeButton.setBounds(topRightArea.removeFromRight(100).reduced(5, 10));

    // Hide the label
    barLengthLabel.setBounds(0, 0, 0, 0);

    // Large step sequencer grid - more prominence
    auto vizArea = area.removeFromTop(160);
    stepGrid.setBounds(vizArea);

    area.removeFromTop(20);

    // Controls area - 5 essential controls in single row
    int knobSize = 72;
    int labelHeight = 22;
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
