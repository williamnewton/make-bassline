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
        auto bounds = getLocalBounds().toFloat();

        // Simple black outline
        g.setColour(juce::Colours::black);
        g.fillRoundedRectangle(bounds, 4.0f);

        // Dark background
        g.setColour(juce::Colour(0xff1a1a1a));
        g.fillRoundedRectangle(bounds.reduced(2), 4.0f);

        // Yellow border
        g.setColour(juce::Colour(0xffffdd00));
        g.drawRoundedRectangle(bounds.reduced(2), 4.0f, 1.5f);

        // Text - simple and clean
        g.setFont(juce::Font(12.0f, juce::Font::bold));
        g.setColour(juce::Colour(0xffffdd00));
        g.drawText("Export MIDI", bounds, juce::Justification::centred);
    }

    void mouseDrag(const juce::MouseEvent& event) override
    {
        if (event.getDistanceFromDragStart() < 5 || isDragging)
            return;

        // Create the MIDI pattern
        if (onCreatePattern)
        {
            auto midiData = onCreatePattern();

            if (midiData.getSize() > 0)
            {
                // Create a temporary MIDI file
                auto tempFile = juce::File::getSpecialLocation(
                    juce::File::tempDirectory).getChildFile("MakeBasslinePattern.mid");

                if (tempFile.replaceWithData(midiData.getData(), midiData.getSize()))
                {
                    isDragging = true;

                    // Use performExternalDragDropOfFiles for proper external drag
                    juce::StringArray files;
                    files.add(tempFile.getFullPathName());

                    auto* container = juce::DragAndDropContainer::findParentDragContainerFor(this);
                    if (container != nullptr)
                    {
                        container->performExternalDragDropOfFiles(files, true, this, [this]()
                        {
                            isDragging = false;
                        });
                    }
                    else
                    {
                        isDragging = false;
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
    bool isDragging = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MidiDragComponent)
};
