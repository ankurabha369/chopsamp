#include "PluginProcessor.h"
#include "PluginEditor.h"

ChopSampAudioProcessorEditor::ChopSampAudioProcessorEditor (ChopSampAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p),
      waveformComponent(p), sliceControlsComponent(p)
{
    setLookAndFeel(&customLookAndFeel);
    sliceControlsComponent.setLookAndFeel(&customLookAndFeel);
    waveformComponent.setLookAndFeel(&customLookAndFeel);
    juce::LookAndFeel::setDefaultLookAndFeel(&customLookAndFeel);
    
    for (int i = 0; i < MAX_SAMPLE_TABS; ++i)
    {
        tabButtons[i].setButtonText(juce::String(i + 1));
        tabButtons[i].onClick = [this, i] { audioProcessor.currentTab = i; repaint(); waveformComponent.repaint(); sliceControlsComponent.repaint(); };
        addAndMakeVisible(&tabButtons[i]);
    }

    recordButton.setTooltip("Record: Arm and record live incoming audio from your DAW track into current sample slot.");
    recordButton.onClick = [this] {
        if (audioProcessor.isRecording)
        {
            audioProcessor.stopRecording();
            recordButton.setToggleState(false, juce::dontSendNotification);
            repaint();
            waveformComponent.repaint();
        }
        else
        {
            audioProcessor.recordingSampleIndex = 0;
            audioProcessor.isRecording = true;
            recordButton.setToggleState(true, juce::dontSendNotification);
        }
    };
    addAndMakeVisible(&recordButton);

    // Hierarchical Slicing Controls
    sliceModeDropdown.addItem("Slice Grid", 1);
    sliceModeDropdown.addItem("Slice Transients", 2);
    sliceModeDropdown.addItem("Slice Random", 3);
    sliceModeDropdown.addItem("Clear Slices", 4);
    sliceModeDropdown.setSelectedId(1);
    sliceModeDropdown.setTooltip("Slice Mode: Choose auto-slicing algorithm (Grid, Transients, or Random).");
    addAndMakeVisible(&sliceModeDropdown);

    gridDropdown.addItem("1/2 Note", 1);
    gridDropdown.addItem("1/4 Note", 2);
    gridDropdown.addItem("1/4 Triplet", 3);
    gridDropdown.addItem("1/8 Note", 4);
    gridDropdown.addItem("1/8 Triplet", 5);
    gridDropdown.addItem("1/16 Note", 6);
    gridDropdown.setSelectedId(2);
    gridDropdown.setTooltip("Grid Division: Slices sample equally by musical note duration.");
    addAndMakeVisible(&gridDropdown);

    transientSensSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    transientSensSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 45, 20);
    transientSensSlider.setRange(0.0, 100.0, 1.0);
    transientSensSlider.setValue(50.0);
    transientSensSlider.setDoubleClickReturnValue(true, 50.0);
    transientSensSlider.textFromValueFunction = [](double v) { return juce::String((int)v) + "%"; };
    transientSensSlider.setTooltip("Transient Sensitivity: Detection threshold for transient auto-slicing.");
    addChildComponent(&transientSensSlider);

    randomCountSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    randomCountSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 45, 20);
    randomCountSlider.setRange(1.0, 64.0, 1.0);
    randomCountSlider.setValue(16.0);
    randomCountSlider.setDoubleClickReturnValue(true, 16.0);
    randomCountSlider.setTooltip("Random Slices: Number of random slice points to generate across sample.");
    addChildComponent(&randomCountSlider);

    sliceModeDropdown.onChange = [this] {
        int mode = sliceModeDropdown.getSelectedId();
        gridDropdown.setVisible(mode == 1);
        transientSensSlider.setVisible(mode == 2);
        randomCountSlider.setVisible(mode == 3);
        resized();
    };

    executeSliceBtn.setTooltip("Slice: Execute auto-slicing with selected algorithm on current audio.");
    executeSliceBtn.onClick = [this] {
        int mode = sliceModeDropdown.getSelectedId();
        if (mode == 1) { // Grid
            double val = 0.25;
            switch(gridDropdown.getSelectedId()) {
                case 1: val = 0.5; break;
                case 2: val = 0.25; break;
                case 3: val = 0.25 * (2.0/3.0); break;
                case 4: val = 0.125; break;
                case 5: val = 0.125 * (2.0/3.0); break;
                case 6: val = 0.0625; break;
            }
            audioProcessor.sliceByGrid(audioProcessor.currentTab, val);
        } else if (mode == 2) { // Transient
            audioProcessor.sliceByTransients(audioProcessor.currentTab, (float)transientSensSlider.getValue());
        } else if (mode == 3) { // Random
            audioProcessor.sliceRandom(audioProcessor.currentTab, (int)randomCountSlider.getValue());
        } else if (mode == 4) { // Clear
            audioProcessor.clearSlices(audioProcessor.currentTab);
        }
        waveformComponent.repaint();
        sliceControlsComponent.repaint();
    };
    addAndMakeVisible(&executeSliceBtn);

    dragMidiButton.setTooltip("Drag MIDI: Drag generated slice MIDI sequence directly into your DAW project.");
    dragMidiButton.onStartDrag = [this] {
        juce::File midiFile = audioProcessor.exportMidiFile(audioProcessor.currentTab);
        if (midiFile.existsAsFile()) {
            juce::DragAndDropContainer::performExternalDragDropOfFiles({ midiFile.getFullPathName() }, false, &dragMidiButton, [this] {
                dragMidiButton.hasInitiatedDrag = false;
            });
        }
    };
    dragMidiButton.onClick = [this] {
        audioProcessor.exportMidiFile(audioProcessor.currentTab);
    };
    addAndMakeVisible(&dragMidiButton);

    dragAudioButton.setTooltip("Drag Audio: Drag current slice audio clip directly into your DAW project.");
    dragAudioButton.onStartDrag = [this] {
        int selectedSlice = waveformComponent.getSelectedSliceIndex() >= 0 ? waveformComponent.getSelectedSliceIndex() : 0;
        juce::File wavFile = audioProcessor.exportSliceWavFile(audioProcessor.currentTab, selectedSlice);
        if (wavFile.existsAsFile()) {
            juce::DragAndDropContainer::performExternalDragDropOfFiles({ wavFile.getFullPathName() }, false, &dragAudioButton, [this] {
                dragAudioButton.hasInitiatedDrag = false;
            });
        }
    };
    dragAudioButton.onClick = [this] {
        int selectedSlice = waveformComponent.getSelectedSliceIndex() >= 0 ? waveformComponent.getSelectedSliceIndex() : 0;
        audioProcessor.exportSliceWavFile(audioProcessor.currentTab, selectedSlice);
    };
    addAndMakeVisible(&dragAudioButton);

    // Playback Mode Dropdown (Mono Choke vs Play Through)
    playbackModeLabel.setText("Play Mode:", juce::dontSendNotification);
    playbackModeLabel.setFont(CustomLookAndFeel::getRobotoFont(11.0f, true));
    addAndMakeVisible(&playbackModeLabel);

    playbackModeDropdown.addItem("Mono Choke", 1);
    playbackModeDropdown.addItem("Play Through", 2);
    playbackModeDropdown.setSelectedId(audioProcessor.playThroughMode.load() ? 2 : 1, juce::dontSendNotification);
    playbackModeDropdown.setTooltip("Play Mode: Mono Choke (cuts off previous slice on new trigger) or Play Through.");
    playbackModeDropdown.onChange = [this] {
        audioProcessor.playThroughMode = (playbackModeDropdown.getSelectedId() == 2);
    };
    addAndMakeVisible(&playbackModeDropdown);

    // Key Mapping Mode Button
    keyMapButton.setTooltip("Key Map: Switch MIDI note mapping between White Keys (natural keys) and Chromatic (all keys).");
    keyMapButton.isWhiteKeys = [this]() {
        return audioProcessor.whiteKeysOnly.load();
    };
    keyMapButton.onToggle = [this]() {
        audioProcessor.whiteKeysOnly = !audioProcessor.whiteKeysOnly.load();
        keyMapButton.repaint();
    };
    addAndMakeVisible(&keyMapButton);

    sliceTypeLabel.setText("Slice Type", juce::dontSendNotification);
    sliceTypeLabel.setFont(CustomLookAndFeel::getRobotoFont(11.0f, true));
    addAndMakeVisible(&sliceTypeLabel);

    keyMapLabel.setText("Key Map type:", juce::dontSendNotification);
    keyMapLabel.setFont(CustomLookAndFeel::getRobotoFont(11.0f, true));
    addAndMakeVisible(&keyMapLabel);

    recordLabel.setText("Record", juce::dontSendNotification);
    recordLabel.setFont(CustomLookAndFeel::getRobotoFont(11.0f, true));
    recordLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(&recordLabel);

    playingLabel.setText("Playing", juce::dontSendNotification);
    playingLabel.setFont(CustomLookAndFeel::getRobotoFont(12.0f, true));
    playingLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    addAndMakeVisible(&playingLabel);

    // GUI Scaling Dropdown
    guiScaleDropdown.addItem("50%", 1);
    guiScaleDropdown.addItem("75%", 2);
    guiScaleDropdown.addItem("100%", 3);
    guiScaleDropdown.addItem("125%", 4);
    guiScaleDropdown.addItem("150%", 5);
    guiScaleDropdown.setSelectedId(3);
    guiScaleDropdown.setTooltip("GUI Scale: Scale the entire plugin user interface (50% to 150%).");
    guiScaleDropdown.onChange = [this] {
        float s = 1.0f;
        switch(guiScaleDropdown.getSelectedId()) {
            case 1: s = 0.5f; break;
            case 2: s = 0.75f; break;
            case 3: s = 1.0f; break;
            case 4: s = 1.25f; break;
            case 5: s = 1.50f; break;
        }
        setTransform(juce::AffineTransform::scale(s));
    };
    addAndMakeVisible(&guiScaleDropdown);

    // Helper Popups Toggle Checkbox
    helperToggleBtn.setButtonText("Helper");
    helperToggleBtn.setToggleState(true, juce::dontSendNotification);
    helperToggleBtn.setTooltip("Helper Popups: Toggle interactive hover pop-up hints on or off.");
    helperToggleBtn.onClick = [this] {
        if (helperToggleBtn.getToggleState()) {
            tooltipWindow.setMillisecondsBeforeTipAppears(350);
        } else {
            tooltipWindow.setMillisecondsBeforeTipAppears(10000000);
        }
    };
    addAndMakeVisible(&helperToggleBtn);

    // Pitch Bend Range Dropdown
    pbRangeDropdown.addItem(juce::String::fromUTF8("Range: \xc2\xb1 2"), 1);
    pbRangeDropdown.addItem(juce::String::fromUTF8("Range: \xc2\xb1 7"), 2);
    pbRangeDropdown.addItem(juce::String::fromUTF8("Range: \xc2\xb1 12"), 3);
    pbRangeDropdown.addItem(juce::String::fromUTF8("Range: \xc2\xb1 24"), 4);
    pbRangeDropdown.setSelectedId(1);
    pbRangeDropdown.setTooltip("Pitch Bend Range: Semitone range (±1 to ±24) for MIDI pitch bend wheel.");
    pbRangeDropdown.onChange = [this] {
        float semi = 2.0f;
        switch(pbRangeDropdown.getSelectedId()) {
            case 1: semi = 2.0f; break;
            case 2: semi = 7.0f; break;
            case 3: semi = 12.0f; break;
            case 4: semi = 24.0f; break;
        }
        audioProcessor.pitchBendRangeSemi = semi;
    };
    addAndMakeVisible(&pbRangeDropdown);

    // Root Key Mapping Dropdown
    rootKeyDropdown.addItem("Root: C0", 1);
    rootKeyDropdown.addItem("Root: C1", 2);
    rootKeyDropdown.addItem("Root: C2", 3);
    rootKeyDropdown.addItem("Root: C3", 4);
    rootKeyDropdown.addItem("Root: C4", 5);
    rootKeyDropdown.setSelectedId(4); // Default C3 (48)
    rootKeyDropdown.setTooltip("Root Key: Base MIDI root note (C0 to C4) for mapped slice playback.");
    rootKeyDropdown.onChange = [this] {
        int root = 48;
        switch(rootKeyDropdown.getSelectedId()) {
            case 1: root = 12; break;
            case 2: root = 24; break;
            case 3: root = 36; break;
            case 4: root = 48; break;
            case 5: root = 60; break;
        }
        audioProcessor.rootNote = root;
    };
    addAndMakeVisible(&rootKeyDropdown);

    // Master Volume Control
    masterVolSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    masterVolSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 15);
    masterVolSlider.setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    masterVolSlider.setColour(juce::Slider::textBoxBackgroundColourId, juce::Colours::transparentBlack);
    masterVolSlider.setRange(0.0, 2.0, 0.01);
    masterVolSlider.setValue(1.0);
    masterVolSlider.setDoubleClickReturnValue(true, 1.0);
    masterVolSlider.setTooltip("Master Volume: Master output gain level (-inf to +6dB).");
    masterVolSlider.onValueChange = [this] {
        audioProcessor.masterVolume = (float)masterVolSlider.getValue();
    };
    addAndMakeVisible(&masterVolSlider);

    masterVolLabel.setText("Master", juce::dontSendNotification);
    masterVolLabel.setFont(CustomLookAndFeel::getRobotoFont(11.0f, true));
    masterVolLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(&masterVolLabel);

    addAndMakeVisible(&waveformComponent);
    addAndMakeVisible(&sliceControlsComponent);
    sliceControlsComponent.setDropdowns(&pbRangeDropdown, &rootKeyDropdown);
    sliceControlsComponent.setLayoutConfig(&layoutConfig);
    waveformComponent.onSliceSelected = [this](int idx) {
        sliceControlsComponent.setSelectedSliceIndex(idx);
    };

    layoutConfig.loadFromFile();

