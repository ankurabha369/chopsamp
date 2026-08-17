#include "SliceControlsComponent.h"
#include "LayoutConfig.h"

SliceControlsComponent::SliceControlsComponent(ChopSampAudioProcessor& p)
    : audioProcessor(p)
{
    setupSlider(crossfadeSlider, crossfadeLabel, "XFade");
    crossfadeSlider.setRange(1.0, 100.0, 1.0);
    crossfadeSlider.setValue(20.0);
    crossfadeSlider.setDoubleClickReturnValue(true, 20.0);
    crossfadeSlider.textFromValueFunction = [](double v) { return juce::String((int)v) + "ms"; };

    setupSlider(startTrimSlider, startTrimLabel, "Start");
    startTrimSlider.setRange(-2000.0, 2000.0, 1.0);
    startTrimSlider.setValue(0.0);
    startTrimSlider.setDoubleClickReturnValue(true, 0.0);
    startTrimSlider.textFromValueFunction = [](double v) { return (v > 0 ? "+" : "") + juce::String((int)v) + "ms"; };

    setupSlider(endTrimSlider, endTrimLabel, "End");
    endTrimSlider.setRange(-2000.0, 2000.0, 1.0);
    endTrimSlider.setValue(0.0);
    endTrimSlider.setDoubleClickReturnValue(true, 0.0);
    endTrimSlider.textFromValueFunction = [](double v) { return (v > 0 ? "+" : "") + juce::String((int)v) + "ms"; };

    setupSlider(volSlider, volLabel, "Vol"); volSlider.setRange(0.0, 2.0, 0.01); volSlider.setValue(1.0); volSlider.setDoubleClickReturnValue(true, 1.0);
    setupSlider(panSlider, panLabel, "Pan"); panSlider.setRange(-1.0, 1.0, 0.01); panSlider.setValue(0.0); panSlider.setDoubleClickReturnValue(true, 0.0);
    setupSlider(pitchSlider, pitchLabel, "Pitch"); pitchSlider.setRange(-24.0, 24.0, 1.0); pitchSlider.setValue(0.0); pitchSlider.setDoubleClickReturnValue(true, 0.0);
    setupSlider(hpfSlider, hpfLabel, "HPF");
    hpfSlider.setRange(20.0, 10000.0, 1.0);
    hpfSlider.setSkewFactorFromMidPoint(500.0);
    hpfSlider.setValue(20.0);
    hpfSlider.setDoubleClickReturnValue(true, 20.0);
    hpfSlider.textFromValueFunction = [](double v) {
        return v >= 1000.0 ? juce::String(v / 1000.0, 1) + "k" : juce::String((int)v) + "Hz";
    };

    setupSlider(lpfSlider, lpfLabel, "LPF");
    lpfSlider.setRange(20.0, 20000.0, 1.0);
    lpfSlider.setSkewFactorFromMidPoint(1000.0);
    lpfSlider.setValue(20000.0);
    lpfSlider.setDoubleClickReturnValue(true, 20000.0);
    lpfSlider.textFromValueFunction = [](double v) {
        return v >= 1000.0 ? juce::String(v / 1000.0, 1) + "k" : juce::String((int)v) + "Hz";
    };

    setupSlider(attackSlider, attackLabel, "Attack"); attackSlider.setRange(0.0, 1000.0, 1.0); attackSlider.setValue(10.0); attackSlider.setDoubleClickReturnValue(true, 10.0);
    setupSlider(decaySlider, decayLabel, "Decay"); decaySlider.setRange(0.0, 2000.0, 1.0); decaySlider.setValue(100.0); decaySlider.setDoubleClickReturnValue(true, 100.0);
    setupSlider(sustainSlider, sustainLabel, "Sustain"); sustainSlider.setRange(0.0, 1.0, 0.01); sustainSlider.setValue(1.0); sustainSlider.setDoubleClickReturnValue(true, 1.0);
    setupSlider(releaseSlider, releaseLabel, "Release"); releaseSlider.setRange(10.0, 5000.0, 1.0); releaseSlider.setValue(100.0); releaseSlider.setDoubleClickReturnValue(true, 100.0);

    crossfadeSlider.setTooltip("Crossfade: De-click smoothing length (1-100ms) at slice boundaries.");
    startTrimSlider.setTooltip("Start Trim: Offset slice start point (-2000 to +2000ms).");
    endTrimSlider.setTooltip("End Trim: Offset slice end point (-2000 to +2000ms).");
    volSlider.setTooltip("Slice Volume: Output gain level for the selected slice.");
    panSlider.setTooltip("Slice Pan: Stereo panning for the selected slice.");
    pitchSlider.setTooltip("Slice Pitch: Transpose pitch (-24 to +24 semitones) for the selected slice.");
    hpfSlider.setTooltip("High Pass Filter: Cut low frequencies (20Hz - 10kHz).");
    lpfSlider.setTooltip("Low Pass Filter: Cut high frequencies (20Hz - 20kHz).");
    attackSlider.setTooltip("Attack: Time taken for volume to reach peak level (0 - 1000ms).");
    decaySlider.setTooltip("Decay: Time taken for volume to drop to sustain level (0 - 2000ms).");
    sustainSlider.setTooltip("Sustain: Steady volume level while note is held (0.0 - 1.0).");
    releaseSlider.setTooltip("Release: Time taken for volume to fade out on release (10 - 5000ms).");
    reverseButton.setTooltip("Reverse: Toggle reverse playback for the selected slice.");
    colorDropdownButton.setTooltip("Color: Change slice waveform and background color theme.");
    applyAdsrToAllBtn.setTooltip("Apply to All: Copy the current slice's ADSR envelope to all slices.");

    reverseLabel.setText("Reverse", juce::dontSendNotification);
    reverseLabel.setFont(CustomLookAndFeel::getRobotoFont(10.5f, true));
    reverseLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(&reverseLabel);

    reverseButton.setClickingTogglesState(true);
    addAndMakeVisible(&reverseButton);
    reverseButton.onClick = [this]() {
        if (selectedSliceIndex >= 0 && selectedSliceIndex < (int)audioProcessor.samples[audioProcessor.currentTab].markers.size()) {
            bool newState = reverseButton.getToggleState();
            audioProcessor.samples[audioProcessor.currentTab].markers[selectedSliceIndex].params.reverse = newState;
            repaint();
        }
    };

    colorLabel.setText("Color", juce::dontSendNotification);
    colorLabel.setFont(CustomLookAndFeel::getRobotoFont(10.5f, true));
    colorLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(&colorLabel);

    colorDropdownButton.getCurrentColour = [this]() -> juce::Colour {
        if (selectedSliceIndex >= 0 && selectedSliceIndex < (int)audioProcessor.samples[audioProcessor.currentTab].markers.size()) {
            return juce::Colour((juce::uint32)audioProcessor.samples[audioProcessor.currentTab].markers[selectedSliceIndex].params.colorARGB);
        }
        return juce::Colour::fromRGB(255, 152, 0);
    };
    colorDropdownButton.onColourSelected = [this](juce::Colour c) {
        if (selectedSliceIndex >= 0 && selectedSliceIndex < (int)audioProcessor.samples[audioProcessor.currentTab].markers.size()) {
            audioProcessor.samples[audioProcessor.currentTab].markers[selectedSliceIndex].params.colorARGB = c.getARGB();
            if (getParentComponent()) getParentComponent()->repaint();
            repaint();
        }
    };
    addAndMakeVisible(&colorDropdownButton);

    applyAdsrToAllBtn.setTooltip("Apply current slice's ADSR values to all slices in this sample");
    addAndMakeVisible(&applyAdsrToAllBtn);
    applyAdsrToAllBtn.onClick = [this]() {
        auto& sample = audioProcessor.samples[audioProcessor.currentTab];
        if (selectedSliceIndex >= 0 && selectedSliceIndex < (int)sample.markers.size()) {
            const auto& src = sample.markers[selectedSliceIndex].params;
            for (int i = 0; i < (int)sample.markers.size(); ++i) {
                if (i != selectedSliceIndex) {
                    sample.markers[i].params.attackMs = src.attackMs;
                    sample.markers[i].params.decayMs = src.decayMs;
                    sample.markers[i].params.sustainLevel = src.sustainLevel;
                    sample.markers[i].params.releaseMs = src.releaseMs;
                }
            }
            repaint();
        }
    };

    // 20 Sample Tab buttons setup with pixel font
    for (int i = 0; i < MAX_SAMPLE_TABS; ++i) {
        tabButtons[i].tabIndex = i;
        tabButtons[i].isActiveCallback = [this, i]() {
            return audioProcessor.currentTab == i;
        };
        addAndMakeVisible(&tabButtons[i]);
        tabButtons[i].onClick = [this, i]() {
            audioProcessor.currentTab = i;
            if (getParentComponent()) getParentComponent()->repaint();
            repaint();
        };
    }

    pbLabel.setText("Ptich Bend:", juce::dontSendNotification);
    pbLabel.setFont(CustomLookAndFeel::getRobotoFont(10.5f, true));
    addAndMakeVisible(&pbLabel);
}

