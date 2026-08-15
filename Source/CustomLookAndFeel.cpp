#include "CustomLookAndFeel.h"

CustomLookAndFeel::CustomLookAndFeel()
{
    // Global colors
    setColour(juce::Slider::thumbColourId, juce::Colour::fromRGB(40, 40, 40));
    setColour(juce::Slider::rotarySliderFillColourId, juce::Colours::transparentBlack);
    setColour(juce::Slider::rotarySliderOutlineColourId, juce::Colours::transparentBlack);
    setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    setColour(juce::Slider::textBoxBackgroundColourId, juce::Colours::transparentBlack);
    setColour(juce::Slider::textBoxTextColourId, juce::Colours::white);
    
    setColour(juce::TextButton::buttonColourId, juce::Colour::fromRGB(45, 45, 45));
    setColour(juce::TextButton::buttonOnColourId, juce::Colour::fromRGB(65, 65, 65));
    setColour(juce::TextButton::textColourOffId, juce::Colours::white);
    setColour(juce::TextButton::textColourOnId, juce::Colour::fromRGB(255, 180, 50));
    
    setColour(juce::ComboBox::backgroundColourId, juce::Colour::fromRGB(10, 10, 10));
    setColour(juce::ComboBox::textColourId, juce::Colours::white);
    setColour(juce::ComboBox::outlineColourId, juce::Colour::fromRGB(45, 45, 45));
    setColour(juce::ComboBox::arrowColourId, juce::Colours::white);
    
    setColour(juce::PopupMenu::backgroundColourId, juce::Colour::fromRGB(10, 10, 10));
    setColour(juce::PopupMenu::textColourId, juce::Colours::white);
    setColour(juce::PopupMenu::highlightedBackgroundColourId, juce::Colour::fromRGB(45, 45, 45));
    setColour(juce::PopupMenu::highlightedTextColourId, juce::Colours::white);

    knobBaseImg = juce::ImageCache::getFromMemory(BinaryData::knob_base_png, BinaryData::knob_base_pngSize);
    knobRotImg  = juce::ImageCache::getFromMemory(BinaryData::knob_rotating_part_png, BinaryData::knob_rotating_part_pngSize);
}

juce::Font CustomLookAndFeel::getMinecraftFont(float height)
{
    static auto tf = juce::Typeface::createSystemTypefaceFor(BinaryData::Minecraft_ttf, BinaryData::Minecraft_ttfSize);
    return juce::Font(juce::FontOptions(tf)).withHeight(height);
}

juce::Font CustomLookAndFeel::getRobotoFont(float height, bool bold)
{
    static auto tfMed = juce::Typeface::createSystemTypefaceFor(BinaryData::RobotoMedium_ttf, BinaryData::RobotoMedium_ttfSize);
    static auto tfBold = juce::Typeface::createSystemTypefaceFor(BinaryData::RobotoBold_ttf, BinaryData::RobotoBold_ttfSize);
    return juce::Font(juce::FontOptions(bold ? tfBold : tfMed)).withHeight(height);
}