#if ENABLE_LAYOUT_INSPECTOR
    inspectorToggleBtn.setButtonText("⚙ Tweak");
    inspectorToggleBtn.onClick = [this]() {
        layoutInspector.setVisible(!layoutInspector.isVisible());
        if (layoutInspector.isVisible()) layoutInspector.toFront(true);
    };
    addAndMakeVisible(&inspectorToggleBtn);

    layoutInspector.onLayoutChanged = [this]() {
        resized();
        repaint();
        sliceControlsComponent.resized();
        sliceControlsComponent.repaint();
    };
    addChildComponent(&layoutInspector);
#endif

    bgTexture = juce::ImageCache::getFromMemory(BinaryData::bg_texture_jpg, BinaryData::bg_texture_jpgSize);
    logoImage = juce::ImageCache::getFromMemory(BinaryData::logo_png, BinaryData::logo_pngSize);
    screwImage = juce::ImageCache::getFromMemory(BinaryData::skrew_png, BinaryData::skrew_pngSize);

    setSize (layoutConfig.windowW, layoutConfig.windowH);
    startTimerHz(25);
}

ChopSampAudioProcessorEditor::~ChopSampAudioProcessorEditor()
{
    setLookAndFeel(nullptr);
    sliceControlsComponent.setLookAndFeel(nullptr);
    waveformComponent.setLookAndFeel(nullptr);
    juce::LookAndFeel::setDefaultLookAndFeel(nullptr);
    stopTimer();
}

