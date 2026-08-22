#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "WaveformComponent.h"
#include "SliceControlsComponent.h"
#include "CustomLookAndFeel.h"
#include "LayoutConfig.h"
#include "LayoutInspectorComponent.h"

struct TactileSliceButton : public juce::Button {
    juce::Image imgOff;
    juce::Image imgOn;

    TactileSliceButton() : juce::Button("slice_exec") {
        imgOff = juce::ImageCache::getFromMemory(BinaryData::slice_off_png, BinaryData::slice_off_pngSize);
        imgOn  = juce::ImageCache::getFromMemory(BinaryData::slice_on_png,  BinaryData::slice_on_pngSize);
    }

    void paintButton(juce::Graphics& g, bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override {
        juce::ignoreUnused(shouldDrawButtonAsHighlighted);
        const auto& img = (shouldDrawButtonAsDown || isDown()) ? imgOn : imgOff;
        if (img.isValid()) {
            g.drawImageWithin(img, 0, 0, getWidth(), getHeight(),
                              juce::RectanglePlacement::centred | juce::RectanglePlacement::onlyReduceInSize);
        }
    }
};

struct TactileDragMidiButton : public juce::Button {
    juce::Image imgOff;
    juce::Image imgOn;
    std::function<void()> onStartDrag;
    bool hasInitiatedDrag = false;

    TactileDragMidiButton() : juce::Button("drag_midi") {
        imgOff = juce::ImageCache::getFromMemory(BinaryData::dragmidi_off_png, BinaryData::dragmidi_off_pngSize);
        imgOn  = juce::ImageCache::getFromMemory(BinaryData::dragmidi_on_png,  BinaryData::dragmidi_on_pngSize);
    }

    void mouseDown(const juce::MouseEvent& e) override {
        hasInitiatedDrag = false;
        juce::Button::mouseDown(e);
    }

    void mouseDrag(const juce::MouseEvent& e) override {
        juce::Button::mouseDrag(e);
        if (!hasInitiatedDrag && e.getDistanceFromDragStart() > 3) {
            hasInitiatedDrag = true;
            if (onStartDrag) onStartDrag();
        }
    }

    void mouseUp(const juce::MouseEvent& e) override {
        hasInitiatedDrag = false;
        juce::Button::mouseUp(e);
    }

    void paintButton(juce::Graphics& g, bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override {
        juce::ignoreUnused(shouldDrawButtonAsHighlighted);
        const auto& img = (shouldDrawButtonAsDown || isDown()) ? imgOn : imgOff;
        if (img.isValid()) {
            g.drawImageWithin(img, 0, 0, getWidth(), getHeight(),
                              juce::RectanglePlacement::centred);
        } else {
            auto bounds = getLocalBounds().toFloat();
            g.setColour(shouldDrawButtonAsDown ? juce::Colour::fromRGB(45, 48, 55) : juce::Colour::fromRGB(26, 28, 32));
            g.fillRoundedRectangle(bounds, 3.0f);
            g.setColour(juce::Colours::white);
            g.setFont(CustomLookAndFeel::getRobotoFont(11.0f, true));
            g.drawText("DRAG MIDI", bounds, juce::Justification::centred);
        }
    }
};

struct TactileDragAudioButton : public juce::Button {
    juce::Image imgOff;
    juce::Image imgOn;
    std::function<void()> onStartDrag;
    bool hasInitiatedDrag = false;

    TactileDragAudioButton() : juce::Button("drag_audio") {
        imgOff = juce::ImageCache::getFromMemory(BinaryData::dragaudio_off_png, BinaryData::dragaudio_off_pngSize);
        imgOn  = juce::ImageCache::getFromMemory(BinaryData::dragaudio_on_png,  BinaryData::dragaudio_on_pngSize);
    }

    void mouseDown(const juce::MouseEvent& e) override {
        hasInitiatedDrag = false;
        juce::Button::mouseDown(e);
    }

