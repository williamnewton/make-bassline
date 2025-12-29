#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <random>

//==============================================================================
BasslineGeneratorProcessor::BasslineGeneratorProcessor()
    : AudioProcessor(BusesProperties()
#if !JucePlugin_IsMidiEffect
#if !JucePlugin_IsSynth
                         .withInput("Input", juce::AudioChannelSet::stereo(), true)
#endif
                         .withOutput("Output", juce::AudioChannelSet::stereo(), true)
#endif
                         ),
      apvts(*this, nullptr, "Parameters", createParameterLayout())
{
    // Initialize manual toggles to false
    for (auto& toggle : manualToggles)
        toggle.store(false);
}

BasslineGeneratorProcessor::~BasslineGeneratorProcessor()
{
}

//==============================================================================
juce::AudioProcessorValueTreeState::ParameterLayout
BasslineGeneratorProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    // Euclidean rhythm parameters
    params.push_back(std::make_unique<juce::AudioParameterInt>(
        "steps", "Steps", 4, 16, 8));
    params.push_back(std::make_unique<juce::AudioParameterInt>(
        "hits", "Hits", 1, 16, 3));
    params.push_back(std::make_unique<juce::AudioParameterInt>(
        "rotation", "Rotation", 0, 15, 0));

    // Pitch parameters
    params.push_back(std::make_unique<juce::AudioParameterInt>(
        "rootNote", "Root Note", 24, 48, 36)); // C1 to C3, default C2
    params.push_back(std::make_unique<juce::AudioParameterChoice>(
        "scale", "Scale",
        juce::StringArray{"Minor Pentatonic", "Major", "Minor", "Dorian", "Chromatic"},
        0));
    params.push_back(std::make_unique<juce::AudioParameterInt>(
        "octaveRange", "Octave Range", 1, 2, 1));

    // Note parameters
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "noteLength", "Note Length", 0.1f, 1.0f, 0.5f)); // Fraction of step
    params.push_back(std::make_unique<juce::AudioParameterInt>(
        "velocity", "Velocity", 1, 127, 100));

    // Groove parameters
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "swing", "Swing", 0.0f, 0.75f, 0.0f)); // 0 = no swing, 0.75 = max swing
    params.push_back(std::make_unique<juce::AudioParameterInt>(
        "humanize", "Humanize", 0, 30, 0)); // Velocity randomization amount

    // Pattern parameters
    params.push_back(std::make_unique<juce::AudioParameterInt>(
        "seed", "Random Seed", 0, 9999, 42));
    params.push_back(std::make_unique<juce::AudioParameterBool>(
        "regenerate", "Regenerate", false));

    return {params.begin(), params.end()};
}

//==============================================================================
void BasslineGeneratorProcessor::prepareToPlay(double sampleRate, int /*samplesPerBlock*/)
{
    currentSampleRate = sampleRate;
    lastPpqPosition = -1;
    currentStep = 0;
    activeNote = -1;
}

void BasslineGeneratorProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