void ChopSampAudioProcessorEditor::paint (juce::Graphics& g)
{
    auto theme = getTheme(audioProcessor.currentTheme.load());

    if (bgTexture.isValid())
    {
        g.drawImage(bgTexture, getLocalBounds().toFloat(), juce::RectanglePlacement::fillDestination);
        g.fillAll(theme.bgColour.withAlpha(0.25f)); // Blend subtle theme tint for dark chassis warmth
    }
    else
    {
        g.fillAll (theme.bgColour);
    }

    masterVolLabel.setColour(juce::Label::textColourId, theme.textColour);
    sliceTypeLabel.setColour(juce::Label::textColourId, theme.textColour);
    keyMapLabel.setColour(juce::Label::textColourId, theme.textColour);
    playbackModeLabel.setColour(juce::Label::textColourId, theme.textColour);
    recordLabel.setColour(juce::Label::textColourId, theme.textColour);

    // Draw Corner Metallic Screws using authentic 3D screw asset
    auto drawScrew = [this, &g](float cx, float cy) {
        if (screwImage.isValid()) {
            float size = 20.0f;
            g.drawImageWithin(screwImage, (int)(cx - size * 0.5f), (int)(cy - size * 0.5f), (int)size, (int)size,
                              juce::RectanglePlacement::centred | juce::RectanglePlacement::onlyReduceInSize);
        } else {
            g.setColour(juce::Colour::fromRGB(60, 62, 68));
            g.fillEllipse(cx - 7.0f, cy - 7.0f, 14.0f, 14.0f);
            g.setColour(juce::Colour::fromRGB(20, 22, 26));
            g.drawEllipse(cx - 7.0f, cy - 7.0f, 14.0f, 14.0f, 1.2f);
            g.setColour(juce::Colour::fromRGB(30, 32, 36));
            g.drawLine(cx - 4.0f, cy - 4.0f, cx + 4.0f, cy + 4.0f, 2.0f);
        }
    };
    drawScrew(14.0f, 14.0f);
    drawScrew((float)getWidth() - 14.0f, 14.0f);
    drawScrew(14.0f, (float)getHeight() - 14.0f);
    drawScrew((float)getWidth() - 14.0f, (float)getHeight() - 14.0f);

    // Check if any voice is active
    bool isAnyVoiceActive = false;
    for (const auto& v : audioProcessor.voices) {
        if (v.isActive && !v.isChoked) { isAnyVoiceActive = true; break; }
    }
    
    // Draw LED Lamp in Header (Top Left) — DEFAULT BLACK/OFF, ONLY LIGHTS UP GREEN WHEN PLAYED
    auto ledArea = juce::Rectangle<float>(48.0f, 9.0f, 10.0f, 10.0f);
    if (isAnyVoiceActive) {
        // Bright glowing green when active
        g.setColour(juce::Colour::fromRGB(46, 255, 120));
        g.fillEllipse(ledArea);
        g.setColour(juce::Colour::fromRGB(180, 255, 200));
        g.drawEllipse(ledArea, 1.5f);
    } else {
        // Default off (dark black/dim)
        g.setColour(juce::Colour::fromRGB(14, 16, 14));
        g.fillEllipse(ledArea);
        g.setColour(juce::Colour::fromRGB(32, 36, 32));
        g.drawEllipse(ledArea, 1.0f);
    }

    // Draw Master VU Peak Meter Bar
    auto meterArea = layoutConfig.masterVuMeter.toRect().toFloat();
    if (meterArea.getWidth() > 0) {
        g.setColour(juce::Colour::fromRGB(0, 0, 0));
        g.fillRect(meterArea);
        
        float level = juce::jmax(audioProcessor.outputLevelL.load(), audioProcessor.outputLevelR.load());
        level = juce::jlimit(0.0f, 1.0f, level);
        
        if (level > 0.001f) {
            auto activeMeter = meterArea.removeFromBottom(meterArea.getHeight() * level);
            g.setColour(level > 0.85f ? juce::Colours::red : (level > 0.65f ? juce::Colours::yellow : juce::Colour::fromRGB(46, 204, 113)));
            g.fillRect(activeMeter);
        }
    }

#if ENABLE_LAYOUT_INSPECTOR
    // Highlight Selected Element & Alignment Guidelines if in Inspector mode
    if (layoutInspector.isVisible()) {
        auto* elem = layoutConfig.getElementById(layoutConfig.selectedElementId);
        int elemX = elem ? elem->x : 0;
        int elemY = elem ? elem->y : 0;
        int elemW = elem ? elem->w : 0;
        int elemH = elem ? elem->h : 0;

        if (layoutConfig.selectedElementId >= 2 && layoutConfig.selectedElementId <= 20) {
            // Adjust relative coords from SliceControlsComponent for global guidelines
            elemX += 18;
            elemY += 280;
        }

        if (elem && (layoutConfig.selectedElementId == 1 || layoutConfig.selectedElementId >= 21)) {
            g.setColour(juce::Colour::fromRGB(240, 115, 125));
            g.drawRect(elem->toRect().expanded(2), 1.5f);
        }

        // Draw Margin Lines & Measurements
        if (layoutConfig.showGuides) {
            float dashes[] = { 4.0f, 4.0f };

            // 1. Draggable Horizontal Guideline (Y)
            g.setColour(juce::Colour::fromRGB(0, 230, 255).withAlpha(0.65f));
            g.drawDashedLine(juce::Line<float>(0.0f, (float)layoutConfig.guideLineH, (float)getWidth(), (float)layoutConfig.guideLineH), dashes, 2, 1.2f);
            g.setColour(juce::Colour::fromRGB(0, 20, 30));
            g.fillRect(6, layoutConfig.guideLineH - 8, 62, 16);
            g.setColour(juce::Colour::fromRGB(0, 240, 255));
            g.drawRect(6, layoutConfig.guideLineH - 8, 62, 16);
            g.setFont(CustomLookAndFeel::getRobotoFont(10.0f, true));
            g.drawText("Y: " + juce::String(layoutConfig.guideLineH) + "px", 6, layoutConfig.guideLineH - 8, 62, 16, juce::Justification::centred);

            // 2. Draggable Vertical Guideline (X)
            g.drawDashedLine(juce::Line<float>((float)layoutConfig.guideLineV, 0.0f, (float)layoutConfig.guideLineV, (float)getHeight()), dashes, 2, 1.2f);
            g.setColour(juce::Colour::fromRGB(0, 20, 30));
            g.fillRect(layoutConfig.guideLineV - 32, 6, 64, 16);
            g.setColour(juce::Colour::fromRGB(0, 240, 255));
            g.drawRect(layoutConfig.guideLineV - 32, 6, 64, 16);
            g.drawText("X: " + juce::String(layoutConfig.guideLineV) + "px", layoutConfig.guideLineV - 32, 6, 64, 16, juce::Justification::centred);

            // 3. Selected Element Measurement Badge & Crosshair
            if (elem) {
                g.setColour(juce::Colour::fromRGB(240, 115, 125).withAlpha(0.40f));
                g.drawDashedLine(juce::Line<float>(0.0f, (float)elemY, (float)getWidth(), (float)elemY), dashes, 2, 1.0f);
                g.drawDashedLine(juce::Line<float>((float)elemX, 0.0f, (float)elemX, (float)getHeight()), dashes, 2, 1.0f);

                g.setColour(juce::Colour::fromRGB(30, 10, 15));
                g.fillRect(elemX, elemY - 18, 120, 16);
                g.setColour(juce::Colour::fromRGB(240, 115, 125));
                g.drawRect(elemX, elemY - 18, 120, 16);
                g.drawText("X:" + juce::String(elemX) + " Y:" + juce::String(elemY) + " (" + juce::String(elemW) + "x" + juce::String(elemH) + ")",
                           elemX, elemY - 18, 120, 16, juce::Justification::centred);
            }
        }
    }
#endif
}