    void mouseDrag(const juce::MouseEvent& e) override {
        juce::Button::mouseDrag(e);
        if (!hasInitiatedDrag && e.getDistanceFromDragStart() > 3) {
            hasInitiatedDrag = true;
            if (onStartDrag) onStartDrag();
        }
    }

    void mouseUp(const juce::MouseEvent& e) override {
        hasInitiatedDrag = false;
        juce::Button::mouseUp(e);
    }

    void paintButton(juce::Graphics& g, bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override {
        juce::ignoreUnused(shouldDrawButtonAsHighlighted);
        const auto& img = (shouldDrawButtonAsDown || isDown()) ? imgOn : imgOff;
        if (img.isValid()) {
            g.drawImageWithin(img, 0, 0, getWidth(), getHeight(),
                              juce::RectanglePlacement::centred);
        } else {
            auto bounds = getLocalBounds().toFloat();
            g.setColour(shouldDrawButtonAsDown ? juce::Colour::fromRGB(45, 48, 55) : juce::Colour::fromRGB(26, 28, 32));
            g.fillRoundedRectangle(bounds, 3.0f);
            g.setColour(juce::Colours::white);
            g.setFont(CustomLookAndFeel::getRobotoFont(11.0f, true));
            g.drawText("DRAG AUDIO", bounds, juce::Justification::centred);
        }
    }
};

struct TactileRecordButton : public juce::Button {
    juce::Image imgOff;
    juce::Image imgOn;

    TactileRecordButton() : juce::Button("record") {
        imgOff = juce::ImageCache::getFromMemory(BinaryData::record_off_png, BinaryData::record_off_pngSize);
        imgOn  = juce::ImageCache::getFromMemory(BinaryData::record_on_png,  BinaryData::record_on_pngSize);
    }

    void paintButton(juce::Graphics& g, bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override {
        juce::ignoreUnused(shouldDrawButtonAsHighlighted);
        const auto& img = (getToggleState() || shouldDrawButtonAsDown || isDown()) ? imgOn : imgOff;
        if (img.isValid()) {
            g.drawImageWithin(img, 0, 0, getWidth(), getHeight(),
                              juce::RectanglePlacement::centred | juce::RectanglePlacement::onlyReduceInSize);
        }
    }
};

struct TactileKeyMapButton : public juce::Button {
    juce::Image chromaticIcon;
    juce::Image whiteKeysIcon;
    std::function<bool()> isWhiteKeys;
    std::function<void()> onToggle;

    TactileKeyMapButton() : juce::Button("keymap_btn") {
        chromaticIcon = juce::ImageCache::getFromMemory(BinaryData::chromatic_keys_png, BinaryData::chromatic_keys_pngSize);
        whiteKeysIcon = juce::ImageCache::getFromMemory(BinaryData::only_white_keys_png, BinaryData::only_white_keys_pngSize);
    }