void CustomLookAndFeel::drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
                                         float sliderPos, const float rotaryStartAngle,
                                         const float rotaryEndAngle, juce::Slider& slider)
{
    juce::ignoreUnused(slider);

    auto bounds = juce::Rectangle<int>(x, y, width, height).toFloat();
    auto diameter = juce::jmin(bounds.getWidth(), bounds.getHeight());
    
    // 1. Base diameter is 100% (500px full size)
    auto baseBounds = juce::Rectangle<float>(bounds.getCentreX() - diameter * 0.5f,
                                             bounds.getCentreY() - diameter * 0.5f,
                                             diameter, diameter);

    // 2. Rotating top knob is 400px (400.0f / 500.0f = 80% ratio of base), centered
    float rotDiameter = diameter * (400.0f / 500.0f);
    auto rotBounds = juce::Rectangle<float>(bounds.getCentreX() - rotDiameter * 0.5f,
                                            bounds.getCentreY() - rotDiameter * 0.5f,
                                            rotDiameter, rotDiameter);

    auto angle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);

    if (knobBaseImg.isValid() && knobRotImg.isValid())
    {
        // 1. Draw static base PNG behind
        g.drawImage(knobBaseImg, baseBounds, juce::RectanglePlacement::centred | juce::RectanglePlacement::onlyReduceInSize);

        // 2. Draw rotating knob PNG on top, rotated around its center
        juce::Graphics::ScopedSaveState state(g);
        
        float cx = bounds.getCentreX();
        float cy = bounds.getCentreY();
        g.addTransform(juce::AffineTransform::rotation(angle, cx, cy));

        g.drawImage(knobRotImg, rotBounds, juce::RectanglePlacement::centred | juce::RectanglePlacement::onlyReduceInSize);
    }
    else
    {
        auto radius = diameter * 0.5f - 4.0f;
        auto centreX = bounds.getCentreX();
        auto centreY = bounds.getCentreY();
        auto rx = centreX - radius;
        auto ry = centreY - radius;
        auto rw = radius * 2.0f;

        // Draw dark, flat knob body
        g.setColour(juce::Colour::fromRGB(40, 40, 40));
        g.fillEllipse(rx, ry, rw, rw);

        // Draw minimalist indicator line (white)
        juce::Path p;
        auto pointerLength = radius * 0.8f;
        auto pointerThickness = 2.0f;
        p.addRectangle(-pointerThickness * 0.5f, -radius, pointerThickness, pointerLength);
        p.applyTransform(juce::AffineTransform::rotation(angle).translated(centreX, centreY));
        
        g.setColour(juce::Colours::white);
        g.fillPath(p);
    }
}

void CustomLookAndFeel::drawLinearSlider(juce::Graphics& g, int x, int y, int width, int height,
                                         float sliderPos, float minSliderPos, float maxSliderPos,
                                         const juce::Slider::SliderStyle style, juce::Slider& slider)
{
    juce::ignoreUnused(minSliderPos, maxSliderPos, style, slider);

    auto trackBounds = juce::Rectangle<float>((float)x, (float)y + (float)height * 0.5f - 3.0f, (float)width, 6.0f);
    
    // Inset solid black track with crisp border
    g.setColour(juce::Colour::fromRGB(0, 0, 0));
    g.fillRect(trackBounds);
    g.setColour(juce::Colour::fromRGB(70, 70, 70));
    g.drawRect(trackBounds, 1.0f);

    // Active fill
    if (sliderPos > (float)x)
    {
        auto activeTrack = trackBounds.withWidth(juce::jmax(0.0f, sliderPos - (float)x));
        g.setColour(juce::Colour::fromRGB(240, 115, 125)); // Coral active fill
        g.fillRect(activeTrack);
    }

    // High-visibility crisp circular thumb
    float thumbDiameter = 14.0f;
    float thumbX = sliderPos - thumbDiameter * 0.5f;
    float thumbY = (float)y + (float)height * 0.5f - thumbDiameter * 0.5f;

    // Thumb outer circle
    g.setColour(juce::Colour::fromRGB(25, 25, 25));
    g.fillEllipse(thumbX, thumbY, thumbDiameter, thumbDiameter);
    g.setColour(juce::Colours::white);
    g.drawEllipse(thumbX, thumbY, thumbDiameter, thumbDiameter, 1.5f);

    // Inner bright center dot
    g.setColour(juce::Colours::white);
    g.fillEllipse(sliderPos - 2.5f, (float)y + (float)height * 0.5f - 2.5f, 5.0f, 5.0f);
}

