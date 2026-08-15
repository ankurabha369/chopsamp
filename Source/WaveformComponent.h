#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

class WaveformComponent : public juce::Component,
                          public juce::SettableTooltipClient
{
public:
    WaveformComponent(ChopSampAudioProcessor& p);
    ~WaveformComponent() override;

    void paint(juce::Graphics&) override;
    void resized() override;
    
    void mouseDown(const juce::MouseEvent& e) override;
    void mouseDrag(const juce::MouseEvent& e) override;
    void mouseUp(const juce::MouseEvent& e) override;
    void mouseDoubleClick(const juce::MouseEvent& e) override;
    void mouseMove(const juce::MouseEvent& e) override;
    void mouseWheelMove(const juce::MouseEvent& e, const juce::MouseWheelDetails& wheel) override;

    std::function<void(int)> onSliceSelected;
    int getSelectedSliceIndex() const { return selectedSliceIndex; }
    void setSelectedSliceIndex(int index) { selectedSliceIndex = index; repaint(); }

private:
    ChopSampAudioProcessor& audioProcessor;
    int selectedSliceIndex = -1;
    int draggingMarkerIndex = -1;

    double zoomRatio = 1.0;
    double scrollOffset = 0.0;
    bool isPanning = false;
    juce::Point<int> lastPanMousePos;
    juce::Image logoImage;

    float sampleToX(int sampleIndex, int numSamples, float width, float startX) const;
    int xToSample(float x, int numSamples, float width, float startX) const;
    juce::Rectangle<int> getScreenBounds() const;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(WaveformComponent)
};