    void paintButton(juce::Graphics& g, bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override {
        juce::ignoreUnused(shouldDrawButtonAsHighlighted, shouldDrawButtonAsDown);

        // Pure flat black box with NO border and NO corner radius
        g.setColour(juce::Colour::fromRGB(0, 0, 0));
        g.fillRect(getLocalBounds());

        bool isWhite = isWhiteKeys ? isWhiteKeys() : false;

        // Draw piano keys icon on left with clean padding (NEVER cropped!)
        auto iconArea = juce::Rectangle<float>(8.0f, (float)(getHeight() - 16) / 2.0f, 20.0f, 16.0f);
        float keyW = (iconArea.getWidth() - 2.0f) / 3.0f;
        
        // 3 White keys
        g.setColour(juce::Colour::fromRGB(230, 230, 235));
        for (int k = 0; k < 3; ++k) {
            auto keyRect = juce::Rectangle<float>(iconArea.getX() + k * (keyW + 1.0f), iconArea.getY(), keyW, iconArea.getHeight());
            g.fillRoundedRectangle(keyRect, 1.8f);
        }

        if (!isWhite) {
            // Chromatic: 2 black keys overlaid
            g.setColour(juce::Colour::fromRGB(20, 20, 25));
            float blackW = keyW * 0.75f;
            float blackH = iconArea.getHeight() * 0.58f;
            g.fillRoundedRectangle(iconArea.getX() + keyW - blackW * 0.5f + 0.5f, iconArea.getY(), blackW, blackH, 1.0f);
            g.fillRoundedRectangle(iconArea.getX() + keyW * 2.0f - blackW * 0.5f + 1.5f, iconArea.getY(), blackW, blackH, 1.0f);
        }

        // Draw mode text in Minecraft font with generous spacing from icon
        g.setFont(CustomLookAndFeel::getMinecraftFont(12.5f));
        g.setColour(juce::Colours::white);
        juce::String labelText = isWhite ? "White Keys" : "Chromatic";
        g.drawText(labelText, 36, 0, getWidth() - 56, getHeight(), juce::Justification::centredLeft, true);

        // Draw navigation arrows ◂ ▸
        g.drawText(juce::String::charToString(0x25C2) + " " + juce::String::charToString(0x25B8),
                   getWidth() - 20, 0, 16, getHeight(), juce::Justification::centredRight, false);
    }

    void clicked() override {
        if (onToggle) onToggle();
        repaint();
    }
};

class ChopSampAudioProcessorEditor  : public juce::AudioProcessorEditor,
                                      public juce::FileDragAndDropTarget,
                                      public juce::Timer
{
public:
    ChopSampAudioProcessorEditor (ChopSampAudioProcessor&);
    ~ChopSampAudioProcessorEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;
    void timerCallback() override;
    
    bool isInterestedInFileDrag (const juce::StringArray& files) override;
    void filesDropped (const juce::StringArray& files, int x, int y) override;

    void updateUIFromProcessorState();

private:
    ChopSampAudioProcessor& audioProcessor;
    CustomLookAndFeel customLookAndFeel;
    
    // Top UI Components
    juce::TextButton tabButtons[MAX_SAMPLE_TABS];
    TactileRecordButton recordButton;
    juce::Label recordLabel;
    juce::Label playingLabel;
    
    juce::ComboBox sliceModeDropdown;
    juce::ComboBox gridDropdown;
    juce::Slider transientSensSlider;
    juce::Slider randomCountSlider;
    TactileSliceButton executeSliceBtn;

    TactileDragMidiButton dragMidiButton;
    TactileDragAudioButton dragAudioButton;
    juce::ComboBox playbackModeDropdown;
    TactileKeyMapButton keyMapButton;
    juce::Label sliceTypeLabel;
    juce::Label keyMapLabel;
    juce::Label playbackModeLabel;
    juce::ComboBox guiScaleDropdown;
    juce::ToggleButton helperToggleBtn;
    juce::ComboBox themeDropdown;
    juce::ComboBox pbRangeDropdown;
    juce::ComboBox rootKeyDropdown;

    juce::Slider masterVolSlider;
    juce::Label masterVolLabel;

    // Sub-components
    WaveformComponent waveformComponent;
    SliceControlsComponent sliceControlsComponent;

#define ENABLE_LAYOUT_INSPECTOR 0

    // Tooltip Window for interactive pop-up hints
    juce::TooltipWindow tooltipWindow { this, 350 };

    // Layout Configuration
    LayoutConfig layoutConfig;

#if ENABLE_LAYOUT_INSPECTOR
    LayoutInspectorComponent layoutInspector{ layoutConfig };
    juce::TextButton inspectorToggleBtn;
    bool keyPressed (const juce::KeyPress& key) override;
#endif

    juce::Image bgTexture;
    juce::Image logoImage;
    juce::Image screwImage;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ChopSampAudioProcessorEditor)
};
