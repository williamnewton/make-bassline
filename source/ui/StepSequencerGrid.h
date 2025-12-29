#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include "../generator/EuclideanRhythm.h"

class StepSequencerGrid : public juce::Component,
                           private juce::Timer
{
public:
    StepSequencerGrid()
    {
        startTimerHz(30); // 30 FPS for smooth animation
    }

    void paint(juce::Graphics& g) override
    {
        auto bounds = getLocalBounds().reduced(2);

        // Background
        g.setColour(juce::Colours::black);
        g.fillRect(bounds);

        if (numSteps <= 0)
            return;

        int stepWidth = bounds.getWidth() / numSteps;
        int stepHeight = bounds.getHeight();

        // Draw each step
        for (int i = 0; i < numSteps; ++i)
        {
            auto stepBounds = juce::Rectangle<int>(
                bounds.getX() + i * stepWidth,
                bounds.getY(),
                stepWidth - 2,
                stepHeight
            );

            // Determine if this step has a hit
            bool hasHit = euclidean.shouldTrigger(i, numSteps, numHits, rotation);

            // Color based on state
            if (i == currentStep && isPlaying)
            {
                // Current playing step
                g.setColour(hasHit ? juce::Colours::lime : juce::Colours::yellow);
                g.fillRect(stepBounds);
            }
            else if (hasHit)
            {
                // Step with a hit
                g.setColour(juce::Colours::lightblue);
                g.fillRect(stepBounds);
            }
            else
            {
                // Empty step
                g.setColour(juce::Colours::darkgrey);
                g.drawRect(stepBounds, 1);
            }

            // Draw step number
            g.setColour(juce::Colours::white.withAlpha(0.5f));
            g.setFont(10.0f);
            g.drawText(juce::String(i + 1), stepBounds, juce::Justification::centred);
        }

        // Border
        g.setColour(juce::Colours::grey);
        g.drawRect(bounds, 2);
    }

    void setPattern(int steps, int hits, int rot)
    {
        if (numSteps != steps || numHits != hits || rotation != rot)
        {
            numSteps = steps;
            numHits = hits;
            rotation = rot;
            repaint();
        }
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
    void timerCallback() override
    {
        // Trigger repaint for animation
        repaint();
    }

    EuclideanRhythm euclidean;
    int numSteps = 8;
    int numHits = 3;
    int rotation = 0;
    int currentStep = 0;
    bool isPlaying = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(StepSequencerGrid)
};