void CustomLookAndFeel::drawButtonBackground(juce::Graphics& g, juce::Button& button, const juce::Colour& backgroundColour,
                                             bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown)
{
    juce::ignoreUnused(backgroundColour);
    auto bounds = button.getLocalBounds().toFloat().reduced(1.0f);
    
    juce::Colour baseColor = shouldDrawButtonAsDown ? juce::Colour::fromRGB(25, 25, 25) : juce::Colour::fromRGB(10, 10, 10);
    if (shouldDrawButtonAsHighlighted) baseColor = baseColor.brighter(0.1f);
    
    g.setColour(baseColor);
    g.fillRoundedRectangle(bounds, 3.0f);
    
    g.setColour(juce::Colour::fromRGB(45, 45, 45));
    g.drawRoundedRectangle(bounds, 3.0f, 1.0f);
}

void CustomLookAndFeel::drawButtonText(juce::Graphics& g, juce::TextButton& button,
                                       bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown)
{
    juce::ignoreUnused(shouldDrawButtonAsHighlighted, shouldDrawButtonAsDown);
    g.setFont(getMinecraftFont(13.0f));
    g.setColour(juce::Colours::white);
    g.drawText(button.getButtonText(), button.getLocalBounds(), juce::Justification::centred, true);
}

void CustomLookAndFeel::drawToggleButton (juce::Graphics& g, juce::ToggleButton& button,
                                          bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown)
{
    juce::ignoreUnused(shouldDrawButtonAsHighlighted, shouldDrawButtonAsDown);
    auto bounds = button.getLocalBounds().toFloat();
    float tickSize = 12.0f;
    auto tickBox = juce::Rectangle<float>(bounds.getX() + 1.0f, bounds.getCentreY() - tickSize * 0.5f, tickSize, tickSize);

    // Box background
    g.setColour(juce::Colour::fromRGB(12, 14, 18));
    g.fillRoundedRectangle(tickBox, 2.0f);

    // Border
    g.setColour(button.getToggleState() ? juce::Colour::fromRGB(240, 115, 125) : juce::Colour::fromRGB(65, 68, 76));
    g.drawRoundedRectangle(tickBox.reduced(0.5f), 2.0f, 1.0f);

    // Checked tick
    if (button.getToggleState())
    {
        g.setColour(juce::Colour::fromRGB(240, 115, 125));
        g.fillRoundedRectangle(tickBox.reduced(2.5f), 1.0f);
    }

    // Text
    g.setColour(button.getToggleState() ? juce::Colours::white : juce::Colour::fromRGB(160, 162, 170));
    g.setFont(getRobotoFont(11.0f, true));
    g.drawText(button.getButtonText(), bounds.withTrimmedLeft(tickSize + 5.0f).toNearestInt(), juce::Justification::centredLeft, true);
}

void CustomLookAndFeel::drawComboBox(juce::Graphics& g, int width, int height, bool isButtonDown,
                                     int buttonX, int buttonY, int buttonW, int buttonH,
                                     juce::ComboBox& box)
{
    juce::ignoreUnused(isButtonDown, buttonX, buttonY, buttonW, buttonH, box);
    
    // Pure flat black box with NO border and NO corner radius
    g.setColour(juce::Colour::fromRGB(0, 0, 0));
    g.fillRect(0, 0, width, height);
    
    // Draw crisp white dropdown arrow on the right
    float arrowW = 7.0f;
    float arrowH = 4.5f;
    float arrowX = (float)width - 13.0f;
    float arrowY = (float)height * 0.5f - arrowH * 0.5f;

    juce::Path arrow;
    arrow.addTriangle(arrowX, arrowY, arrowX + arrowW, arrowY, arrowX + arrowW * 0.5f, arrowY + arrowH);
    
    g.setColour(juce::Colours::white);
    g.fillPath(arrow);
}

juce::Font CustomLookAndFeel::getComboBoxFont(juce::ComboBox& box)
{
    juce::ignoreUnused(box);
    return getMinecraftFont(13.0f);
}

void CustomLookAndFeel::positionComboBoxText(juce::ComboBox& box, juce::Label& label)
{
    label.setFont(getMinecraftFont(13.0f));
    label.setColour(juce::Label::textColourId, juce::Colours::white);
    label.setBounds(6, 0, box.getWidth() - 18, box.getHeight());
    label.setJustificationType(juce::Justification::centredLeft);
}

