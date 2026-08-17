#pragma once

#include <JuceHeader.h>

const int MAX_SAMPLE_TABS = 20;

struct UITheme {
    juce::Colour bgColour;
    juce::Colour panelBgColour;
    juce::Colour panelBorderColour;
    juce::Colour accentColour;
    juce::Colour textColour;
    juce::Colour buttonBgColour;
    juce::Colour waveformColour;
};

inline UITheme getTheme(int themeIndex) {
    switch (themeIndex) {
        case 1: // MPC Retro Cream
            return {
                juce::Colour::fromRGB(227, 222, 195),
                juce::Colour::fromRGB(212, 206, 178),
                juce::Colour::fromRGB(170, 160, 130),
                juce::Colour::fromRGB(229, 107, 56),
                juce::Colour::fromRGB(40, 35, 30),
                juce::Colour::fromRGB(195, 188, 160),
                juce::Colour::fromRGB(229, 107, 56)
            };
        case 2: // Cyberpunk Neon
            return {
                juce::Colour::fromRGB(18, 14, 22),
                juce::Colour::fromRGB(28, 20, 36),
                juce::Colour::fromRGB(255, 0, 127),
                juce::Colour::fromRGB(0, 240, 255),
                juce::Colour::fromRGB(255, 255, 255),
                juce::Colour::fromRGB(45, 30, 60),
                juce::Colour::fromRGB(0, 240, 255)
            };
        case 3: // Minimalist Flat
            return {
                juce::Colour::fromRGB(240, 242, 245),
                juce::Colour::fromRGB(255, 255, 255),
                juce::Colour::fromRGB(210, 215, 220),
                juce::Colour::fromRGB(37, 99, 235),
                juce::Colour::fromRGB(26, 29, 32),
                juce::Colour::fromRGB(230, 235, 242),
                juce::Colour::fromRGB(37, 99, 235)
            };
        case 0: // Digitakt Dark
        default:
            return {
                juce::Colour::fromRGB(30, 32, 36),
                juce::Colour::fromRGB(20, 22, 26),
                juce::Colour::fromRGB(60, 65, 75),
                juce::Colour::fromRGB(0, 255, 180),
                juce::Colour::fromRGB(255, 255, 255),
                juce::Colour::fromRGB(45, 50, 60),
                juce::Colour::fromRGB(45, 125, 246)
            };
    }
}

struct SliceParams {
    float volume = 1.0f;
    float pan = 0.0f;
    float pitchSemi = 0.0f;
    bool reverse = false;
    float startTrimMs = 0.0f;
    float endTrimMs = 0.0f;
    float attackMs = 10.0f;
    float decayMs = 100.0f;
    float sustainLevel = 1.0f;
    float releaseMs = 100.0f;
    float crossfadeMs = 20.0f;
    float filterCutoff = 20000.0f;
    float lpfCutoff = 20000.0f;
    float hpfCutoff = 20.0f;
    float delayMix = 0.0f;
    float reverbMix = 0.0f;
    juce::String sliceName = "";
    juce::uint32 colorARGB = 0;
};

struct SliceMarker {
    int sampleIndex;
    bool isSelected = false;
    SliceParams params;
};

struct SampleData
{
    juce::AudioBuffer<float> buffer;
    juce::String name = "Empty";
    bool isLoaded = false;
    double sampleRate = 44100.0;
    int bitDepth = 16;
    juce::String format = ".wav";
    std::vector<SliceMarker> markers;
};

struct ChopSampVoice {
    bool isActive = false;
    bool isChoked = false;
    int note = -1;
    int tabIndex = -1;
    int sliceIndex = -1;
    double currentPosition = 0.0;
    double pitchRatio = 1.0;
    
    juce::ADSR adsr;
    juce::IIRFilter filterL, filterR;
    juce::IIRFilter lpfL, lpfR;
    juce::IIRFilter hpfL, hpfR;
    float lastLpf = -1.0f;
    float lastHpf = -1.0f;

    float chokeStartGain = 1.0f;
    float chokeTotalSamples = 1.0f;
    float chokeElapsedSamples = 0.0f;

    float fadeInTotalSamples = 0.0f;
    float fadeInElapsedSamples = 0.0f;

    float currentEffectiveGain = 0.0f;
};

class ChopSampAudioProcessor  : public juce::AudioProcessor
{
public:
    ChopSampAudioProcessor();
    ~ChopSampAudioProcessor() override;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

    juce::AudioFormatManager formatManager;
    juce::CriticalSection sampleLock;
    SampleData samples[MAX_SAMPLE_TABS];
    std::atomic<int> currentTab { 0 };

    juce::MidiKeyboardState keyboardState;

    std::atomic<bool> isRecording { false };
    int recordingSampleIndex = 0;
    juce::AudioBuffer<float> tempRecordBuffer;
    
    ChopSampVoice voices[16]; // 16 voice polyphony

    void loadFile(const juce::String& path, int tabIndex);
    void stopRecording();

    int findNearestZeroCrossing(int tabIndex, int sampleIndex) const;

    void sliceByGrid(int tabIndex, double noteValue); // noteValue: 0.5 (1/2), 0.25 (1/4), etc.
    void sliceByTransients(int tabIndex, float sensitivity);
    void sliceRandom(int tabIndex, int numSlices);
    void clearSlices(int tabIndex);

    juce::File exportMidiFile(int tabIndex);
    juce::File exportSliceWavFile(int tabIndex, int sliceIndex);

    std::atomic<bool> playThroughMode { false };
    std::atomic<float> masterVolume { 1.0f };
    std::atomic<float> outputLevelL { 0.0f };
    std::atomic<float> outputLevelR { 0.0f };
    std::atomic<bool> whiteKeysOnly { true };
    std::atomic<float> currentPitchBendSemi { 0.0f };
    std::atomic<float> pitchBendRangeSemi { 2.0f };
    std::atomic<float> modWheelCutoffHz { 20000.0f };
    std::atomic<int> rootNote { 48 }; // Default C3 = 48 (C0=12, C1=24, C2=36, C3=48, C4=60)
    std::atomic<int> currentTheme { 0 }; // 0: Digitakt Dark, 1: MPC Retro Cream, 2: Cyberpunk Neon, 3: Minimalist Flat
    double currentBpm = 120.0;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ChopSampAudioProcessor)
};
