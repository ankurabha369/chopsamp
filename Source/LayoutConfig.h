#pragma once
#include <JuceHeader.h>

struct ElementRect {
    int x = 0;
    int y = 0;
    int w = 0;
    int h = 0;

    juce::Rectangle<int> toRect() const { return { x, y, w, h }; }
    void setFrom(const juce::Rectangle<int>& r) { x = r.getX(); y = r.getY(); w = r.getWidth(); h = r.getHeight(); }
};

struct LayoutConfig {
    int windowW = 1000;
    int windowH = 545;
    int selectedElementId = 1;

    // Elements
    ElementRect guiScale        { 908, 6, 58, 18 };
    
    // Left Group (Slice Controls relative or absolute)
    ElementRect xfadeKnob       { 0, 0, 54, 62 };
    ElementRect startTrimKnob   { 54, 0, 54, 62 };
    ElementRect endTrimKnob     { 108, 0, 54, 62 };
    ElementRect reverseBtn      { 183, 0, 44, 62 };
    ElementRect colorDropdown   { 238, 0, 48, 58 };
    ElementRect volKnob         { 0, 85, 57, 62 };
    ElementRect panKnob         { 57, 85, 57, 62 };
    ElementRect pitchKnob       { 114, 85, 57, 62 };
    ElementRect hpfKnob         { 171, 85, 57, 62 };
    ElementRect lpfKnob         { 228, 85, 57, 62 };
    ElementRect tabGrid         { 0, 156, 285, 78 };

    // Center Group
    ElementRect adsrGraph       { 345, 0, 265, 82 };
    ElementRect applyToAllBtn   { 376, 87, 200, 26 };
    ElementRect attackKnob      { 345, 116, 66, 62 };
    ElementRect decayKnob       { 411, 116, 66, 62 };
    ElementRect sustainKnob     { 477, 116, 66, 62 };
    ElementRect releaseKnob     { 543, 116, 66, 62 };
    ElementRect pbRangeDropdown { 345, 209, 95, 24 };
    ElementRect rootKeyDropdown { 527, 209, 85, 24 };

    // Right Panel
    ElementRect sliceModeDropdown { 676, 296, 140, 24 };
    ElementRect gridDropdown      { 826, 296, 140, 24 };
    ElementRect keyMapBtn         { 676, 350, 140, 28 };
    ElementRect sliceBtn          { 893, 344, 76, 40 };
    ElementRect playModeDropdown  { 676, 406, 130, 24 };
    ElementRect dragMidiBtn       { 811, 404, 88, 28 };
    ElementRect dragAudioBtn      { 886, 404, 88, 28 };
    ElementRect recordBtn         { 716, 454, 58, 58 };
    ElementRect masterVolKnob     { 864, 454, 62, 74 };
    ElementRect masterVuMeter     { 947, 456, 7, 44 };

    static juce::File getConfigFile() {
        return juce::File::getSpecialLocation(juce::File::userDocumentsDirectory)
               .getChildFile("ChopSamp")
               .getChildFile("layout.json");
    }

    bool showGuides = true;
    int guideLineH = 280; // Draggable Horizontal Margin Line (Y)
    int guideLineV = 676; // Draggable Vertical Margin Line (X)

    void resetElementToDefault(int id) {
        LayoutConfig def;
        auto* cur = getElementById(id);
        auto* d = def.getElementById(id);
        if (cur && d) {
            *cur = *d;
        }
    }

    ElementRect* getElementById(int id) {
        switch (id) {
            case 1:  return &guiScale;
            case 2:  return &xfadeKnob;
            case 3:  return &startTrimKnob;
            case 4:  return &endTrimKnob;
            case 5:  return &reverseBtn;
            case 6:  return &colorDropdown;
            case 7:  return &volKnob;
            case 8:  return &panKnob;
            case 9:  return &pitchKnob;
            case 10: return &hpfKnob;
            case 11: return &lpfKnob;
            case 12: return &tabGrid;
            case 13: return &adsrGraph;
            case 14: return &applyToAllBtn;
            case 15: return &attackKnob;
            case 16: return &decayKnob;
            case 17: return &sustainKnob;
            case 18: return &releaseKnob;
            case 19: return &pbRangeDropdown;
            case 20: return &rootKeyDropdown;
            case 21: return &sliceModeDropdown;
            case 22: return &gridDropdown;
            case 23: return &keyMapBtn;
            case 24: return &sliceBtn;
            case 25: return &playModeDropdown;
            case 26: return &dragMidiBtn;
            case 27: return &dragAudioBtn;
            case 28: return &recordBtn;
            case 29: return &masterVolKnob;
            case 30: return &masterVuMeter;
            default: return nullptr;
        }
    }