void CustomLookAndFeel::drawPopupMenuBackground(juce::Graphics& g, int width, int height)
{
    g.setColour(juce::Colour::fromRGB(0, 0, 0));
    g.fillRect(0, 0, width, height);
}

void CustomLookAndFeel::drawPopupMenuItem(juce::Graphics& g, const juce::Rectangle<int>& area,
                                          bool isSeparator, bool isActive, bool isHighlighted, bool isTicked,
                                          bool hasSubMenu, const juce::String& text, const juce::String& shortcutKeyText,
                                          const juce::Drawable* icon, const juce::Colour* textColour)
{
    juce::ignoreUnused(isSeparator, isActive, isTicked, hasSubMenu, shortcutKeyText, icon, textColour);
    if (isHighlighted)
    {
        g.setColour(juce::Colour::fromRGB(35, 35, 35));
        g.fillRect(area);
    }
    
    g.setFont(getMinecraftFont(13.0f));
    g.setColour(juce::Colours::white);
    g.drawText(text, area.reduced(8, 0), juce::Justification::centredLeft, true);
}

juce::Font CustomLookAndFeel::getPopupMenuFont()
{
    return getMinecraftFont(13.0f);
}

juce::Font CustomLookAndFeel::getLabelFont(juce::Label& label)
{
    if (dynamic_cast<juce::ComboBox*>(label.getParentComponent()) != nullptr)
        return getMinecraftFont(13.0f);
    return label.getFont();
}

void CustomLookAndFeel::drawLabel(juce::Graphics& g, juce::Label& label)
{
    g.setColour(label.findColour(juce::Label::textColourId));
    if (dynamic_cast<juce::ComboBox*>(label.getParentComponent()) != nullptr)
    {
        g.setFont(getMinecraftFont(13.0f));
    }
    else
    {
        g.setFont(label.getFont());
    }
    g.drawFittedText(label.getText(), label.getLocalBounds(), label.getJustificationType(),
                     1, label.getMinimumHorizontalScale());
}

juce::Rectangle<int> CustomLookAndFeel::getTooltipBounds(const juce::String& tipText, juce::Point<int> screenPos, juce::Rectangle<int> parentArea)
{
    auto font = getRobotoFont(11.5f, false);
    int maxW = 280;
    
    juce::AttributedString att;
    att.append(tipText, font, juce::Colours::white);
    att.setWordWrap(juce::AttributedString::WordWrap::byWord);
    
    juce::TextLayout layout;
    layout.createLayout(att, (float)maxW);
    
    int w = juce::jlimit(60, maxW + 16, (int)std::ceil(layout.getWidth()) + 18);
    int h = juce::jlimit(24, 120, (int)std::ceil(layout.getHeight()) + 12);
    
    int x = screenPos.x + 12;
    int y = screenPos.y + 12;
    
    if (x + w > parentArea.getRight())  x = screenPos.x - w - 8;
    if (y + h > parentArea.getBottom()) y = screenPos.y - h - 8;
    
    return { x, y, w, h };
}

void CustomLookAndFeel::drawTooltip(juce::Graphics& g, const juce::String& text, int width, int height)
{
    auto bounds = juce::Rectangle<int>(0, 0, width, height).toFloat();
    
    // Sleek dark glass tooltip box with coral border
    g.setColour(juce::Colour::fromRGB(14, 16, 22).withAlpha(0.96f));
    g.fillRoundedRectangle(bounds, 4.0f);

    g.setColour(juce::Colour::fromRGB(240, 115, 125).withAlpha(0.85f));
    g.drawRoundedRectangle(bounds.reduced(0.5f), 4.0f, 1.0f);

    g.setColour(juce::Colours::white);
    g.setFont(getRobotoFont(11.5f, false));
    g.drawFittedText(text, bounds.reduced(8.0f, 4.0f).toNearestInt(), juce::Justification::centredLeft, 3);
}
