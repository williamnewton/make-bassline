#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include "../utils/MidiPatternExporter.h"

class MidiDragComponent : public juce::Component
{
public:
    MidiDragComponent()
    {
        setSize(200, 60);
    }

    void paint(juce::Graphics& g) override
    {
        auto bounds = getLocalBounds();

        // Background
        g.setColour(juce::Colours::darkslateblue);
        g.fillRoundedRectangle(bounds.toFloat(), 5.0f);

        // Border
        g.setColour(juce::Colours::cyan);
        g.drawRoundedRectangle(bounds.toFloat().reduced(2), 5.0f, 2.0f);

        // Icon area
        auto iconArea = bounds.removeFromLeft(50);
        g.setColour(juce::Colours::white);
        g.setFont(30.0f);
        g.drawText("\u266B", iconArea, juce::Justification::centred); // Musical note symbol

        // Text
        g.setFont(14.0f);
        g.setColour(juce::Colours::white);
        g.drawText("Drag MIDI", bounds, juce::Justification::centred);

        // Instruction text
        g.setFont(10.0f);
        g.setColour(juce::Colours::lightgrey);
        bounds.removeFromTop(20);
        juce::String info = juce::String(numBars) + " bar" + (numBars > 1 ? "s" : "");
        g.drawText(info, bounds, juce::Justification::centred);
    }

    void mouseDrag(const juce::MouseEvent& event) override
    {
        if (event.getDistanceFromDragStart() < 5)
            return;

        // Create the MIDI pattern
        if (onCreatePattern)
        {
            auto midiData = onCreatePattern();

            if (midiData.getSize() > 0)
            {
                // Create a temporary MIDI file
                auto tempFile = juce::File::getSpecialLocation(
                    juce::File::tempDirectory).getChildFile("BasslinePattern.mid");

                if (tempFile.replaceWithData(midiData.getData(), midiData.getSize()))
                {
                    // Start drag operation
                    auto* container = juce::DragAndDropContainer::findParentDragContainerFor(this);

                    if (container != nullptr)
                    {
                        juce::StringArray files;
                        files.add(tempFile.getFullPathName());

                        container->startDragging(files.joinIntoString("\n"), this);
                    }
                }
            }
        }
    }

    void setNumBars(int bars)
    {
        if (numBars != bars)
        {
            numBars = bars;
            repaint();
        }
    }

    std::function<juce::MemoryBlock()> onCreatePattern;

private:
    int numBars = 1;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MidiDragComponent)
};
