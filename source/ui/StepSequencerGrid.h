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

        // Black background
        g.setColour(juce::Colours::black);
        g.fillRoundedRectangle(bounds.toFloat(), 8.0f);

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
                stepWidth - 3,
                stepHeight
            ).reduced(4);

            // Determine if this step has a hit
            bool hasHit = euclidean.shouldTrigger(i, numSteps, numHits, rotation);

            // Color based on state
            if (i == currentStep && isPlaying)
            {
                // Black comic outline
                g.setColour(juce::Colours::black);
                g.fillRoundedRectangle(stepBounds.toFloat().expanded(2), 4.0f);

                // Current playing step - bright yellow or red
                g.setColour(hasHit ? juce::Colour(0xffffdd00) : juce::Colour(0xffff0000));
                g.fillRoundedRectangle(stepBounds.toFloat(), 4.0f);

                // White highlight
                g.setColour(juce::Colours::white.withAlpha(0.4f));
                g.fillRoundedRectangle(stepBounds.toFloat().removeFromTop(stepBounds.getHeight() * 0.3f), 4.0f);
            }
            else if (hasHit)
            {
                // Step with a hit - red
                g.setColour(juce::Colours::black);
                g.fillRoundedRectangle(stepBounds.toFloat().expanded(2), 4.0f);

                g.setColour(juce::Colour(0xffcc0000));
                g.fillRoundedRectangle(stepBounds.toFloat(), 4.0f);

                g.setColour(juce::Colours::white.withAlpha(0.3f));
                g.fillRoundedRectangle(stepBounds.toFloat().removeFromTop(stepBounds.getHeight() * 0.3f), 4.0f);
            }
            else
            {
                // Empty step
                g.setColour(juce::Colour(0xff330000));
                g.fillRoundedRectangle(stepBounds.toFloat(), 4.0f);

                g.setColour(juce::Colour(0xff660000));
                g.drawRoundedRectangle(stepBounds.toFloat(), 4.0f, 1.5f);
            }

            // Draw step number with bold font
            g.setColour(juce::Colours::white.withAlpha(hasHit ? 0.9f : 0.5f));
            g.setFont(juce::Font(10.0f, juce::Font::bold));
            g.drawText(juce::String(i + 1), stepBounds, juce::Justification::centred);
        }

        // Bold comic border
        g.setColour(juce::Colours::black);
        g.drawRoundedRectangle(bounds.toFloat(), 8.0f, 3.0f);

        g.setColour(juce::Colour(0xffffdd00));
        g.drawRoundedRectangle(bounds.toFloat().reduced(2), 8.0f, 2.0f);
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
