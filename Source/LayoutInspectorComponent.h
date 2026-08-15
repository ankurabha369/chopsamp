#pragma once
#include <JuceHeader.h>
#include "LayoutConfig.h"
#include "CustomLookAndFeel.h"

class LayoutInspectorComponent : public juce::Component
{
public:
    std::function<void()> onLayoutChanged;
    LayoutConfig& config;

    LayoutInspectorComponent(LayoutConfig& cfg) : config(cfg)
    {
        setAlwaysOnTop(true);

        titleLabel.setText("⚙ INSPECTOR (Drag to move)", juce::dontSendNotification);
        titleLabel.setFont(CustomLookAndFeel::getRobotoFont(12.0f, true));
        titleLabel.setColour(juce::Label::textColourId, juce::Colour::fromRGB(240, 115, 125));
        titleLabel.setInterceptsMouseClicks(false, false);
        addAndMakeVisible(&titleLabel);

        closeBtn.setButtonText(juce::String::charToString(0x2715));
        closeBtn.onClick = [this]() { setVisible(false); };
        addAndMakeVisible(&closeBtn);

        for (int i = 1; i <= 30; ++i) {
            elementSelector.addItem(LayoutConfig::getElementName(i), i);
        }
        elementSelector.setSelectedId(1, juce::dontSendNotification);
        elementSelector.onChange = [this]() {
            config.selectedElementId = elementSelector.getSelectedId();
            updateSlidersForSelectedElement();
            if (onLayoutChanged) onLayoutChanged();
        };
        addAndMakeVisible(&elementSelector);

        auto setupSlider = [this](juce::Slider& s, juce::Label& l) {
            s.setSliderStyle(juce::Slider::LinearHorizontal);
            s.setTextBoxStyle(juce::Slider::TextBoxRight, false, 50, 18);
            s.onValueChange = [this]() { applySliderValuesToConfig(); };
            addAndMakeVisible(&s);

            l.setFont(CustomLookAndFeel::getRobotoFont(11.0f, true));
            l.setColour(juce::Label::textColourId, juce::Colours::white);
            addAndMakeVisible(&l);
        };

        setupSlider(sliderX, labelX); labelX.setText("X Pos:", juce::dontSendNotification);
        setupSlider(sliderY, labelY); labelY.setText("Y Pos:", juce::dontSendNotification);
        setupSlider(sliderW, labelW); labelW.setText("Width:", juce::dontSendNotification);
        setupSlider(sliderH, labelH); labelH.setText("Height:", juce::dontSendNotification);

        // Alignment Guidelines & Snap
        guidesToggleBtn.setButtonText("📏 Alignment Guides");
        guidesToggleBtn.setToggleState(config.showGuides, juce::dontSendNotification);
        guidesToggleBtn.onClick = [this]() {
            config.showGuides = guidesToggleBtn.getToggleState();
            if (onLayoutChanged) onLayoutChanged();
        };
        addAndMakeVisible(&guidesToggleBtn);

        setupSlider(guideHSlider, guideHLabel); guideHLabel.setText("Guide-Y:", juce::dontSendNotification);
        guideHSlider.setRange(0, 600, 1.0);
        guideHSlider.setValue(config.guideLineH, juce::dontSendNotification);
        guideHSlider.onValueChange = [this]() {
            config.guideLineH = (int)guideHSlider.getValue();
            if (onLayoutChanged) onLayoutChanged();
        };

        setupSlider(guideVSlider, guideVLabel); guideVLabel.setText("Guide-X:", juce::dontSendNotification);
        guideVSlider.setRange(0, 1000, 1.0);
        guideVSlider.setValue(config.guideLineV, juce::dontSendNotification);
        guideVSlider.onValueChange = [this]() {
            config.guideLineV = (int)guideVSlider.getValue();
            if (onLayoutChanged) onLayoutChanged();
        };

        snapGuideBtn.setButtonText("🎯 Snap to Guides");
        snapGuideBtn.onClick = [this]() {
            int id = elementSelector.getSelectedId();
            auto* elem = config.getElementById(id);
            if (elem) {
                // If element is in slice controls area, adjust relative coords
                if (id >= 2 && id <= 20) {
                    elem->y = juce::jmax(0, config.guideLineH - 280);
                    elem->x = juce::jmax(0, config.guideLineV - 18);
                } else {
                    elem->y = config.guideLineH;
                    elem->x = config.guideLineV;
                }
                updateSlidersForSelectedElement();
                if (onLayoutChanged) onLayoutChanged();
                statusLabel.setText("Snapped to Guide Lines!", juce::dontSendNotification);
            }
        };
        addAndMakeVisible(&snapGuideBtn);

        opacityLabel.setText("Opacity:", juce::dontSendNotification);
        opacityLabel.setFont(CustomLookAndFeel::getRobotoFont(10.5f, true));
        opacityLabel.setColour(juce::Label::textColourId, juce::Colour::fromRGB(180, 180, 180));
        addAndMakeVisible(&opacityLabel);

        opacitySlider.setSliderStyle(juce::Slider::LinearHorizontal);
        opacitySlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
        opacitySlider.setRange(0.15, 1.0, 0.05);
        opacitySlider.setValue(panelAlpha, juce::dontSendNotification);
        opacitySlider.onValueChange = [this]() {
            panelAlpha = (float)opacitySlider.getValue();
            repaint();
        };
        addAndMakeVisible(&opacitySlider);

        saveBtn.setButtonText("💾 Save to layout.json");
        saveBtn.onClick = [this]() {
            config.saveToFile();
            statusLabel.setText("Saved to layout.json!", juce::dontSendNotification);
        };
        addAndMakeVisible(&saveBtn);

        copyCodeBtn.setButtonText("📋 Copy C++ Code");
        copyCodeBtn.onClick = [this]() { copyCppCodeToClipboard(); };
        addAndMakeVisible(&copyCodeBtn);

        // Reset Selected Element ONLY (As requested!)
        resetBtn.setButtonText("🔄 Reset Selected to Default");
        resetBtn.onClick = [this]() {
            int id = elementSelector.getSelectedId();
            config.resetElementToDefault(id);
            updateSlidersForSelectedElement();
            if (onLayoutChanged) onLayoutChanged();
            statusLabel.setText("Reset " + LayoutConfig::getElementName(id) + "!", juce::dontSendNotification);
        };
        addAndMakeVisible(&resetBtn);

        resetAllBtn.setButtonText("⚠️ Reset All Defaults");
        resetAllBtn.onClick = [this]() {
            config = LayoutConfig();
            updateSlidersForSelectedElement();
            if (onLayoutChanged) onLayoutChanged();
            statusLabel.setText("Reset all elements to defaults!", juce::dontSendNotification);
        };
        addAndMakeVisible(&resetAllBtn);

        statusLabel.setFont(CustomLookAndFeel::getRobotoFont(10.5f, false));
        statusLabel.setColour(juce::Label::textColourId, juce::Colour::fromRGB(46, 204, 113));
        statusLabel.setJustificationType(juce::Justification::centred);
        addAndMakeVisible(&statusLabel);

        updateSlidersForSelectedElement();
    }