void BasslineGeneratorProcessor::processBlock(juce::AudioBuffer<float>& buffer,
                                               juce::MidiBuffer& midiMessages)
{
    // Clear any incoming MIDI (we're generating, not processing)
    midiMessages.clear();

    // Get host playhead info
    auto* playHead = getPlayHead();
    if (playHead == nullptr)
        return;

    auto posInfo = playHead->getPosition();
    if (!posInfo.hasValue())
        return;

    // Only generate when playing
    if (!posInfo->getIsPlaying())
    {
        // Send note-off if we were playing a note
        if (activeNote >= 0)
        {
            midiMessages.addEvent(juce::MidiMessage::noteOff(1, activeNote), 0);
            activeNote = -1;
        }
        lastPpqPosition = -1;
        patternState.isPlaying.store(false);
        return;
    }

    patternState.isPlaying.store(true);

    // Get timing info
    auto bpm = posInfo->getBpm().orFallback(120.0);
    auto ppqPosition = posInfo->getPpqPosition().orFallback(0.0);
    auto timeSignature = posInfo->getTimeSignature().orFallback(
        juce::AudioPlayHead::TimeSignature{4, 4});

    // Calculate timing values
    int numSteps = apvts.getRawParameterValue("steps")->load();
    double beatsPerBar = timeSignature.numerator;
    double ppqPerStep = beatsPerBar / numSteps; // Subdivide bar into steps
    double samplesPerBeat = (currentSampleRate * 60.0) / bpm;
    double samplesPerStep = samplesPerBeat * ppqPerStep;

    // Get swing amount (0.0 to 0.75)
    float swingAmount = apvts.getRawParameterValue("swing")->load();

    // Get note length
    float noteLengthFraction = apvts.getRawParameterValue("noteLength")->load();
    noteDurationSamples = static_cast<int>(samplesPerStep * noteLengthFraction);

    // Process each sample in the buffer
    int numSamples = buffer.getNumSamples();

    for (int sample = 0; sample < numSamples; ++sample)
    {
        // Calculate PPQ at this sample
        double samplePpq = ppqPosition + (sample * bpm) / (currentSampleRate * 60.0);

        // Determine which step we're on (accounting for swing)
        double barPosition = std::fmod(samplePpq, beatsPerBar);

        // Apply swing: delay even steps
        double adjustedBarPosition = barPosition;
        if (swingAmount > 0.0f)
        {
            double stepProgress = barPosition / ppqPerStep;
            int baseStep = static_cast<int>(stepProgress);
            double fractionalStep = stepProgress - baseStep;

            // If we're on an even step, delay it
            if (baseStep % 2 == 1)
            {
                adjustedBarPosition = barPosition + (ppqPerStep * swingAmount * fractionalStep);
            }
        }

        int step = static_cast<int>(adjustedBarPosition / ppqPerStep) % numSteps;

        // Detect step change (new step triggered)
        if (step != currentStep)
        {
            currentStep = step;
            patternState.currentStep.store(step);

            // Check if this step should trigger a note (Euclidean pattern)
            int hits = apvts.getRawParameterValue("hits")->load();
            int rotation = apvts.getRawParameterValue("rotation")->load();

            bool shouldTrigger = euclidean.shouldTrigger(step, numSteps, hits, rotation);

            // Apply manual toggles if present
            if (hasManualToggles.load() && step < 16)
            {
                if (manualToggles[step].load())
                    shouldTrigger = !shouldTrigger; // Toggle the state
            }

            if (shouldTrigger)
            {
                // Send note-off for previous note if active
                if (activeNote >= 0)
                {
                    midiMessages.addEvent(
                        juce::MidiMessage::noteOff(1, activeNote), sample);
                }

                // Generate pitch
                int rootNote = apvts.getRawParameterValue("rootNote")->load();
                int scaleIndex = apvts.getRawParameterValue("scale")->load();
                int octaveRange = apvts.getRawParameterValue("octaveRange")->load();
                int seed = apvts.getRawParameterValue("seed")->load();

                int pitch = pitchGen.generatePitch(
                    rootNote, scaleIndex, octaveRange, step, seed);

                // Apply velocity with humanization
                int baseVelocity = apvts.getRawParameterValue("velocity")->load();
                int humanizeAmount = apvts.getRawParameterValue("humanize")->load();

                int velocity = baseVelocity;
                if (humanizeAmount > 0)
                {
                    // Use seed + step for deterministic but varied humanization
                    std::mt19937 rng(seed + step * 251);
                    std::uniform_int_distribution<int> velDist(-humanizeAmount, humanizeAmount);
                    velocity = juce::jlimit(1, 127, baseVelocity + velDist(rng));
                }

                // Send note-on
                midiMessages.addEvent(
                    juce::MidiMessage::noteOn(1, pitch, (juce::uint8)velocity),
                    sample);

                activeNote = pitch;
                samplesUntilNoteOff = noteDurationSamples;
            }
        }

        // Handle note-off timing
        if (activeNote >= 0)
        {
            samplesUntilNoteOff--;
            if (samplesUntilNoteOff <= 0)
            {
                midiMessages.addEvent(
                    juce::MidiMessage::noteOff(1, activeNote), sample);
                activeNote = -1;
            }
        }
    }
}

//==============================================================================
juce::AudioProcessorEditor* BasslineGeneratorProcessor::createEditor()
{
    return new BasslineGeneratorEditor(*this);
}

//==============================================================================
void BasslineGeneratorProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void BasslineGeneratorProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xml(getXmlFromBinary(data, sizeInBytes));
    if (xml != nullptr && xml->hasTagName(apvts.state.getType()))
    {
        apvts.replaceState(juce::ValueTree::fromXml(*xml));
    }
}

//==============================================================================
// Manual pattern editing
void BasslineGeneratorProcessor::toggleStep(int step)
{
    if (step >= 0 && step < 16)
    {
        bool currentState = manualToggles[step].load();
        manualToggles[step].store(!currentState);
        hasManualToggles.store(true);
    }
}

bool BasslineGeneratorProcessor::isStepManuallyToggled(int step) const
{
    if (step >= 0 && step < 16)
        return manualToggles[step].load();
    return false;
}

void BasslineGeneratorProcessor::clearManualToggles()
{
    for (auto& toggle : manualToggles)
        toggle.store(false);
    hasManualToggles.store(false);
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new BasslineGeneratorProcessor();
}
