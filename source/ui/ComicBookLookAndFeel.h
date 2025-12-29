#pragma once
#include <juce_gui_basics/juce_gui_basics.h>

class ComicBookLookAndFeel : public juce::LookAndFeel_V4
{
public:
    ComicBookLookAndFeel()
    {
        // Yellow/Red/Black color palette
        setColour(juce::Slider::thumbColourId, juce::Colour(0xffffdd00));        // Bright Yellow
        setColour(juce::Slider::trackColourId, juce::Colour(0xffff0000));        // Red
        setColour(juce::Slider::backgroundColourId, juce::Colour(0xff1a0000));   // Deep red-black
        setColour(juce::Slider::rotarySliderFillColourId, juce::Colour(0xffffdd00)); // Yellow
        setColour(juce::Slider::rotarySliderOutlineColourId, juce::Colour(0xff000000)); // Black

        setColour(juce::Label::textColourId, juce::Colour(0xffffdd00));          // Yellow text
        setColour(juce::ComboBox::backgroundColourId, juce::Colour(0xff330000)); // Dark red
        setColour(juce::ComboBox::outlineColourId, juce::Colour(0xffffdd00));    // Yellow outline
        setColour(juce::ComboBox::textColourId, juce::Colour(0xffffdd00));       // Yellow text

        setColour(juce::TextButton::buttonColourId, juce::Colour(0xffff0000));   // Red button
        setColour(juce::TextButton::textColourOffId, juce::Colour(0xffffdd00));  // Yellow text
    }

    void drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
                          float sliderPosProportional, float rotaryStartAngle,
                          float rotaryEndAngle, juce::Slider& slider) override
    {
        auto bounds = juce::Rectangle<int>(x, y, width, height).toFloat().reduced(10);
        auto radius = juce::jmin(bounds.getWidth(), bounds.getHeight()) / 2.0f;
        auto toAngle = rotaryStartAngle + sliderPosProportional * (rotaryEndAngle - rotaryStartAngle);
        auto lineW = juce::jmin(8.0f, radius * 0.5f);
        auto arcRadius = radius - lineW * 0.5f;

        // Draw outer ring with comic-book style
        juce::Path backgroundArc;
        backgroundArc.addCentredArc(bounds.getCentreX(), bounds.getCentreY(),
                                    arcRadius, arcRadius, 0.0f,
                                    rotaryStartAngle, rotaryEndAngle, true);

        // Black outline (comic book style)
        g.setColour(juce::Colours::black);
        g.strokePath(backgroundArc, juce::PathStrokeType(lineW + 3.0f,
                     juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

        // Dark red background
        g.setColour(juce::Colour(0xff330000));
        g.strokePath(backgroundArc, juce::PathStrokeType(lineW,
                     juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

        // Value arc with red to yellow gradient
        if (toAngle > rotaryStartAngle)
        {
            juce::Path valueArc;
            valueArc.addCentredArc(bounds.getCentreX(), bounds.getCentreY(),
                                   arcRadius, arcRadius, 0.0f,
                                   rotaryStartAngle, toAngle, true);

            // Red to yellow gradient
            auto gradient = juce::ColourGradient(juce::Colour(0xffff0000), bounds.getCentreX(), bounds.getY(),
                                                 juce::Colour(0xffffdd00), bounds.getCentreX(), bounds.getBottom(),
                                                 false);
            g.setGradientFill(gradient);
            g.strokePath(valueArc, juce::PathStrokeType(lineW,
                         juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
        }

        // Center circle (thumb)
        auto thumbWidth = lineW * 1.5f;
        juce::Point<float> thumbPoint(bounds.getCentreX() + arcRadius * std::cos(toAngle - juce::MathConstants<float>::halfPi),
                                      bounds.getCentreY() + arcRadius * std::sin(toAngle - juce::MathConstants<float>::halfPi));

        // Black outline for thumb
        g.setColour(juce::Colours::black);
        g.fillEllipse(juce::Rectangle<float>(thumbWidth + 4, thumbWidth + 4).withCentre(thumbPoint));

        // Yellow thumb
        g.setColour(juce::Colour(0xffffdd00));
        g.fillEllipse(juce::Rectangle<float>(thumbWidth, thumbWidth).withCentre(thumbPoint));

        // White highlight
        g.setColour(juce::Colours::white.withAlpha(0.6f));
        g.fillEllipse(juce::Rectangle<float>(thumbWidth * 0.4f, thumbWidth * 0.4f)
                     .withCentre(thumbPoint.translated(-thumbWidth * 0.15f, -thumbWidth * 0.15f)));
    }

    void drawComboBox(juce::Graphics& g, int width, int height, bool,
                     int, int, int, int, juce::ComboBox& box) override
    {
        auto cornerSize = box.findColour(juce::ComboBox::outlineColourId).isTransparent() ? 0.0f : 3.0f;
        juce::Rectangle<int> boxBounds(0, 0, width, height);

        // Black comic outline
        g.setColour(juce::Colours::black);
        g.fillRoundedRectangle(boxBounds.toFloat(), cornerSize);

        // Fill
        g.setColour(box.findColour(juce::ComboBox::backgroundColourId));
        g.fillRoundedRectangle(boxBounds.reduced(2).toFloat(), cornerSize);

        // Yellow border
        g.setColour(box.findColour(juce::ComboBox::outlineColourId));
        g.drawRoundedRectangle(boxBounds.reduced(2).toFloat(), cornerSize, 2.0f);

        // Arrow
        juce::Rectangle<int> arrowZone(width - 30, 0, 20, height);
        juce::Path path;
        path.startNewSubPath(arrowZone.getX() + 3.0f, arrowZone.getCentreY() - 2.0f);
        path.lineTo(arrowZone.getCentreX(), arrowZone.getCentreY() + 3.0f);
        path.lineTo(arrowZone.getRight() - 3.0f, arrowZone.getCentreY() - 2.0f);

        g.setColour(juce::Colour(0xffffea00));
        g.strokePath(path, juce::PathStrokeType(2.0f));
    }

    void drawButtonBackground(juce::Graphics& g, juce::Button& button, const juce::Colour& backgroundColour,
                             bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override
    {
        auto bounds = button.getLocalBounds().toFloat().reduced(2.0f, 2.0f);
        auto baseColour = backgroundColour.withMultipliedSaturation(button.hasKeyboardFocus(true) ? 1.3f : 0.9f)
                                         .withMultipliedAlpha(button.isEnabled() ? 1.0f : 0.5f);

        if (shouldDrawButtonAsDown || shouldDrawButtonAsHighlighted)
            baseColour = baseColour.contrasting(shouldDrawButtonAsDown ? 0.2f : 0.05f);

        // Black outline
        g.setColour(juce::Colours::black);
        g.fillRoundedRectangle(bounds.expanded(2), 5.0f);

        // Fill
        g.setColour(baseColour);
        g.fillRoundedRectangle(bounds, 5.0f);

        // Highlight
        g.setColour(juce::Colours::white.withAlpha(0.2f));
        g.fillRoundedRectangle(bounds.removeFromTop(bounds.getHeight() * 0.4f), 5.0f);
    }

    juce::Font getTextButtonFont(juce::TextButton&, int buttonHeight) override
    {
        return juce::Font(buttonHeight * 0.6f, juce::Font::bold);
    }

    juce::Font getComboBoxFont(juce::ComboBox&) override
    {
        return juce::Font(14.0f, juce::Font::bold);
    }

    juce::Font getLabelFont(juce::Label&) override
    {
        return juce::Font(12.0f, juce::Font::bold);
    }

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ComicBookLookAndFeel)
};
