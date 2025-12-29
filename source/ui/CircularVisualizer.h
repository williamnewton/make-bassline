#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include "../generator/EuclideanRhythm.h"

class CircularVisualizer : public juce::Component,
                            private juce::Timer
{
public:
    CircularVisualizer()
    {
        startTimerHz(30);
    }

    void paint(juce::Graphics& g) override
    {
        auto bounds = getLocalBounds().toFloat().reduced(4);
        auto centre = bounds.getCentre();
        float radius = juce::jmin(bounds.getWidth(), bounds.getHeight()) * 0.5f;
        float innerRadius = radius * 0.7f;

        // Background circle
        g.setColour(juce::Colours::black);
        g.fillEllipse(bounds);

        if (numSteps <= 0)
            return;

        // Draw step arcs
        float anglePerStep = juce::MathConstants<float>::twoPi / numSteps;

        for (int i = 0; i < numSteps; ++i)
        {
            float angle = -juce::MathConstants<float>::halfPi + (i * anglePerStep);
            bool hasHit = euclidean.shouldTrigger(i, numSteps, numHits, rotation);

            // Create arc path
            juce::Path arc;
            arc.addCentredArc(
                centre.x, centre.y,
                innerRadius, innerRadius,
                0.0f,
                angle - anglePerStep * 0.4f,
                angle + anglePerStep * 0.4f,
                true
            );

            juce::Path outerArc;
            outerArc.addCentredArc(
                centre.x, centre.y,
                radius, radius,
                0.0f,
                angle - anglePerStep * 0.4f,
                angle + anglePerStep * 0.4f,
                true
            );

            // Color based on state
            if (i == currentStep && isPlaying)
            {
                g.setColour(hasHit ? juce::Colours::lime : juce::Colours::yellow);
            }
            else if (hasHit)
            {
                g.setColour(juce::Colours::cyan);
            }
            else
            {
                g.setColour(juce::Colours::darkgrey.withAlpha(0.3f));
            }

            // Draw filled wedge
            juce::Path wedge;
            wedge.addPieSegment(
                centre.x - radius, centre.y - radius,
                radius * 2, radius * 2,
                angle - anglePerStep * 0.45f,
                angle + anglePerStep * 0.45f,
                innerRadius / radius
            );

            g.fillPath(wedge);

            // Draw step marker
            if (hasHit)
            {
                float markerAngle = angle;
                float markerRadius = (radius + innerRadius) * 0.5f;
                float markerX = centre.x + std::cos(markerAngle) * markerRadius;
                float markerY = centre.y + std::sin(markerAngle) * markerRadius;

                g.setColour(juce::Colours::white);
                g.fillEllipse(markerX - 4, markerY - 4, 8, 8);
            }
        }

        // Center circle with info
        float centerRadius = innerRadius * 0.5f;
        g.setColour(juce::Colours::darkgrey.darker());
        g.fillEllipse(
            centre.x - centerRadius,
            centre.y - centerRadius,
            centerRadius * 2,
            centerRadius * 2
        );

        // Display pattern info
        g.setColour(juce::Colours::white);
        g.setFont(14.0f);
        juce::String patternInfo = juce::String(numHits) + "/" + juce::String(numSteps);
        g.drawText(patternInfo,
                   centre.x - centerRadius, centre.y - centerRadius,
                   centerRadius * 2, centerRadius * 2,
                   juce::Justification::centred);

        // Outer border
        g.setColour(juce::Colours::grey);
        g.drawEllipse(bounds, 2.0f);
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
        repaint();
    }

    EuclideanRhythm euclidean;
    int numSteps = 8;
    int numHits = 3;
    int rotation = 0;
    int currentStep = 0;
    bool isPlaying = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CircularVisualizer)
};