void ChopSampAudioProcessorEditor::resized()
{
    const int W = getWidth();
    const int H = getHeight();

    // Top Bar: GUI Scale Dropdown & Helper Checkbox
    guiScaleDropdown.setBounds(layoutConfig.guiScale.toRect());
    helperToggleBtn.setBounds(layoutConfig.guiScale.x - 70, layoutConfig.guiScale.y, 66, 18);

    // Header Playing label
    playingLabel.setBounds(64, 4, 60, 20);

    // Waveform + Logo LCD box
    waveformComponent.setBounds(18, 30, W - 36, 230);

    // Left+Centre Slice Controls
    sliceControlsComponent.setBounds(18, 280, 640, H - 280 - 14);

    // Right Panel Elements (placed from individual ElementRects)
    sliceTypeLabel.setBounds(layoutConfig.sliceModeDropdown.x, layoutConfig.sliceModeDropdown.y - 16, 140, 14);
    sliceModeDropdown.setBounds(layoutConfig.sliceModeDropdown.toRect());
    if (gridDropdown.isVisible())             gridDropdown.setBounds(layoutConfig.gridDropdown.toRect());
    else if (transientSensSlider.isVisible()) transientSensSlider.setBounds(layoutConfig.gridDropdown.toRect());
    else if (randomCountSlider.isVisible())   randomCountSlider.setBounds(layoutConfig.gridDropdown.toRect());

    keyMapLabel.setBounds(layoutConfig.keyMapBtn.x, layoutConfig.keyMapBtn.y - 16, 140, 14);
    keyMapButton.setBounds(layoutConfig.keyMapBtn.toRect());
    executeSliceBtn.setBounds(layoutConfig.sliceBtn.toRect());

    playbackModeLabel.setBounds(layoutConfig.playModeDropdown.x, layoutConfig.playModeDropdown.y - 16, 140, 14);
    playbackModeDropdown.setBounds(layoutConfig.playModeDropdown.toRect());

    dragMidiButton.setBounds(layoutConfig.dragMidiBtn.toRect());
    dragAudioButton.setBounds(layoutConfig.dragAudioBtn.toRect());

    recordLabel.setBounds(layoutConfig.recordBtn.x, layoutConfig.recordBtn.y - 14, 58, 14);
    recordButton.setBounds(layoutConfig.recordBtn.toRect());

    masterVolLabel.setBounds(layoutConfig.masterVolKnob.x, layoutConfig.masterVolKnob.y - 14, 62, 14);
    masterVolSlider.setBounds(layoutConfig.masterVolKnob.toRect());

#if ENABLE_LAYOUT_INSPECTOR
    inspectorToggleBtn.setBounds(135, 6, 64, 20);
    // Only set initial default inspector position if uninitialized (preserves user dragged position!)
    if (layoutInspector.getWidth() == 0 || layoutInspector.getHeight() == 0) {
        layoutInspector.setBounds(W - 325, 28, 315, 490);
    }
#endif
}