SliceControlsComponent::~SliceControlsComponent()
{
}

void SliceControlsComponent::setDropdowns(juce::ComboBox* pbDropdown, juce::ComboBox* rootDropdown)
{
    pbRangeDropdownPtr = pbDropdown;
    rootKeyDropdownPtr = rootDropdown;
    if (pbRangeDropdownPtr) addAndMakeVisible(pbRangeDropdownPtr);
    if (rootKeyDropdownPtr) addAndMakeVisible(rootKeyDropdownPtr);
}

void SliceControlsComponent::setupSlider(juce::Slider& s, juce::Label& lbl, const juce::String& name)
{
    s.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    s.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 48, 14);
    addAndMakeVisible(&s);

    lbl.setText(name, juce::dontSendNotification);
    lbl.setFont(CustomLookAndFeel::getRobotoFont(10.5f, true));
    lbl.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(&lbl);

    s.onValueChange = [this, &s]() { updateParametersFromSlider(s); };
}

void SliceControlsComponent::updateParametersFromSlider(juce::Slider& s)
{
    if (selectedSliceIndex >= 0 && selectedSliceIndex < (int)audioProcessor.samples[audioProcessor.currentTab].markers.size()) {
        auto& p = audioProcessor.samples[audioProcessor.currentTab].markers[selectedSliceIndex].params;
        if (&s == &volSlider) p.volume = (float)s.getValue();
        if (&s == &panSlider) p.pan = (float)s.getValue();
        if (&s == &pitchSlider) p.pitchSemi = (float)s.getValue();
        if (&s == &attackSlider) p.attackMs = (float)s.getValue();
        if (&s == &decaySlider) p.decayMs = (float)s.getValue();
        if (&s == &sustainSlider) p.sustainLevel = (float)s.getValue();
        if (&s == &releaseSlider) p.releaseMs = (float)s.getValue();
        if (&s == &crossfadeSlider) p.crossfadeMs = (float)s.getValue();
        if (&s == &startTrimSlider) p.startTrimMs = (float)s.getValue();
        if (&s == &endTrimSlider) p.endTrimMs = (float)s.getValue();
        if (&s == &lpfSlider) p.lpfCutoff = (float)s.getValue();
        if (&s == &hpfSlider) p.hpfCutoff = (float)s.getValue();
        repaint();
    }
}