    void selectElement(int id)
    {
        if (id >= 1 && id <= 30)
        {
            config.selectedElementId = id;
            elementSelector.setSelectedId(id, juce::dontSendNotification);
            updateSlidersForSelectedElement();
            if (onLayoutChanged) onLayoutChanged();
        }
    }

    void mouseDown(const juce::MouseEvent& e) override
    {
        dragger.startDraggingComponent(this, e);
    }

    void mouseDrag(const juce::MouseEvent& e) override
    {
        dragger.dragComponent(this, e, nullptr);
    }

    void paint(juce::Graphics& g) override
    {
        g.setColour(juce::Colour::fromRGB(8, 8, 12).withAlpha(panelAlpha));
        g.fillRect(getLocalBounds());

        g.setColour(juce::Colour::fromRGB(240, 115, 125).withAlpha(0.20f));
        g.fillRect(0, 0, getWidth(), 28);

        g.setColour(juce::Colour::fromRGB(240, 115, 125).withAlpha(0.75f));
        g.drawRect(getLocalBounds(), 1.5f);
    }

    void resized() override
    {
        auto area = getLocalBounds().reduced(8);
        
        auto topRow = area.removeFromTop(22);
        closeBtn.setBounds(topRow.removeFromRight(22));
        titleLabel.setBounds(topRow);

        area.removeFromTop(6);
        elementSelector.setBounds(area.removeFromTop(24));
        area.removeFromTop(6);

        auto placeSliderRow = [&area](juce::Label& l, juce::Slider& s) {
            auto row = area.removeFromTop(20);
            l.setBounds(row.removeFromLeft(60));
            s.setBounds(row);
            area.removeFromTop(3);
        };

        placeSliderRow(labelX, sliderX);
        placeSliderRow(labelY, sliderY);
        placeSliderRow(labelW, sliderW);
        placeSliderRow(labelH, sliderH);

        area.removeFromTop(4);
        guidesToggleBtn.setBounds(area.removeFromTop(20));
        placeSliderRow(guideHLabel, guideHSlider);
        placeSliderRow(guideVLabel, guideVSlider);
        snapGuideBtn.setBounds(area.removeFromTop(22));

        area.removeFromTop(4);
        auto opacRow = area.removeFromTop(18);
        opacityLabel.setBounds(opacRow.removeFromLeft(60));
        opacitySlider.setBounds(opacRow);

        area.removeFromTop(6);
        saveBtn.setBounds(area.removeFromTop(24));
        area.removeFromTop(4);
        copyCodeBtn.setBounds(area.removeFromTop(24));
        area.removeFromTop(4);
        resetBtn.setBounds(area.removeFromTop(24));
        area.removeFromTop(4);
        resetAllBtn.setBounds(area.removeFromTop(20));
        area.removeFromTop(2);
        statusLabel.setBounds(area.removeFromTop(16));
    }

private:
    juce::ComponentDragger dragger;
    float panelAlpha = 0.68f;

