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

        // Pop-art style: Bold black outline
        g.setColour(juce::Colours::black);
        g.fillRoundedRectangle(bounds, 4.0f);

        // Bright yellow background (pop-art)
        g.setColour(juce::Colour(0xffffdd00));
        g.fillRoundedRectangle(bounds.reduced(3), 4.0f);

        // Black inner border for comic-book effect
        g.setColour(juce::Colours::black);
        g.drawRoundedRectangle(bounds.reduced(3), 4.0f, 2.0f);

        // Text centered - consistent with other buttons
        auto textArea = bounds.reduced(8);
        g.setFont(juce::Font(13.0f, juce::Font::bold));
        g.setColour(juce::Colours::black);
        g.drawText("Drag to export", textArea, juce::Justification::centred);
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