void SliceControlsComponent::setSelectedSliceIndex(int index)
{
    selectedSliceIndex = index;
    if (selectedSliceIndex >= 0 && selectedSliceIndex < (int)audioProcessor.samples[audioProcessor.currentTab].markers.size()) {
        auto& p = audioProcessor.samples[audioProcessor.currentTab].markers[selectedSliceIndex].params;
        volSlider.setValue(p.volume, juce::dontSendNotification);
        panSlider.setValue(p.pan, juce::dontSendNotification);
        pitchSlider.setValue(p.pitchSemi, juce::dontSendNotification);
        attackSlider.setValue(p.attackMs, juce::dontSendNotification);
        decaySlider.setValue(p.decayMs, juce::dontSendNotification);
        sustainSlider.setValue(p.sustainLevel, juce::dontSendNotification);
        releaseSlider.setValue(p.releaseMs, juce::dontSendNotification);
        crossfadeSlider.setValue(p.crossfadeMs, juce::dontSendNotification);
        startTrimSlider.setValue(p.startTrimMs, juce::dontSendNotification);
        endTrimSlider.setValue(p.endTrimMs, juce::dontSendNotification);
        lpfSlider.setValue(p.lpfCutoff, juce::dontSendNotification);
        hpfSlider.setValue(p.hpfCutoff, juce::dontSendNotification);
        reverseButton.setToggleState(p.reverse, juce::dontSendNotification);
    }
    repaint();
}