    juce::Label titleLabel;
    juce::TextButton closeBtn;
    juce::ComboBox elementSelector;

    juce::Label labelX, labelY, labelW, labelH;
    juce::Slider sliderX, sliderY, sliderW, sliderH;

    juce::ToggleButton guidesToggleBtn;
    juce::Label guideHLabel, guideVLabel;
    juce::Slider guideHSlider, guideVSlider;
    juce::TextButton snapGuideBtn;

    juce::Label opacityLabel;
    juce::Slider opacitySlider;

    juce::TextButton saveBtn;
    juce::TextButton copyCodeBtn;
    juce::TextButton resetBtn;
    juce::TextButton resetAllBtn;
    juce::Label statusLabel;

    bool isUpdatingSliders = false;

    void updateSlidersForSelectedElement()
    {
        isUpdatingSliders = true;
        int id = elementSelector.getSelectedId();
        auto* elem = config.getElementById(id);

        if (elem) {
            sliderX.setRange(0, 1000, 1.0);
            sliderX.setValue(elem->x, juce::dontSendNotification);

            sliderY.setRange(0, 600, 1.0);
            sliderY.setValue(elem->y, juce::dontSendNotification);

            sliderW.setRange(2, 400, 1.0);
            sliderW.setValue(elem->w, juce::dontSendNotification);

            sliderH.setRange(2, 300, 1.0);
            sliderH.setValue(elem->h, juce::dontSendNotification);
        }

        isUpdatingSliders = false;
    }

    void applySliderValuesToConfig()
    {
        if (isUpdatingSliders) return;
        int id = elementSelector.getSelectedId();
        auto* elem = config.getElementById(id);

        if (elem) {
            elem->x = (int)sliderX.getValue();
            elem->y = (int)sliderY.getValue();
            elem->w = (int)sliderW.getValue();
            elem->h = (int)sliderH.getValue();
        }

        if (onLayoutChanged) onLayoutChanged();
    }

    void copyCppCodeToClipboard()
    {
        juce::String code;
        code << "// --- ChopSamp Custom Layout ---\n";
        for (int i = 1; i <= 30; ++i) {
            auto* e = config.getElementById(i);
            if (e) {
                code << "// " << LayoutConfig::getElementName(i) << "\n";
                code << "setPos(" << e->x << ", " << e->y << ", " << e->w << ", " << e->h << ");\n";
            }
        }
        
        juce::SystemClipboard::copyTextToClipboard(code);
        statusLabel.setText("Copied C++ layout code!", juce::dontSendNotification);
    }
};
