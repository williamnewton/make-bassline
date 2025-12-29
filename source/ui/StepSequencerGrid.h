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
        setMouseCursor(juce::MouseCursor::PointingHandCursor);
    }

    // Callback when a step is clicked
    std::function<void(int step)> onStepClicked;

    // Callback to check if step is manually toggled
    std::function<bool(int step)> isStepManuallyToggled;

    void paint(juce::Graphics& g) override
    {
        auto bounds = getLocalBounds().reduced(2);

        // White background for pop-art look
        g.setColour(juce::Colours::white);
        g.fillRoundedRectangle(bounds.toFloat(), 8.0f);

        if (numSteps <= 0)
            return;

        int stepWidth = bounds.getWidth() / numSteps;
        int stepHeight = bounds.getHeight();

        // Draw each step - clean, no text
        for (int i = 0; i < numSteps; ++i)
        {
            auto stepBounds = juce::Rectangle<int>(
                bounds.getX() + i * stepWidth,
                bounds.getY(),
                stepWidth - 3,
                stepHeight
            ).reduced(6);

            // Determine if this step has a hit
            bool hasHit = euclidean.shouldTrigger(i, numSteps, numHits, rotation);
            bool isHovered = (i == hoveredStep);
            bool isManuallyToggled = isStepManuallyToggled ? isStepManuallyToggled(i) : false;

            // Color based on state - bold pop-art colors
            if (i == currentStep && isPlaying)
            {
                // Black comic outline
                g.setColour(juce::Colours::black);
                g.fillRoundedRectangle(stepBounds.toFloat().expanded(3), 5.0f);

                // Current playing step - bright yellow or red
                g.setColour(hasHit ? juce::Colour(0xffffdd00) : juce::Colour(0xffff3333));
                g.fillRoundedRectangle(stepBounds.toFloat(), 5.0f);

                // Shine effect
                g.setColour(juce::Colours::white.withAlpha(0.5f));
                g.fillRoundedRectangle(stepBounds.toFloat().removeFromTop(stepBounds.getHeight() * 0.35f), 5.0f);
            }
            else if (hasHit)
            {
                // Step with a hit - bold red with black outline
                g.setColour(juce::Colours::black);
                g.fillRoundedRectangle(stepBounds.toFloat().expanded(isHovered ? 3 : 2), 5.0f);

                // Brighter red on hover
                g.setColour(isHovered ? juce::Colour(0xffff0000) : juce::Colour(0xffdd0000));
                g.fillRoundedRectangle(stepBounds.toFloat(), 5.0f);

                // Subtle shine
                g.setColour(juce::Colours::white.withAlpha(isHovered ? 0.3f : 0.2f));
                g.fillRoundedRectangle(stepBounds.toFloat().removeFromTop(stepBounds.getHeight() * 0.3f), 5.0f);

                // Indicator for manually toggled steps
                if (isManuallyToggled)
                {
                    g.setColour(juce::Colour(0xffffdd00));
                    g.fillEllipse(stepBounds.toFloat().getCentreX() - 3, stepBounds.toFloat().getY() + 4, 6, 6);
                }
            }
            else
            {
                // Empty step - light gray with black outline
                g.setColour(juce::Colours::black);
                g.fillRoundedRectangle(stepBounds.toFloat().expanded(isHovered ? 2.5f : 1.5f), 5.0f);

                // Slightly darker on hover
                g.setColour(isHovered ? juce::Colour(0xffc0c0c0) : juce::Colour(0xffe0e0e0));
                g.fillRoundedRectangle(stepBounds.toFloat(), 5.0f);

                // Indicator for manually toggled steps
                if (isManuallyToggled)
                {
                    g.setColour(juce::Colour(0xffffdd00));
                    g.fillEllipse(stepBounds.toFloat().getCentreX() - 3, stepBounds.toFloat().getY() + 4, 6, 6);
                }
            }
        }

        // Bold black border - comic book style
        g.setColour(juce::Colours::black);
        g.drawRoundedRectangle(bounds.toFloat(), 8.0f, 3.0f);
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

    void mouseMove(const juce::MouseEvent& event) override
    {
        if (numSteps <= 0)
        {
            if (hoveredStep != -1)
            {
                hoveredStep = -1;
                repaint();
            }
            return;
        }

        auto bounds = getLocalBounds().reduced(2);
        int stepWidth = bounds.getWidth() / numSteps;

        // Calculate which step is being hovered
        int newHoveredStep = (event.x - bounds.getX()) / stepWidth;

        if (newHoveredStep >= 0 && newHoveredStep < numSteps)
        {
            if (hoveredStep != newHoveredStep)
            {
                hoveredStep = newHoveredStep;
                repaint();
            }
        }
        else if (hoveredStep != -1)
        {
            hoveredStep = -1;
            repaint();
        }
    }

    void mouseExit(const juce::MouseEvent&) override
    {
        if (hoveredStep != -1)
        {
            hoveredStep = -1;
            repaint();
        }
    }

    void mouseDown(const juce::MouseEvent& event) override
    {
        if (numSteps <= 0)
            return;

        auto bounds = getLocalBounds().reduced(2);
        int stepWidth = bounds.getWidth() / numSteps;

        // Calculate which step was clicked
        int clickedStep = (event.x - bounds.getX()) / stepWidth;

        if (clickedStep >= 0 && clickedStep < numSteps && onStepClicked)
        {
            onStepClicked(clickedStep);
            repaint(); // Force immediate UI update
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
    int hoveredStep = -1;  // Track which step is hovered (-1 = none)

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(StepSequencerGrid)
};
