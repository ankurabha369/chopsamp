#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "CustomLookAndFeel.h"

struct PixelTabButton : public juce::Button {
    int tabIndex = 0;
    std::function<bool()> isActiveCallback;
    
    PixelTabButton() : juce::Button("pixel_tab") {}

    void paintButton(juce::Graphics& g, bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override {
        juce::ignoreUnused(shouldDrawButtonAsDown);
        
        g.setFont(CustomLookAndFeel::getMinecraftFont(15.0f));

        bool isActive = isActiveCallback ? isActiveCallback() : false;

        // Inactive: Pure white (#FFFFFF), Active: Coral / Salmon Pink-Red (#F07178)
        juce::Colour textCol = isActive ? juce::Colour::fromRGB(240, 115, 125) : juce::Colours::white;
        if (shouldDrawButtonAsHighlighted && !isActive)
            textCol = textCol.withAlpha(0.8f);

        g.setColour(textCol);
        g.drawText(juce::String(tabIndex + 1), getLocalBounds(), juce::Justification::centred, false);
    }
};

struct TactileReverseButton : public juce::Button {
    juce::Image imgOff;
    juce::Image imgOn;

    TactileReverseButton() : juce::Button("reverse") {
        setClickingTogglesState(true);
        imgOff = juce::ImageCache::getFromMemory(BinaryData::reverse_off_png, BinaryData::reverse_off_pngSize);
        imgOn  = juce::ImageCache::getFromMemory(BinaryData::reverse_on_png,  BinaryData::reverse_on_pngSize);
    }

    void paintButton(juce::Graphics& g, bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override {
        juce::ignoreUnused(shouldDrawButtonAsHighlighted, shouldDrawButtonAsDown);
        const auto& img = getToggleState() ? imgOn : imgOff;
        if (img.isValid()) {
            g.drawImageWithin(img, 0, 0, getWidth(), getHeight(),
                              juce::RectanglePlacement::centred | juce::RectanglePlacement::onlyReduceInSize);
        }
    }
};

struct TactileApplyToAllButton : public juce::Button {
    TactileApplyToAllButton() : juce::Button("applytoall") {}

    void paintButton(juce::Graphics& g, bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override {
        juce::ignoreUnused(shouldDrawButtonAsHighlighted);
        auto bounds = getLocalBounds().toFloat();
        
        // Tactile dark button surface
        g.setColour(shouldDrawButtonAsDown ? juce::Colour::fromRGB(45, 48, 56) : (isMouseOver() ? juce::Colour::fromRGB(36, 38, 44) : juce::Colour::fromRGB(22, 24, 28)));
        g.fillRoundedRectangle(bounds, 3.0f);
        
        // Clean high-contrast border
        g.setColour(shouldDrawButtonAsDown ? juce::Colour::fromRGB(240, 115, 125) : (isMouseOver() ? juce::Colour::fromRGB(80, 84, 94) : juce::Colour::fromRGB(55, 58, 66)));
        g.drawRoundedRectangle(bounds.reduced(0.5f), 3.0f, 1.0f);

        // Bold centered text
        g.setColour(juce::Colours::white);
        g.setFont(CustomLookAndFeel::getRobotoFont(11.5f, true));
        g.drawText("APPLY TO ALL", bounds, juce::Justification::centred);
    }
};

struct ColorDropdownButton : public juce::Button {
    std::function<juce::Colour()> getCurrentColour;
    std::function<void(juce::Colour)> onColourSelected;

    ColorDropdownButton() : juce::Button("color_dropdown") {}