    static juce::String getElementName(int id) {
        switch (id) {
            case 1:  return "1. GUI Scale Dropdown";
            case 2:  return "2. XFade Knob";
            case 3:  return "3. Start Trim Knob";
            case 4:  return "4. End Trim Knob";
            case 5:  return "5. Reverse Button";
            case 6:  return "6. Color Dropdown";
            case 7:  return "7. Vol Knob";
            case 8:  return "8. Pan Knob";
            case 9:  return "9. Pitch Knob";
            case 10: return "10. HPF Knob";
            case 11: return "11. LPF Knob";
            case 12: return "12. 20 Sample Tab Grid";
            case 13: return "13. ADSR Graph Box";
            case 14: return "14. Apply to All Button";
            case 15: return "15. Attack Knob";
            case 16: return "16. Decay Knob";
            case 17: return "17. Sustain Knob";
            case 18: return "18. Release Knob";
            case 19: return "19. PB Range Dropdown";
            case 20: return "20. Root Key Dropdown";
            case 21: return "21. Slice Mode Dropdown";
            case 22: return "22. Grid / Sens Dropdown";
            case 23: return "23. Key Map Button";
            case 24: return "24. Slice Exec Button";
            case 25: return "25. Play Mode Dropdown";
            case 26: return "26. Drag MIDI Button";
            case 27: return "27. Drag Audio Button";
            case 28: return "28. Record Button";
            case 29: return "29. Master Vol Knob";
            case 30: return "30. Master VU Meter";
            default: return "";
        }
    }

    void loadFromFile() {
        auto file = getConfigFile();
        if (!file.existsAsFile()) return;

        auto json = juce::JSON::parse(file);
        if (!json.isObject()) return;

        auto* obj = json.getDynamicObject();
        if (!obj) return;

        auto readRect = [obj](const juce::String& prefix, ElementRect& r) {
            if (obj->hasProperty(prefix + "_x")) r.x = (int)obj->getProperty(prefix + "_x");
            if (obj->hasProperty(prefix + "_y")) r.y = (int)obj->getProperty(prefix + "_y");
            if (obj->hasProperty(prefix + "_w")) r.w = (int)obj->getProperty(prefix + "_w");
            if (obj->hasProperty(prefix + "_h")) r.h = (int)obj->getProperty(prefix + "_h");
        };

        if (obj->hasProperty("windowW")) windowW = (int)obj->getProperty("windowW");
        if (obj->hasProperty("windowH")) windowH = (int)obj->getProperty("windowH");

        readRect("guiScale", guiScale);
        readRect("xfade", xfadeKnob);
        readRect("startTrim", startTrimKnob);
        readRect("endTrim", endTrimKnob);
        readRect("reverse", reverseBtn);
        readRect("color", colorDropdown);
        readRect("vol", volKnob);
        readRect("pan", panKnob);
        readRect("pitch", pitchKnob);
        readRect("hpf", hpfKnob);
        readRect("lpf", lpfKnob);
        readRect("tabGrid", tabGrid);
        readRect("adsrGraph", adsrGraph);
        readRect("applyToAll", applyToAllBtn);
        readRect("attack", attackKnob);
        readRect("decay", decayKnob);
        readRect("sustain", sustainKnob);
        readRect("release", releaseKnob);
        readRect("pbRange", pbRangeDropdown);
        readRect("rootKey", rootKeyDropdown);
        readRect("sliceMode", sliceModeDropdown);
        readRect("grid", gridDropdown);
        readRect("keyMap", keyMapBtn);
        readRect("sliceBtn", sliceBtn);
        readRect("playMode", playModeDropdown);
        readRect("dragMidi", dragMidiBtn);
        readRect("dragAudio", dragAudioBtn);
        readRect("record", recordBtn);
        readRect("masterVol", masterVolKnob);
        readRect("masterVu", masterVuMeter);
    }

    void saveToFile() const {
        auto file = getConfigFile();
        file.getParentDirectory().createDirectory();

        auto* obj = new juce::DynamicObject();
        obj->setProperty("windowW", windowW);
        obj->setProperty("windowH", windowH);

        auto writeRect = [obj](const juce::String& prefix, const ElementRect& r) {
            obj->setProperty(prefix + "_x", r.x);
            obj->setProperty(prefix + "_y", r.y);
            obj->setProperty(prefix + "_w", r.w);
            obj->setProperty(prefix + "_h", r.h);
        };

        writeRect("guiScale", guiScale);
        writeRect("xfade", xfadeKnob);
        writeRect("startTrim", startTrimKnob);
        writeRect("endTrim", endTrimKnob);
        writeRect("reverse", reverseBtn);
        writeRect("color", colorDropdown);
        writeRect("vol", volKnob);
        writeRect("pan", panKnob);
        writeRect("pitch", pitchKnob);
        writeRect("hpf", hpfKnob);
        writeRect("lpf", lpfKnob);
        writeRect("tabGrid", tabGrid);
        writeRect("adsrGraph", adsrGraph);
        writeRect("applyToAll", applyToAllBtn);
        writeRect("attack", attackKnob);
        writeRect("decay", decayKnob);
        writeRect("sustain", sustainKnob);
        writeRect("release", releaseKnob);
        writeRect("pbRange", pbRangeDropdown);
        writeRect("rootKey", rootKeyDropdown);
        writeRect("sliceMode", sliceModeDropdown);
        writeRect("grid", gridDropdown);
        writeRect("keyMap", keyMapBtn);
        writeRect("sliceBtn", sliceBtn);
        writeRect("playMode", playModeDropdown);
        writeRect("dragMidi", dragMidiBtn);
        writeRect("dragAudio", dragAudioBtn);
        writeRect("record", recordBtn);
        writeRect("masterVol", masterVolKnob);
        writeRect("masterVu", masterVuMeter);

        auto jsonString = juce::JSON::toString(juce::var(obj), true);
        file.replaceWithText(jsonString);
    }
};
