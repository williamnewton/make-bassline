#pragma once
#include <juce_audio_basics/juce_audio_basics.h>
#include "../generator/EuclideanRhythm.h"
#include "../generator/PitchGenerator.h"
#include <random>

class MidiPatternExporter
{
public:
    struct PatternParams
    {
        int steps = 8;
        int hits = 3;
        int rotation = 0;
        int rootNote = 36;
        int scaleIndex = 0;
        int octaveRange = 1;
        int velocity = 100;
        float noteLength = 0.5f;
        float swing = 0.0f;
        int humanize = 0;
        int seed = 42;
        int numBars = 1;
        double bpm = 120.0;
        int timeSignatureNumerator = 4;
    };

    static juce::MidiFile generatePattern(const PatternParams& params)
    {
        juce::MidiFile midiFile;
        midiFile.setTicksPerQuarterNote(960); // Standard MIDI resolution

        juce::MidiMessageSequence sequence;

        EuclideanRhythm euclidean;
        PitchGenerator pitchGen;

        // Calculate timing
        double beatsPerBar = params.timeSignatureNumerator;
        double totalBeats = beatsPerBar * params.numBars;
        double beatsPerStep = beatsPerBar / params.steps;
        double ticksPerBeat = 960.0;
        double ticksPerStep = ticksPerBeat * beatsPerStep;

        // Generate pattern for all bars
        for (int bar = 0; bar < params.numBars; ++bar)
        {
            for (int step = 0; step < params.steps; ++step)
            {
                // Check if this step should trigger
                bool shouldTrigger = euclidean.shouldTrigger(
                    step, params.steps, params.hits, params.rotation);

                if (shouldTrigger)
                {
                    // Calculate base timestamp
                    double baseTimestamp = (bar * params.steps + step) * ticksPerStep;

                    // Apply swing to odd steps
                    double timestamp = baseTimestamp;
                    if (params.swing > 0.0f && step % 2 == 1)
                    {
                        timestamp += ticksPerStep * params.swing;
                    }

                    // Generate pitch
                    int pitch = pitchGen.generatePitch(
                        params.rootNote,
                        params.scaleIndex,
                        params.octaveRange,
                        step,
                        params.seed
                    );

                    // Apply humanization to velocity
                    int velocity = params.velocity;
                    if (params.humanize > 0)
                    {
                        std::mt19937 rng(params.seed + step * 251);
                        std::uniform_int_distribution<int> velDist(-params.humanize, params.humanize);
                        velocity = juce::jlimit(1, 127, params.velocity + velDist(rng));
                    }

                    // Calculate note duration
                    double noteDuration = ticksPerStep * params.noteLength;

                    // Add note on
                    sequence.addEvent(
                        juce::MidiMessage::noteOn(1, pitch, (juce::uint8)velocity),
                        timestamp
                    );

                    // Add note off
                    sequence.addEvent(
                        juce::MidiMessage::noteOff(1, pitch),
                        timestamp + noteDuration
                    );
                }
            }
        }

        // Update sequence end time
        sequence.updateMatchedPairs();

        // Add sequence to MIDI file
        midiFile.addTrack(sequence);

        // Set tempo
        juce::MidiMessageSequence tempoTrack;
        tempoTrack.addEvent(
            juce::MidiMessage::tempoMetaEvent(static_cast<int>(60000000.0 / params.bpm)),
            0.0
        );
        midiFile.addTrack(tempoTrack);

        return midiFile;
    }

    static juce::MemoryBlock exportToMemory(const PatternParams& params)
    {
        auto midiFile = generatePattern(params);

        juce::MemoryOutputStream outStream;
        midiFile.writeTo(outStream);

        return outStream.getMemoryBlock();
    }

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MidiPatternExporter)
};