#if ENABLE_LAYOUT_INSPECTOR
bool ChopSampAudioProcessorEditor::keyPressed (const juce::KeyPress& key)
{
    if (key.getKeyCode() == juce::KeyPress::F12Key || key.getKeyCode() == juce::KeyPress::tabKey)
    {
        layoutInspector.setVisible(!layoutInspector.isVisible());
        if (layoutInspector.isVisible()) layoutInspector.toFront(true);
        return true;
    }
    return false;
}
#endif

void ChopSampAudioProcessorEditor::timerCallback()
{
    bool isAnyVoiceActive = false;
    for (const auto& v : audioProcessor.voices) {
        if (v.isActive && !v.isChoked) { isAnyVoiceActive = true; break; }
    }
    float lvlL = audioProcessor.outputLevelL.load();
    float lvlR = audioProcessor.outputLevelR.load();
    bool isRec = audioProcessor.isRecording.load();

    if (recordButton.getToggleState() != isRec) {
        recordButton.setToggleState(isRec, juce::dontSendNotification);
        waveformComponent.repaint();
    }
    
    if (isAnyVoiceActive || isRec || lvlL > 0.005f || lvlR > 0.005f) {
        repaint();
        if (isRec) waveformComponent.repaint();
    }
}

bool ChopSampAudioProcessorEditor::isInterestedInFileDrag (const juce::StringArray& files)
{
    return true;
}

void ChopSampAudioProcessorEditor::filesDropped (const juce::StringArray& files, int x, int y)
{
    if (files.size() > 0)
    {
        juce::String cleanPath = files[0].unquoted().trim();
        if (cleanPath.endsWithIgnoreCase(".reapeaks")) {
            cleanPath = cleanPath.substring(0, cleanPath.length() - 9);
        }
        audioProcessor.loadFile(cleanPath, audioProcessor.currentTab);
        waveformComponent.repaint();
    }
}