void SliceControlsComponent::paint(juce::Graphics& g)
{
    auto theme = getTheme(audioProcessor.currentTheme.load());
    
    volLabel.setColour(juce::Label::textColourId, theme.textColour);
    panLabel.setColour(juce::Label::textColourId, theme.textColour);
    pitchLabel.setColour(juce::Label::textColourId, theme.textColour);
    attackLabel.setColour(juce::Label::textColourId, theme.textColour);
    decayLabel.setColour(juce::Label::textColourId, theme.textColour);
    sustainLabel.setColour(juce::Label::textColourId, theme.textColour);
    releaseLabel.setColour(juce::Label::textColourId, theme.textColour);
    crossfadeLabel.setColour(juce::Label::textColourId, theme.textColour);
    startTrimLabel.setColour(juce::Label::textColourId, theme.textColour);
    endTrimLabel.setColour(juce::Label::textColourId, theme.textColour);
    lpfLabel.setColour(juce::Label::textColourId, theme.textColour);
    hpfLabel.setColour(juce::Label::textColourId, theme.textColour);
    reverseLabel.setColour(juce::Label::textColourId, theme.textColour);
    colorLabel.setColour(juce::Label::textColourId, theme.textColour);
    pbLabel.setColour(juce::Label::textColourId, theme.textColour);

    volSlider.setColour(juce::Slider::textBoxTextColourId, theme.textColour);
    panSlider.setColour(juce::Slider::textBoxTextColourId, theme.textColour);
    pitchSlider.setColour(juce::Slider::textBoxTextColourId, theme.textColour);
    attackSlider.setColour(juce::Slider::textBoxTextColourId, theme.textColour);
    decaySlider.setColour(juce::Slider::textBoxTextColourId, theme.textColour);
    sustainSlider.setColour(juce::Slider::textBoxTextColourId, theme.textColour);
    releaseSlider.setColour(juce::Slider::textBoxTextColourId, theme.textColour);
    crossfadeSlider.setColour(juce::Slider::textBoxTextColourId, theme.textColour);
    startTrimSlider.setColour(juce::Slider::textBoxTextColourId, theme.textColour);
    endTrimSlider.setColour(juce::Slider::textBoxTextColourId, theme.textColour);
    lpfSlider.setColour(juce::Slider::textBoxTextColourId, theme.textColour);
    hpfSlider.setColour(juce::Slider::textBoxTextColourId, theme.textColour);

    volSlider.setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    panSlider.setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    pitchSlider.setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    attackSlider.setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    decaySlider.setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    sustainSlider.setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    releaseSlider.setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    crossfadeSlider.setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    startTrimSlider.setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    endTrimSlider.setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    lpfSlider.setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    hpfSlider.setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);

    // Draw Tab Grid Box (Pure Flat Solid Black, NO border, 0 corner radius)
    g.setColour(juce::Colour::fromRGB(0, 0, 0));
    g.fillRect(tabGridArea);

    // Draw ADSR Graph Box (Pure Flat Solid Black, NO border, 0 corner radius)
    g.setColour(juce::Colour::fromRGB(0, 0, 0));
    g.fillRect(adsrGraphArea);

    // ADSR Grid Background Lines (Oscilloscope / Synthesizer Grid)
    g.setColour(juce::Colour::fromRGB(22, 24, 30));
    for (int gx = adsrGraphArea.getX() + 24; gx < adsrGraphArea.getRight(); gx += 24) {
        g.drawVerticalLine(gx, (float)adsrGraphArea.getY(), (float)adsrGraphArea.getBottom());
    }
    for (int gy = adsrGraphArea.getY() + 16; gy < adsrGraphArea.getBottom(); gy += 16) {
        g.drawHorizontalLine(gy, (float)adsrGraphArea.getX(), (float)adsrGraphArea.getRight());
    }

    auto pts = getAdsrPoints();

    juce::Path adsrPath;
    adsrPath.startNewSubPath(pts.pStart);
    adsrPath.lineTo(pts.pAttack);
    adsrPath.lineTo(pts.pDecay);
    adsrPath.lineTo(pts.pSustain);
    adsrPath.lineTo(pts.pRelease);
    
    // Crisp White Envelope Curve
    g.setColour(juce::Colours::white);
    g.strokePath(adsrPath, juce::PathStrokeType(2.5f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    
    // Draw Node Control Points (larger, high-visibility white dots matching design)
    auto drawNode = [&g](const juce::Point<float>& pt) {
        g.setColour(juce::Colour::fromRGB(0, 0, 0));
        g.fillEllipse(pt.x - 5.5f, pt.y - 5.5f, 11.0f, 11.0f);
        g.setColour(juce::Colours::white);
        g.fillEllipse(pt.x - 4.5f, pt.y - 4.5f, 9.0f, 9.0f);
    };
    drawNode(pts.pAttack);
    drawNode(pts.pDecay);
    drawNode(pts.pSustain);
    drawNode(pts.pRelease);

    // Highlight Selected Element if in Inspector mode
    if (layoutConfigPtr && layoutConfigPtr->selectedElementId >= 2 && layoutConfigPtr->selectedElementId <= 20) {
        auto* elem = layoutConfigPtr->getElementById(layoutConfigPtr->selectedElementId);
        if (elem) {
            g.setColour(juce::Colour::fromRGB(240, 115, 125));
            g.drawRect(elem->toRect().expanded(2), 1.5f);
        }
    }
}

SliceControlsComponent::AdsrPoints SliceControlsComponent::getAdsrPoints() const
{
    float maxMs = 5000.0f;
    float atkMs = (float)attackSlider.getValue();
    float decMs = (float)decaySlider.getValue();
    float susLvl = (float)sustainSlider.getValue();
    float relMs = (float)releaseSlider.getValue();

    float aw = juce::jlimit(12.0f, (float)adsrGraphArea.getWidth() * 0.32f, (atkMs / maxMs) * adsrGraphArea.getWidth() + 10.0f);
    float dw = juce::jlimit(12.0f, (float)adsrGraphArea.getWidth() * 0.32f, (decMs / maxMs) * adsrGraphArea.getWidth() + 12.0f);
    float rw = juce::jlimit(12.0f, (float)adsrGraphArea.getWidth() * 0.32f, (relMs / maxMs) * adsrGraphArea.getWidth() + 12.0f);
    float sw = 35.0f; // sustain width

    float startX = (float)adsrGraphArea.getX() + 8.0f;
    float startY = (float)adsrGraphArea.getBottom() - 8.0f;
    float peakY = (float)adsrGraphArea.getY() + 10.0f;
    float sustainY = (float)adsrGraphArea.getBottom() - 10.0f - (susLvl * (adsrGraphArea.getHeight() - 22.0f));

    AdsrPoints pts;
    pts.pStart   = { startX, startY };
    pts.pAttack  = { startX + aw, peakY };
    pts.pDecay   = { startX + aw + dw, sustainY };
    pts.pSustain = { startX + aw + dw + sw, sustainY };
    pts.pRelease = { juce::jmin((float)adsrGraphArea.getRight() - 8.0f, startX + aw + dw + sw + rw), startY };
    return pts;
}

void SliceControlsComponent::resized()
{
    auto placeKnob = [](juce::Slider& s, juce::Label& l, const ElementRect& r) {
        auto rect = r.toRect();
        l.setBounds(rect.removeFromTop(13));
        s.setBounds(rect);
    };

    if (layoutConfigPtr) {
        placeKnob(crossfadeSlider, crossfadeLabel, layoutConfigPtr->xfadeKnob);
        placeKnob(startTrimSlider, startTrimLabel, layoutConfigPtr->startTrimKnob);
        placeKnob(endTrimSlider,   endTrimLabel,   layoutConfigPtr->endTrimKnob);

        auto revR = layoutConfigPtr->reverseBtn.toRect();
        reverseLabel.setBounds(revR.removeFromTop(13));
        reverseButton.setBounds(revR.withSizeKeepingCentre(layoutConfigPtr->reverseBtn.w, layoutConfigPtr->reverseBtn.h - 13));

        auto colR = layoutConfigPtr->colorDropdown.toRect();
        colorLabel.setBounds(colR.removeFromTop(13));
        colorDropdownButton.setBounds(colR.withSizeKeepingCentre(layoutConfigPtr->colorDropdown.w, layoutConfigPtr->colorDropdown.h - 13));

        placeKnob(volSlider,   volLabel,   layoutConfigPtr->volKnob);
        placeKnob(panSlider,   panLabel,   layoutConfigPtr->panKnob);
        placeKnob(pitchSlider, pitchLabel, layoutConfigPtr->pitchKnob);
        placeKnob(hpfSlider,   hpfLabel,   layoutConfigPtr->hpfKnob);
        placeKnob(lpfSlider,   lpfLabel,   layoutConfigPtr->lpfKnob);

        tabGridArea = layoutConfigPtr->tabGrid.toRect();
        auto tabBox = tabGridArea.reduced(4, 4);
        auto tabRow1 = tabBox.removeFromTop(tabBox.getHeight() / 2);
        auto tabRow2 = tabBox;
        int tabW = tabRow1.getWidth() / 10;
        for (int i = 0;  i < 10; ++i) {
            tabButtons[i].setTooltip("Slot " + juce::String(i + 1) + ": Select sample slot / drop audio here.");
            tabButtons[i].setBounds(tabRow1.removeFromLeft(tabW));
        }
        for (int i = 10; i < 20; ++i) {
            tabButtons[i].setTooltip("Slot " + juce::String(i + 1) + ": Select sample slot / drop audio here.");
            tabButtons[i].setBounds(tabRow2.removeFromLeft(tabW));
        }

        adsrGraphArea = layoutConfigPtr->adsrGraph.toRect();
        applyAdsrToAllBtn.setBounds(layoutConfigPtr->applyToAllBtn.toRect());

        placeKnob(attackSlider,  attackLabel,  layoutConfigPtr->attackKnob);
        placeKnob(decaySlider,   decayLabel,   layoutConfigPtr->decayKnob);
        placeKnob(sustainSlider, sustainLabel, layoutConfigPtr->sustainKnob);
        placeKnob(releaseSlider, releaseLabel, layoutConfigPtr->releaseKnob);

        pbLabel.setBounds(layoutConfigPtr->pbRangeDropdown.x, layoutConfigPtr->pbRangeDropdown.y - 16, 80, 14);
        if (pbRangeDropdownPtr) pbRangeDropdownPtr->setBounds(layoutConfigPtr->pbRangeDropdown.toRect());
        if (rootKeyDropdownPtr) rootKeyDropdownPtr->setBounds(layoutConfigPtr->rootKeyDropdown.toRect());
    }
}

void SliceControlsComponent::mouseDown(const juce::MouseEvent& e)
{
    juce::ignoreUnused(e);
}

void SliceControlsComponent::mouseDrag(const juce::MouseEvent& e)
{
    juce::ignoreUnused(e);
}

void SliceControlsComponent::mouseUp(const juce::MouseEvent& e)
{
    juce::ignoreUnused(e);
}
