#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include "../generator/EuclideanRhythm.h"
#include "../generator/PitchGenerator.h"

class PitchRhythmVisualizer : public juce::Component
{
public:
    PitchRhythmVisualizer() = default;

    ~PitchRhythmVisualizer() override = default;

    void paint(juce::Graphics& g) override
    {
        auto bounds = getLocalBounds().toFloat();

        // Black background with dark red tint
        g.setColour(juce::Colour(0xff0a0000));
        g.fillRoundedRectangle(bounds, 8.0f);

        // Comic book border - black outline
        g.setColour(juce::Colours::black);
        g.drawRoundedRectangle(bounds, 8.0f, 3.0f);

        // Yellow inner border
        g.setColour(juce::Colour(0xffffdd00));
        g.drawRoundedRectangle(bounds.reduced(3), 8.0f, 2.0f);

        // Draw title
        g.setFont(11.0f);
        g.setColour(juce::Colour(0xffffdd00).withAlpha(0.7f));
        g.drawText("PATTERN VISUALIZER", bounds.removeFromTop(20).reduced(10, 2),
                   juce::Justification::centredLeft);

        // Safety checks
        if (cachedPitches.empty() || cachedSteps <= 0)
        {
            // Show "waiting for pattern" message
            g.setFont(14.0f);
            g.setColour(juce::Colour(0xffffdd00).withAlpha(0.5f));
            g.drawText("Waiting for pattern...", bounds, juce::Justification::centred);
            return;
        }

        auto vizArea = bounds.reduced(15);
        float stepWidth = vizArea.getWidth() / static_cast<float>(cachedSteps);

        // Draw each step
        for (int i = 0; i < cachedSteps && i < static_cast<int>(cachedPitches.size()); ++i)
        {
            float xPos = vizArea.getX() + i * stepWidth;
            auto stepBounds = juce::Rectangle<float>(xPos, vizArea.getY(), stepWidth - 4, vizArea.getHeight() - 10);

            bool hasHit = (cachedPitches[i] >= 0);
            bool isCurrentStep = (i == currentStep && isPlaying);

            if (hasHit)
            {
                // Calculate normalized height (0.0 to 1.0)
                float normalizedHeight = cachedNormalizedHeights[i];
                float barHeight = 30.0f + (normalizedHeight * (stepBounds.getHeight() - 30.0f));

                auto barBounds = stepBounds.removeFromBottom(barHeight).reduced(2);

                if (isCurrentStep)
                {
                    // Current playing step - bright glow
                    g.setColour(juce::Colour(0xffffdd00).withAlpha(0.3f));
                    g.fillRoundedRectangle(barBounds.expanded(3), 5.0f);
                }

                // Black outline
                g.setColour(juce::Colours::black);
                g.fillRoundedRectangle(barBounds.expanded(1.5f), 5.0f);

                // Bar color - yellow for root, red for others
                bool isRoot = cachedIsRootNote[i];
                g.setColour(isRoot ? juce::Colour(0xffffdd00) : juce::Colour(0xffff0000));
                g.fillRoundedRectangle(barBounds, 5.0f);

                // Top highlight
                g.setColour(juce::Colours::white.withAlpha(0.3f));
                auto highlightBounds = barBounds.removeFromTop(barBounds.getHeight() * 0.3f);
                g.fillRoundedRectangle(highlightBounds, 5.0f);
            }
            else
            {
                // Empty step - small indicator at bottom
                auto emptyBounds = stepBounds.removeFromBottom(12).reduced(3);

                if (isCurrentStep)
                {
                    g.setColour(juce::Colour(0xffffdd00).withAlpha(0.5f));
                }
                else
                {
                    g.setColour(juce::Colour(0xff330000));
                }
                g.fillRoundedRectangle(emptyBounds, 3.0f);
            }
        }
    }

    void setPattern(int steps, int hits, int rot, int root, int scale, int octaves, int randomSeed)
    {
        // Only recalculate if something changed
        if (cachedSteps == steps && cachedHits == hits && cachedRotation == rot &&
            cachedRoot == root && cachedScale == scale && cachedOctaves == octaves &&
            cachedSeed == randomSeed)
        {
            return; // No change, skip recalculation
        }

        // Store parameters
        cachedSteps = steps;
        cachedHits = hits;
        cachedRotation = rot;
        cachedRoot = root;
        cachedScale = scale;
        cachedOctaves = octaves;
        cachedSeed = randomSeed;

        // Clear and pre-allocate
        cachedPitches.clear();
        cachedNormalizedHeights.clear();
        cachedIsRootNote.clear();

        if (steps <= 0 || steps > 16)
            return;

        cachedPitches.reserve(steps);
        cachedNormalizedHeights.reserve(steps);
        cachedIsRootNote.reserve(steps);

        // Generate pattern data
        int minPitch = 127;
        int maxPitch = 0;

        // First pass: collect pitches
        for (int i = 0; i < steps; ++i)
        {
            if (euclidean.shouldTrigger(i, steps, hits, rot))
            {
                int pitch = pitchGen.generatePitch(i, root, scale, octaves, randomSeed);
                pitch = juce::jlimit(0, 127, pitch);
                cachedPitches.push_back(pitch);
                minPitch = std::min(minPitch, pitch);
                maxPitch = std::max(maxPitch, pitch);
            }
            else
            {
                cachedPitches.push_back(-1); // No note
            }
        }

        // Ensure valid range
        if (minPitch > maxPitch)
        {
            minPitch = root;
            maxPitch = root + 12;
        }

        int pitchRange = std::max(1, maxPitch - minPitch);

        // Second pass: calculate normalized heights and root notes
        for (int i = 0; i < steps; ++i)
        {
            if (cachedPitches[i] >= 0)
            {
                float normalized = static_cast<float>(cachedPitches[i] - minPitch) / static_cast<float>(pitchRange);
                cachedNormalizedHeights.push_back(juce::jlimit(0.0f, 1.0f, normalized));
                cachedIsRootNote.push_back(((cachedPitches[i] - root) % 12) == 0);
            }
            else
            {
                cachedNormalizedHeights.push_back(0.0f);
                cachedIsRootNote.push_back(false);
            }
        }

        repaint();
    }

    void setCurrentStep(int step, bool playing)
    {
        if (currentStep != step || isPlaying != playing)
        {
            currentStep = step;
            isPlaying = playing;
            repaint();
        }
    }

private:
    EuclideanRhythm euclidean;
    PitchGenerator pitchGen;

    // Cached pattern data (pre-calculated in setPattern)
    std::vector<int> cachedPitches;
    std::vector<float> cachedNormalizedHeights;
    std::vector<bool> cachedIsRootNote;

    int cachedSteps = 0;
    int cachedHits = 0;
    int cachedRotation = 0;
    int cachedRoot = 36;
    int cachedScale = 0;
    int cachedOctaves = 1;
    int cachedSeed = 42;

    int currentStep = 0;
    bool isPlaying = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PitchRhythmVisualizer)
};