    void paintButton(juce::Graphics& g, bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override {
        juce::ignoreUnused(shouldDrawButtonAsHighlighted, shouldDrawButtonAsDown);
        
        // Pure flat black box with NO border and NO corner radius
        g.setColour(juce::Colour::fromRGB(0, 0, 0));
        g.fillRect(getLocalBounds());

        // Current color swatch on left
        float arrowAreaW = 12.0f;
        auto colorArea = juce::Rectangle<float>(4.0f, 4.0f, (float)getWidth() - arrowAreaW - 6.0f, (float)getHeight() - 8.0f);
        
        juce::Colour col = getCurrentColour ? getCurrentColour() : juce::Colour::fromRGB(255, 152, 0);
        g.setColour(col);
        g.fillRect(colorArea);

        // Small white dropdown arrow on right
        float arrowW = 6.0f;
        float arrowH = 4.0f;
        float arrowX = (float)getWidth() - 9.0f;
        float arrowY = (float)getHeight() * 0.5f - arrowH * 0.5f;
        juce::Path arrow;
        arrow.addTriangle(arrowX, arrowY, arrowX + arrowW, arrowY, arrowX + arrowW * 0.5f, arrowY + arrowH);
        g.setColour(juce::Colours::white);
        g.fillPath(arrow);
    }

    void clicked() override {
        juce::PopupMenu menu;
        const struct { const char* name; juce::Colour c; } palette[] = {
            { "Orange",  juce::Colour::fromRGB(255, 152, 0) },
            { "Coral",   juce::Colour::fromRGB(240, 115, 125) },
            { "Green",   juce::Colour::fromRGB(76, 175, 80) },
            { "Blue",    juce::Colour::fromRGB(33, 150, 243) },
            { "Yellow",  juce::Colour::fromRGB(255, 235, 59) },
            { "Cyan",    juce::Colour::fromRGB(0, 188, 212) },
            { "Magenta", juce::Colour::fromRGB(233, 30, 99) },
            { "Purple",  juce::Colour::fromRGB(156, 39, 176) }
        };
        for (int i = 0; i < 8; ++i) {
            menu.addItem(i + 1, palette[i].name);
        }
        menu.showMenuAsync(juce::PopupMenu::Options().withTargetComponent(this),
            [this, palette](int result) {
                if (result >= 1 && result <= 8 && onColourSelected) {
                    onColourSelected(palette[result - 1].c);
                    repaint();
                }
            });
    }
};

class SliceControlsComponent : public juce::Component
{
public:
    SliceControlsComponent(ChopSampAudioProcessor& p);
    ~SliceControlsComponent() override;

    void paint(juce::Graphics&) override;
    void resized() override;
    
    void mouseDown(const juce::MouseEvent& e) override;
    void mouseDrag(const juce::MouseEvent& e) override;
    void mouseUp(const juce::MouseEvent& e) override;

    void setSelectedSliceIndex(int index);

    void setDropdowns(juce::ComboBox* pbDropdown, juce::ComboBox* rootDropdown);
    void setLayoutConfig(struct LayoutConfig* cfg) { layoutConfigPtr = cfg; }

private:
    ChopSampAudioProcessor& audioProcessor;
    struct LayoutConfig* layoutConfigPtr = nullptr;
    int selectedSliceIndex = -1;
    int draggingAdsrNode = -1;
    juce::Rectangle<int> adsrGraphArea;
    juce::Rectangle<int> tabGridArea;

    juce::Slider volSlider, panSlider, pitchSlider, attackSlider, decaySlider, sustainSlider, releaseSlider, crossfadeSlider, startTrimSlider, endTrimSlider, lpfSlider, hpfSlider;
    juce::Label volLabel, panLabel, pitchLabel, attackLabel, decayLabel, sustainLabel, releaseLabel, crossfadeLabel, startTrimLabel, endTrimLabel, lpfLabel, hpfLabel;
    juce::Label reverseLabel, colorLabel;
    TactileReverseButton reverseButton;
    TactileApplyToAllButton applyAdsrToAllBtn;
    ColorDropdownButton colorDropdownButton;
    PixelTabButton tabButtons[MAX_SAMPLE_TABS];

    juce::Label pbLabel;
    juce::ComboBox* pbRangeDropdownPtr = nullptr;
    juce::ComboBox* rootKeyDropdownPtr = nullptr;

    struct AdsrPoints {
        juce::Point<float> pStart;
        juce::Point<float> pAttack;
        juce::Point<float> pDecay;
        juce::Point<float> pSustain;
        juce::Point<float> pRelease;
    };
    AdsrPoints getAdsrPoints() const;

    void setupSlider(juce::Slider& s, juce::Label& lbl, const juce::String& name);
    void updateParametersFromSlider(juce::Slider& s);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SliceControlsComponent)
};
