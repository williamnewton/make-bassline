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

        // Comic book style - black outline
        g.setColour(juce::Colours::black);
        g.fillRoundedRectangle(bounds, 8.0f);

        // Dark red background
        g.setColour(juce::Colour(0xff1a0000));
        g.fillRoundedRectangle(bounds.reduced(3), 8.0f);

        // Yellow border
        g.setColour(juce::Colour(0xffffdd00));
        g.drawRoundedRectangle(bounds.reduced(3), 8.0f, 2.5f);

        // Icon area with musical note
        auto iconArea = bounds.reduced(10);
        iconArea.removeFromRight(bounds.getWidth() * 0.6f);
        g.setColour(juce::Colour(0xffffdd00));
        g.setFont(juce::Font(32.0f, juce::Font::bold));
        g.drawText("\u266B", iconArea.toNearestInt(), juce::Justification::centred);

        // Text area
        auto textArea = bounds.reduced(10);
        textArea.removeFromLeft(bounds.getWidth() * 0.25f);

        g.setFont(juce::Font(13.0f, juce::Font::bold));
        g.setColour(juce::Colour(0xffffdd00));
        g.drawText("DRAG MIDI", textArea.removeFromTop(18).toNearestInt(), juce::Justification::centredLeft);

        g.setFont(juce::Font(11.0f, juce::Font::plain));
        g.setColour(juce::Colour(0xffff8800));
        juce::String info = juce::String(numBars) + " bar" + (numBars > 1 ? "s" : "");
        g.drawText(info, textArea.toNearestInt(), juce::Justification::centredLeft);
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
