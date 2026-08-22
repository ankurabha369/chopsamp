#include "PluginProcessor.h"
#include "PluginEditor.h"

ChopSampAudioProcessor::ChopSampAudioProcessor()
     : AudioProcessor (BusesProperties()
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                       )
{
    formatManager.registerBasicFormats();
    tempRecordBuffer.setSize(2, 44100 * 60);
}

ChopSampAudioProcessor::~ChopSampAudioProcessor()
{
}

void ChopSampAudioProcessor::loadFile(const juce::String& path, int tabIndex)
{
    if (tabIndex < 0 || tabIndex >= MAX_SAMPLE_TABS) return;

    juce::File file(path);
    if (!file.existsAsFile()) return;

    std::unique_ptr<juce::AudioFormatReader> reader(formatManager.createReaderFor(file));
    if (reader != nullptr)
    {
        int numChans = (int)reader->numChannels;
        int numSamps = (int)reader->lengthInSamples;

        if (numChans > 0 && numSamps > 0)
        {
            // 1. Read into local temporary buffer first off-lock
            juce::AudioBuffer<float> tempBuf(numChans, numSamps);
            reader->read(&tempBuf, 0, numSamps, 0, true, numChans > 1);

            // 2. Initial single marker at start (whole sample unchopped)
            std::vector<SliceMarker> initialMarkers;
            SliceMarker m;
            m.sampleIndex = 0;
            m.isSelected = false;
            initialMarkers.push_back(m);

            // 3. Acquire lock and swap buffer & markers atomically
            const juce::ScopedLock sl (sampleLock);
            for (auto& v : voices) {
                if (v.tabIndex == tabIndex) v.isActive = false;
            }

            samples[tabIndex].isLoaded = false;
            samples[tabIndex].markers = std::move(initialMarkers);
            samples[tabIndex].buffer.setSize(numChans, numSamps);
            for (int ch = 0; ch < numChans; ++ch) {
                samples[tabIndex].buffer.copyFrom(ch, 0, tempBuf, ch, 0, numSamps);
            }

            samples[tabIndex].name = file.getFileNameWithoutExtension();
            samples[tabIndex].filePath = file.getFullPathName();
            samples[tabIndex].sampleRate = reader->sampleRate;
            samples[tabIndex].bitDepth = (int)reader->bitsPerSample > 0 ? (int)reader->bitsPerSample : 16;
            samples[tabIndex].format = file.getFileExtension().toLowerCase();
            if (samples[tabIndex].format.isEmpty()) samples[tabIndex].format = ".wav";
            samples[tabIndex].isLoaded = true;
        }
    }
}

void ChopSampAudioProcessor::stopRecording()
{
    isRecording = false;
    if (recordingSampleIndex > 0)
    {
        int recChans = tempRecordBuffer.getNumChannels();
        int recSamps = recordingSampleIndex;

        if (recChans > 0 && recSamps > 0) {
            juce::AudioBuffer<float> tempBuf(recChans, recSamps);
            for (int ch = 0; ch < recChans; ++ch) {
                tempBuf.copyFrom(ch, 0, tempRecordBuffer, ch, 0, recSamps);
            }

            const juce::ScopedLock sl (sampleLock);
            for (auto& v : voices) {
                if (v.tabIndex == currentTab) v.isActive = false;
            }

            samples[currentTab].isLoaded = false;
            samples[currentTab].markers.clear();
            samples[currentTab].buffer.setSize(recChans, recSamps);
            for (int ch = 0; ch < recChans; ++ch) {
                samples[currentTab].buffer.copyFrom(ch, 0, tempBuf, ch, 0, recSamps);
            }
            samples[currentTab].name = "Recorded Audio";
            samples[currentTab].filePath = "";
            samples[currentTab].sampleRate = getSampleRate() > 0 ? getSampleRate() : 44100.0;
            samples[currentTab].bitDepth = 16;
            samples[currentTab].format = ".wav";
            samples[currentTab].isLoaded = true;
            sliceByGrid(currentTab, 0.25);
        }
        recordingSampleIndex = 0;
    }
}

int ChopSampAudioProcessor::findNearestZeroCrossing(int tabIndex, int sampleIndex) const
{
    if (tabIndex < 0 || tabIndex >= MAX_SAMPLE_TABS || !samples[tabIndex].isLoaded)
        return sampleIndex;

    const auto& buf = samples[tabIndex].buffer;
    int numSamples = buf.getNumSamples();
    int numChannels = buf.getNumChannels();
    if (numSamples <= 0 || numChannels <= 0 || sampleIndex <= 0) return 0;
    if (sampleIndex >= numSamples - 1) return numSamples - 1;

    const float* channelData = buf.getReadPointer(0);
    if (!channelData) return sampleIndex;

    int bestSample = sampleIndex;
    float minVal = std::abs(channelData[sampleIndex]);
    int searchWindow = 250;

    int start = juce::jmax(0, sampleIndex - searchWindow);
    int end = juce::jmin(numSamples - 1, sampleIndex + searchWindow);

    for (int i = start; i < end; ++i)
    {
        if ((channelData[i] * channelData[i + 1]) <= 0.0f)
        {
            return i;
        }
        float absVal = std::abs(channelData[i]);
        if (absVal < minVal)
        {
            minVal = absVal;
            bestSample = i;
        }
    }
    return bestSample;
}

void ChopSampAudioProcessor::sliceByGrid(int tabIndex, double noteValue)
{
    if (tabIndex < 0 || tabIndex >= MAX_SAMPLE_TABS || !samples[tabIndex].isLoaded) return;
    
    int totalSamples = samples[tabIndex].buffer.getNumSamples();
    if (totalSamples <= 0) return;

    double sr = getSampleRate() > 0 ? getSampleRate() : 44100.0;
    double samplesPerBeat = (sr * 60.0) / currentBpm;
    if (samplesPerBeat <= 0) samplesPerBeat = (sr * 60.0) / 120.0;
    double samplesPerSlice = samplesPerBeat * (noteValue * 4.0);
    if (samplesPerSlice <= 10.0) samplesPerSlice = totalSamples / 4.0;
    if (samplesPerSlice <= 10.0) return;

    std::vector<SliceMarker> newMarkers;
    for (double pos = 0; pos < totalSamples; pos += samplesPerSlice)
    {
        int rawPos = (int)pos;
        int zcPos = (rawPos == 0) ? 0 : findNearestZeroCrossing(tabIndex, rawPos);
        newMarkers.push_back({ zcPos, false });
    }

    if (newMarkers.empty()) {
        newMarkers.push_back({ 0, false });
    }

    const juce::ScopedLock sl (sampleLock);
    for (auto& v : voices) {
        if (v.tabIndex == tabIndex) v.isActive = false;
    }
    samples[tabIndex].markers = std::move(newMarkers);
}

void ChopSampAudioProcessor::sliceByTransients(int tabIndex, float sensitivity)
{
    if (tabIndex < 0 || tabIndex >= MAX_SAMPLE_TABS || !samples[tabIndex].isLoaded) return;
    
    auto& buffer = samples[tabIndex].buffer;
    int numSamples = buffer.getNumSamples();
    int numChannels = buffer.getNumChannels();
    if (numSamples <= 0 || numChannels <= 0) return;

    const float* readPtr = buffer.getReadPointer(0); 
    if (!readPtr) return;
    
    std::vector<SliceMarker> newMarkers;
    newMarkers.push_back({ 0, false });

    float threshold = juce::jmap(sensitivity, 0.0f, 100.0f, 0.5f, 0.01f);
    
    double sr = getSampleRate() > 0 ? getSampleRate() : 44100.0;
    int cooldownSamples = (int)(sr * 0.05);
    int lastTransient = 0;

    for (int i = 1; i < numSamples; ++i)
    {
        float diff = std::abs(readPtr[i]) - std::abs(readPtr[i-1]);
        if (diff > threshold && (i - lastTransient) > cooldownSamples)
        {
            int zcPos = findNearestZeroCrossing(tabIndex, i);
            newMarkers.push_back({ zcPos, false });
            lastTransient = i;
        }
    }

    const juce::ScopedLock sl (sampleLock);
    for (auto& v : voices) {
        if (v.tabIndex == tabIndex) v.isActive = false;
    }
    samples[tabIndex].markers = std::move(newMarkers);
}

void ChopSampAudioProcessor::sliceRandom(int tabIndex, int numSlices)
{
    if (tabIndex < 0 || tabIndex >= MAX_SAMPLE_TABS || !samples[tabIndex].isLoaded || numSlices <= 0) return;
    
    int totalSamples = samples[tabIndex].buffer.getNumSamples();
    if (totalSamples <= 0) return;
    
    std::vector<SliceMarker> newMarkers;
    newMarkers.push_back({ 0, false });

    int maxPos = totalSamples - 1000;
    if (maxPos <= 0) maxPos = totalSamples;
    
    for (int i = 1; i < numSlices; ++i)
    {
        int randomPos = juce::Random::getSystemRandom().nextInt(maxPos);
        if (randomPos > 0)
        {
            int zcPos = findNearestZeroCrossing(tabIndex, randomPos);
            newMarkers.push_back({ zcPos, false });
        }
    }
    
    std::sort(newMarkers.begin(), newMarkers.end(),
              [](const SliceMarker& a, const SliceMarker& b) { return a.sampleIndex < b.sampleIndex; });
              
    auto last = std::unique(newMarkers.begin(), newMarkers.end(),
                            [](const SliceMarker& a, const SliceMarker& b) { return a.sampleIndex == b.sampleIndex; });
    newMarkers.erase(last, newMarkers.end());

    const juce::ScopedLock sl (sampleLock);
    for (auto& v : voices) {
        if (v.tabIndex == tabIndex) v.isActive = false;
    }
    samples[tabIndex].markers = std::move(newMarkers);
}

juce::File ChopSampAudioProcessor::exportMidiFile(int tabIndex)
{
    juce::File tempDir = juce::File::getSpecialLocation(juce::File::tempDirectory);
    juce::File midiFile = tempDir.getChildFile("ChopSamp_Pattern.mid");
    midiFile.deleteFile();

    juce::MidiMessageSequence seq;
    const auto& sample = samples[tabIndex];
    int numSlices = (int)sample.markers.size();
    if (numSlices == 0) return {};

    double ticksPerQuarter = 960.0;
    int baseRoot = rootNote.load();
    bool isWhiteOnly = whiteKeysOnly.load();
    const int whiteKeyOffsets[] = { 0, 2, 4, 5, 7, 9, 11 };

    for (int i = 0; i < numSlices; ++i)
    {
        int noteNum = baseRoot;
        if (isWhiteOnly) {
            int oct = i / 7;
            int idx = i % 7;
            noteNum = baseRoot + (oct * 12) + whiteKeyOffsets[idx];
        } else {
            noteNum = baseRoot + i;
        }

        double timeInBeats = i * 1.0;
        double noteOnTime = timeInBeats * ticksPerQuarter;
        double noteOffTime = (timeInBeats + 0.9) * ticksPerQuarter;

        seq.addEvent(juce::MidiMessage::noteOn(1, noteNum, (juce::uint8)100), noteOnTime);
        seq.addEvent(juce::MidiMessage::noteOff(1, noteNum, (juce::uint8)0), noteOffTime);
    }

    juce::MidiFile mf;
    mf.setTicksPerQuarterNote((int)ticksPerQuarter);
    mf.addTrack(seq);

    {
        juce::FileOutputStream stream(midiFile);
        if (stream.openedOk())
        {
            mf.writeTo(stream);
            stream.flush();
        }
    }

    return midiFile;
}

juce::File ChopSampAudioProcessor::exportSliceWavFile(int tabIndex, int sliceIndex)
{
    juce::File tempDir = juce::File::getSpecialLocation(juce::File::tempDirectory);
    juce::File wavFile = tempDir.getChildFile("ChopSamp_Slice_" + juce::String(sliceIndex + 1) + ".wav");
    wavFile.deleteFile();

    const auto& sample = samples[tabIndex];
    int targetSlice = (sliceIndex >= 0 && sliceIndex < sample.markers.size()) ? sliceIndex : 0;
    if (sample.isLoaded && sample.markers.size() > 0)
    {
        int startSamp = sample.markers[targetSlice].sampleIndex;
        int endSamp = (targetSlice + 1 < sample.markers.size()) ? sample.markers[targetSlice + 1].sampleIndex : sample.buffer.getNumSamples();
        int sliceLength = juce::jmax(1, endSamp - startSamp);

        juce::AudioBuffer<float> sliceBuf(sample.buffer.getNumChannels(), sliceLength);
        for (int ch = 0; ch < sample.buffer.getNumChannels(); ++ch)
        {
            sliceBuf.copyFrom(ch, 0, sample.buffer, ch, startSamp, sliceLength);
        }

        juce::WavAudioFormat wavFormat;
        if (auto writer = std::unique_ptr<juce::AudioFormatWriter>(wavFormat.createWriterFor(
                new juce::FileOutputStream(wavFile), sample.sampleRate > 0 ? sample.sampleRate : 44100.0,
                sliceBuf.getNumChannels(), 16, {}, 0)))
        {
            writer->writeFromAudioSampleBuffer(sliceBuf, 0, sliceLength);
        }
    }

    return wavFile;
}

void ChopSampAudioProcessor::clearSlices(int tabIndex)
{
    if (tabIndex >= 0 && tabIndex < MAX_SAMPLE_TABS)
        samples[tabIndex].markers.clear();
}

const juce::String ChopSampAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool ChopSampAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool ChopSampAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool ChopSampAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double ChopSampAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int ChopSampAudioProcessor::getNumPrograms()
{
    return 1;
}

int ChopSampAudioProcessor::getCurrentProgram()
{
    return 0;
}

void ChopSampAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String ChopSampAudioProcessor::getProgramName (int index)
{
    return {};
}

void ChopSampAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

void ChopSampAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    tempRecordBuffer.setSize(2, (int)(sampleRate * 60.0)); // 60 seconds max recording buffer
    for (auto& v : voices) {
        v.adsr.setSampleRate(sampleRate);
    }
}

void ChopSampAudioProcessor::releaseResources()
{
}

bool ChopSampAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // Allow any input channel configuration, even if it's 0 (if user doesn't route audio),
    // but we support stereo inputs for recording.
    return true;
}

void ChopSampAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();
    juce::ignoreUnused (totalNumInputChannels);

    // 1. Capture incoming audio if recording is active BEFORE clearing output buffer
    if (isRecording.load())
    {
        int numSamples = buffer.getNumSamples();
        int maxSamples = tempRecordBuffer.getNumSamples();
        if (recordingSampleIndex + numSamples <= maxSamples)
        {
            int chansToCopy = juce::jmin(buffer.getNumChannels(), tempRecordBuffer.getNumChannels());
            for (int ch = 0; ch < chansToCopy; ++ch)
            {
                tempRecordBuffer.copyFrom(ch, recordingSampleIndex, buffer, ch, 0, numSamples);
            }
            recordingSampleIndex += numSamples;
        }
        else
        {
            stopRecording();
        }
    }

    // 2. Clear outputs for synthesizer voice rendering
    for (int i = 0; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    if (auto* playHead = getPlayHead())
    {
        if (auto positionInfo = playHead->getPosition())
        {
            if (positionInfo->getBpm().hasValue())
                currentBpm = *positionInfo->getBpm();
        }
    }

    keyboardState.processNextMidiBuffer(midiMessages, 0, buffer.getNumSamples(), true);

    const int numSamples = buffer.getNumSamples();
    const double dawSr = getSampleRate() > 0 ? getSampleRate() : 44100.0;
    const int baseRoot = rootNote.load();
    const bool isWhiteOnly = whiteKeysOnly.load();

    auto midiIt = midiMessages.cbegin();
    auto midiEnd = midiMessages.cend();

    // Try locking sample mutex; if locked by UI (loading file / auto chop), silence gracefully to avoid audio thread stalls
    const juce::ScopedTryLock sl(sampleLock);
    if (!sl.isLocked())
        return;

    int activeTab = currentTab.load();
    if (activeTab < 0 || activeTab >= MAX_SAMPLE_TABS)
        return;

    auto& activeSample = samples[activeTab];

    for (int s = 0; s < numSamples; ++s)
    {
        // 1. Process all MIDI messages that occur at or before sample 's'
        while (midiIt != midiEnd && (*midiIt).samplePosition <= s)
        {
            auto message = (*midiIt).getMessage();
            if (message.isPitchWheel())
            {
                int pw = message.getPitchWheelValue();
                currentPitchBendSemi = ((float)pw - 8192.0f) / 8192.0f * pitchBendRangeSemi.load();
            }
            else if (message.isController() && message.getControllerNumber() == 1)
            {
                int ccVal = message.getControllerValue();
                modWheelCutoffHz = juce::jmap((float)ccVal, 0.0f, 127.0f, 200.0f, 20000.0f);
            }
            else if (message.isNoteOn())
            {
                int note = message.getNoteNumber();
                int sliceIndex = -1;

                if (isWhiteOnly)
                {
                    int octave = note / 12;
                    int pitchClass = note % 12;
                    int indexInOctave = -1;
                    switch(pitchClass) {
                        case 0: indexInOctave = 0; break;
                        case 2: indexInOctave = 1; break;
                        case 4: indexInOctave = 2; break;
                        case 5: indexInOctave = 3; break;
                        case 7: indexInOctave = 4; break;
                        case 9: indexInOctave = 5; break;
                        case 11: indexInOctave = 6; break;
                    }
                    if (indexInOctave != -1) {
                        int rootOctave = baseRoot / 12;
                        sliceIndex = (octave - rootOctave) * 7 + indexInOctave;
                    }
                }
                else
                {
                    sliceIndex = note - baseRoot;
                }

                if (sliceIndex >= 0 && activeSample.isLoaded && sliceIndex < (int)activeSample.markers.size())
                {
                    auto& sp = activeSample.markers[sliceIndex].params;
                    float xfadeMs = juce::jmax(2.0f, sp.crossfadeMs);
                    float chokeSamples = juce::jmax(32.0f, (float)(dawSr * (xfadeMs * 0.001f)));

                    // Monophonic choke group: smoothly fade out all currently active voices on this tab
                    for (auto& v : voices) {
                        if (v.isActive && v.tabIndex == activeTab) {
                            v.isChoked = true;
                            v.chokeTotalSamples = chokeSamples;
                            v.chokeElapsedSamples = 0.0f;
                            // Start choke fade-out from its current effective gain for continuous smooth transition
                            v.chokeStartGain = juce::jlimit(0.001f, 1.0f, v.currentEffectiveGain);
                        }
                    }

                    // Voice allocation & stealing
                    ChopSampVoice* voiceToUse = nullptr;
                    for (auto& v : voices) {
                        if (!v.isActive) {
                            voiceToUse = &v;
                            break;
                        }
                    }

                    // If all 16 voices are busy, steal the voice with lowest remaining volume / furthest choke progress
                    if (voiceToUse == nullptr) {
                        float lowestGain = 999.0f;
                        for (auto& v : voices) {
                            float currentGain = v.currentEffectiveGain;
                            if (currentGain < lowestGain) {
                                lowestGain = currentGain;
                                voiceToUse = &v;
                            }
                        }
                    }
                    if (voiceToUse == nullptr) voiceToUse = &voices[0];

                    int startOffsetSamples = (int)(sp.startTrimMs * 0.001 * dawSr);
                    int sliceStart = juce::jlimit(0, activeSample.buffer.getNumSamples() - 1,
                                                 activeSample.markers[sliceIndex].sampleIndex + startOffsetSamples);

                    voiceToUse->isActive = true;
                    voiceToUse->isChoked = false;
                    voiceToUse->chokeStartGain = 1.0f;
                    voiceToUse->chokeTotalSamples = chokeSamples;
                    voiceToUse->chokeElapsedSamples = 0.0f;
                    voiceToUse->fadeInTotalSamples = chokeSamples;
                    voiceToUse->fadeInElapsedSamples = 0.0f;
                    voiceToUse->currentEffectiveGain = 1.0f;
                    voiceToUse->note = note;
                    voiceToUse->tabIndex = activeTab;
                    voiceToUse->sliceIndex = sliceIndex;
                    voiceToUse->currentPosition = sliceStart;
                    voiceToUse->lastLpf = -1.0f;
                    voiceToUse->lastHpf = -1.0f;

                    double fileSr = activeSample.sampleRate > 0 ? activeSample.sampleRate : dawSr;
                    double sampleRateRatio = fileSr / dawSr;
                    voiceToUse->pitchRatio = sampleRateRatio * std::pow(2.0, (sp.pitchSemi + currentPitchBendSemi.load()) / 12.0);

                    voiceToUse->lpfL.reset();
                    voiceToUse->lpfR.reset();
                    voiceToUse->hpfL.reset();
                    voiceToUse->hpfR.reset();

                    float lpfCutoff = juce::jmin(sp.lpfCutoff, modWheelCutoffHz.load());
                    lpfCutoff = juce::jlimit(20.0f, 20000.0f, lpfCutoff);
                    voiceToUse->lpfL.setCoefficients(juce::IIRCoefficients::makeLowPass(dawSr, lpfCutoff));
                    voiceToUse->lpfR.setCoefficients(juce::IIRCoefficients::makeLowPass(dawSr, lpfCutoff));
                    voiceToUse->lastLpf = lpfCutoff;

                    float hpfCutoff = juce::jlimit(20.0f, 10000.0f, sp.hpfCutoff);
                    voiceToUse->hpfL.setCoefficients(juce::IIRCoefficients::makeHighPass(dawSr, hpfCutoff));
                    voiceToUse->hpfR.setCoefficients(juce::IIRCoefficients::makeHighPass(dawSr, hpfCutoff));
                    voiceToUse->lastHpf = hpfCutoff;

                    juce::ADSR::Parameters p;
                    p.attack = juce::jmax(0.002f, sp.attackMs / 1000.0f);
                    p.decay = sp.decayMs / 1000.0f;
                    p.sustain = sp.sustainLevel;
                    p.release = juce::jmax(0.005f, sp.releaseMs / 1000.0f);
                    voiceToUse->adsr.reset();
                    voiceToUse->adsr.setParameters(p);
                    voiceToUse->adsr.noteOn();
                }
            }
            else if (message.isNoteOff())
            {
                int note = message.getNoteNumber();
                for (auto& v : voices) {
                    if (v.isActive && v.note == note && !v.isChoked) {
                        v.adsr.noteOff();
                    }
                }
            }
            ++midiIt;
        }

        // 2. Synthesizer Voice Rendering for Sample 's'
        for (auto& v : voices)
        {
            if (!v.isActive) continue;

            if (v.tabIndex < 0 || v.tabIndex >= MAX_SAMPLE_TABS) {
                v.isActive = false;
                continue;
            }

            auto& sample = samples[v.tabIndex];
            int numSamps = sample.buffer.getNumSamples();
            int numChans = sample.buffer.getNumChannels();
            if (!sample.isLoaded || numSamps <= 0 || numChans <= 0 ||
                v.sliceIndex < 0 || v.sliceIndex >= (int)sample.markers.size()) {
                v.isActive = false;
                continue;
            }

            auto& params = sample.markers[v.sliceIndex].params;
            int rawSliceStart = sample.markers[v.sliceIndex].sampleIndex;
            int rawSliceEnd = (v.sliceIndex + 1 < (int)sample.markers.size()) ?
                sample.markers[v.sliceIndex + 1].sampleIndex : numSamps;

            int startOffsetSamples = (int)(params.startTrimMs * 0.001 * dawSr);
            int endOffsetSamples = (int)(params.endTrimMs * 0.001 * dawSr);

            int sliceStart = juce::jlimit(0, numSamps - 1, rawSliceStart + startOffsetSamples);
            int sliceEnd = juce::jlimit(sliceStart + 1, numSamps, rawSliceEnd + endOffsetSamples);

            double fileSr = sample.sampleRate > 0 ? sample.sampleRate : dawSr;
            double sampleRateRatio = fileSr / dawSr;
            v.pitchRatio = sampleRateRatio * std::pow(2.0, (params.pitchSemi + currentPitchBendSemi.load()) / 12.0);

            float targetLpf = juce::jmin(params.lpfCutoff, modWheelCutoffHz.load());
            targetLpf = juce::jlimit(20.0f, 20000.0f, targetLpf);
            if (std::abs(v.lastLpf - targetLpf) > 0.5f) {
                v.lastLpf = targetLpf;
                v.lpfL.setCoefficients(juce::IIRCoefficients::makeLowPass(dawSr, targetLpf));
                v.lpfR.setCoefficients(juce::IIRCoefficients::makeLowPass(dawSr, targetLpf));
            }

            float targetHpf = juce::jlimit(20.0f, 10000.0f, params.hpfCutoff);
            if (std::abs(v.lastHpf - targetHpf) > 0.5f) {
                v.lastHpf = targetHpf;
                v.hpfL.setCoefficients(juce::IIRCoefficients::makeHighPass(dawSr, targetHpf));
                v.hpfR.setCoefficients(juce::IIRCoefficients::makeHighPass(dawSr, targetHpf));
            }

            float normPan = (params.pan + 1.0f) * 0.5f;
            float panRad = normPan * juce::MathConstants<float>::halfPi;
            float panL = std::cos(panRad);
            float panR = std::sin(panRad);
            float vol = params.volume;

            const float* lPtr = sample.buffer.getReadPointer(0);
            const float* rPtr = (numChans > 1) ? sample.buffer.getReadPointer(1) : lPtr;

            // ADSR envelope calculation
            float env = v.adsr.getNextSample();
            if (!v.adsr.isActive()) {
                v.isActive = false;
                continue;
            }

            // Choke fade-out (equal-power cosine curve)
            float chokeFade = 1.0f;
            if (v.isChoked) {
                if (v.chokeElapsedSamples >= v.chokeTotalSamples) {
                    v.isActive = false;
                    continue;
                }
                float t = v.chokeElapsedSamples / v.chokeTotalSamples;
                chokeFade = v.chokeStartGain * std::cos(t * juce::MathConstants<float>::halfPi);
                v.chokeElapsedSamples += 1.0f;
            }

            // Crossfade fade-in for incoming voice (equal-power sine curve)
            float crossFadeIn = 1.0f;
            if (v.fadeInElapsedSamples < v.fadeInTotalSamples) {
                float t = v.fadeInElapsedSamples / v.fadeInTotalSamples;
                crossFadeIn = std::sin(t * juce::MathConstants<float>::halfPi);
                v.fadeInElapsedSamples += 1.0f;
            }

            // Automatic Reaper-style micro-fades at slice boundaries (de-clicking)
            float boundaryFade = 1.0f;
            const float declickSamples = juce::jlimit(32.0f, (float)(dawSr * 0.01), (float)(dawSr * (params.crossfadeMs * 0.001f)));

            double pos = v.currentPosition;
            if (params.reverse) {
                int anchor = playThroughMode ? numSamps : sliceEnd;
                double offset = v.currentPosition - (double)rawSliceStart;
                pos = (double)anchor - 1.0 - offset;

                // Start fade for reverse playback
                double distFromRevStart = v.currentPosition - (double)rawSliceStart;
                if (distFromRevStart >= 0.0 && distFromRevStart < declickSamples) {
                    float t = (float)(distFromRevStart / declickSamples);
                    boundaryFade *= std::sin(juce::jlimit(0.0f, 1.0f, t) * juce::MathConstants<float>::halfPi);
                }

                // End fade for reverse playback (approaching sliceStart)
                if (!playThroughMode) {
                    double distToStart = pos - (double)sliceStart;
                    if (distToStart <= 0.0) {
                        v.isActive = false;
                        continue;
                    } else if (distToStart < declickSamples) {
                        float t = (float)(distToStart / declickSamples);
                        boundaryFade *= std::sin(juce::jlimit(0.0f, 1.0f, t) * juce::MathConstants<float>::halfPi);
                    }
                }
            } else {
                // Forward playback
                // Start fade for forward playback (de-click start)
                double distFromStart = v.currentPosition - (double)sliceStart;
                if (distFromStart >= 0.0 && distFromStart < declickSamples) {
                    float t = (float)(distFromStart / declickSamples);
                    boundaryFade *= std::sin(juce::jlimit(0.0f, 1.0f, t) * juce::MathConstants<float>::halfPi);
                }

                // End fade for forward playback (approaching sliceEnd)
                if (!playThroughMode) {
                    double distToEnd = (double)sliceEnd - v.currentPosition;
                    if (distToEnd <= 0.0) {
                        v.isActive = false;
                        continue;
                    } else if (distToEnd < declickSamples) {
                        float t = (float)(distToEnd / declickSamples);
                        boundaryFade *= std::sin(juce::jlimit(0.0f, 1.0f, t) * juce::MathConstants<float>::halfPi);
                    }
                }
            }

            if (pos < 0.0 || pos >= (double)(numSamps - 1))
            {
                v.isActive = false;
                continue;
            }

            int p1 = (int)pos;
            int p2 = p1 + 1;
            float frac = (float)(pos - (double)p1);

            float leftVal = lPtr[p1] + frac * (lPtr[p2] - lPtr[p1]);
            float rightVal = rPtr[p1] + frac * (rPtr[p2] - rPtr[p1]);

            if (targetLpf < 19500.0f) {
                leftVal = v.lpfL.processSingleSampleRaw(leftVal);
                rightVal = v.lpfR.processSingleSampleRaw(rightVal);
            }
            if (targetHpf > 25.0f) {
                leftVal = v.hpfL.processSingleSampleRaw(leftVal);
                rightVal = v.hpfR.processSingleSampleRaw(rightVal);
            }

            float gain = vol * env * chokeFade * crossFadeIn * boundaryFade;
            v.currentEffectiveGain = chokeFade;

            buffer.addSample(0, s, leftVal * panL * gain);
            if (buffer.getNumChannels() > 1)
                buffer.addSample(1, s, rightVal * panR * gain);

            v.currentPosition += v.pitchRatio;
        }
    }

    // Apply Master Volume and track Peak Audio Output Levels
    float mVol = masterVolume.load();
    buffer.applyGain(mVol);
    
    float peakL = buffer.getMagnitude(0, 0, buffer.getNumSamples());
    float peakR = buffer.getNumChannels() > 1 ? buffer.getMagnitude(1, 0, buffer.getNumSamples()) : peakL;
    
    outputLevelL = juce::jmax(peakL, outputLevelL.load() * 0.85f);
    outputLevelR = juce::jmax(peakR, outputLevelR.load() * 0.85f);
}

bool ChopSampAudioProcessor::hasEditor() const
{
    return true; 
}

juce::AudioProcessorEditor* ChopSampAudioProcessor::createEditor()
{
    return new ChopSampAudioProcessorEditor (*this);
}

void ChopSampAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    juce::ValueTree state("ChopSampState");
    state.setProperty("version", 1, nullptr);
    state.setProperty("currentTab", currentTab.load(), nullptr);
    state.setProperty("playThroughMode", playThroughMode.load(), nullptr);
    state.setProperty("masterVolume", masterVolume.load(), nullptr);
    state.setProperty("whiteKeysOnly", whiteKeysOnly.load(), nullptr);
    state.setProperty("pitchBendRangeSemi", pitchBendRangeSemi.load(), nullptr);
    state.setProperty("rootNote", rootNote.load(), nullptr);
    state.setProperty("currentTheme", currentTheme.load(), nullptr);

    juce::ValueTree tabsNode("Tabs");
    {
        const juce::ScopedLock sl (sampleLock);
        for (int i = 0; i < MAX_SAMPLE_TABS; ++i)
        {
            const auto& sample = samples[i];
            if (sample.isLoaded && sample.buffer.getNumSamples() > 0)
            {
                juce::ValueTree tabNode("Tab");
                tabNode.setProperty("index", i, nullptr);
                tabNode.setProperty("name", sample.name, nullptr);
                tabNode.setProperty("filePath", sample.filePath, nullptr);
                tabNode.setProperty("sampleRate", sample.sampleRate, nullptr);
                tabNode.setProperty("bitDepth", sample.bitDepth, nullptr);
                tabNode.setProperty("format", sample.format, nullptr);

                // Serialize audio buffer as lossless WAV into a MemoryBlock
                juce::MemoryBlock audioMem;
                {
                    juce::WavAudioFormat wavFormat;
                    if (auto writer = std::unique_ptr<juce::AudioFormatWriter>(
                            wavFormat.createWriterFor(new juce::MemoryOutputStream(audioMem, false),
                                                      sample.sampleRate > 0 ? sample.sampleRate : 44100.0,
                                                      (unsigned int)sample.buffer.getNumChannels(),
                                                      sample.bitDepth > 0 ? sample.bitDepth : 16,
                                                      {}, 0)))
                    {
                        writer->writeFromAudioSampleBuffer(sample.buffer, 0, sample.buffer.getNumSamples());
                    }
                }
                tabNode.setProperty("audioData", audioMem, nullptr);

                // Markers & per-slice parameters
                juce::ValueTree markersNode("Markers");
                for (const auto& m : sample.markers)
                {
                    juce::ValueTree mNode("Marker");
                    mNode.setProperty("sampleIndex", m.sampleIndex, nullptr);
                    mNode.setProperty("isSelected", m.isSelected, nullptr);
                    mNode.setProperty("volume", m.params.volume, nullptr);
                    mNode.setProperty("pan", m.params.pan, nullptr);
                    mNode.setProperty("pitchSemi", m.params.pitchSemi, nullptr);
                    mNode.setProperty("reverse", m.params.reverse, nullptr);
                    mNode.setProperty("startTrimMs", m.params.startTrimMs, nullptr);
                    mNode.setProperty("endTrimMs", m.params.endTrimMs, nullptr);
                    mNode.setProperty("attackMs", m.params.attackMs, nullptr);
                    mNode.setProperty("decayMs", m.params.decayMs, nullptr);
                    mNode.setProperty("sustainLevel", m.params.sustainLevel, nullptr);
                    mNode.setProperty("releaseMs", m.params.releaseMs, nullptr);
                    mNode.setProperty("crossfadeMs", m.params.crossfadeMs, nullptr);
                    mNode.setProperty("filterCutoff", m.params.filterCutoff, nullptr);
                    mNode.setProperty("lpfCutoff", m.params.lpfCutoff, nullptr);
                    mNode.setProperty("hpfCutoff", m.params.hpfCutoff, nullptr);
                    mNode.setProperty("delayMix", m.params.delayMix, nullptr);
                    mNode.setProperty("reverbMix", m.params.reverbMix, nullptr);
                    mNode.setProperty("sliceName", m.params.sliceName, nullptr);
                    mNode.setProperty("colorARGB", (juce::int64)m.params.colorARGB, nullptr);
                    markersNode.appendChild(mNode, nullptr);
                }
                tabNode.appendChild(markersNode, nullptr);
                tabsNode.appendChild(tabNode, nullptr);
            }
        }
    }
    state.appendChild(tabsNode, nullptr);

    juce::MemoryOutputStream stream(destData, false);
    state.writeToStream(stream);
}

void ChopSampAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    if (data == nullptr || sizeInBytes <= 0)
        return;

    juce::MemoryInputStream stream(data, static_cast<size_t>(sizeInBytes), false);
    auto state = juce::ValueTree::readFromStream(stream);
    if (!state.isValid() || !state.hasType("ChopSampState"))
        return;

    // 1. Restore global parameters
    if (state.hasProperty("currentTab"))
        currentTab = (int)state.getProperty("currentTab");
    if (state.hasProperty("playThroughMode"))
        playThroughMode = (bool)state.getProperty("playThroughMode");
    if (state.hasProperty("masterVolume"))
        masterVolume = (float)state.getProperty("masterVolume");
    if (state.hasProperty("whiteKeysOnly"))
        whiteKeysOnly = (bool)state.getProperty("whiteKeysOnly");
    if (state.hasProperty("pitchBendRangeSemi"))
        pitchBendRangeSemi = (float)state.getProperty("pitchBendRangeSemi");
    if (state.hasProperty("rootNote"))
        rootNote = (int)state.getProperty("rootNote");
    if (state.hasProperty("currentTheme"))
        currentTheme = (int)state.getProperty("currentTheme");

    // 2. Restore samples and markers under lock
    {
        const juce::ScopedLock sl (sampleLock);
        for (auto& v : voices) {
            v.isActive = false;
        }

        for (int i = 0; i < MAX_SAMPLE_TABS; ++i) {
            samples[i].isLoaded = false;
            samples[i].buffer.setSize(0, 0);
            samples[i].markers.clear();
            samples[i].name = "Empty";
            samples[i].filePath = "";
        }

        auto tabsNode = state.getChildWithName("Tabs");
        if (tabsNode.isValid())
        {
            for (int t = 0; t < tabsNode.getNumChildren(); ++t)
            {
                auto tabNode = tabsNode.getChild(t);
                if (tabNode.isValid() && tabNode.hasType("Tab"))
                {
                    int tabIndex = tabNode.getProperty("index", -1);
                    if (tabIndex >= 0 && tabIndex < MAX_SAMPLE_TABS)
                    {
                        auto& sample = samples[tabIndex];
                        sample.name = tabNode.getProperty("name", "Audio");
                        sample.filePath = tabNode.getProperty("filePath", "");
                        sample.sampleRate = tabNode.getProperty("sampleRate", 44100.0);
                        sample.bitDepth = tabNode.getProperty("bitDepth", 16);
                        sample.format = tabNode.getProperty("format", ".wav");

                        bool bufferLoaded = false;

                        // Check for embedded audioData MemoryBlock first
                        if (tabNode.hasProperty("audioData"))
                        {
                            if (auto* mb = tabNode.getProperty("audioData").getBinaryData())
                            {
                                if (mb->getSize() > 0)
                                {
                                    auto memIn = std::make_unique<juce::MemoryInputStream>(*mb, false);
                                    juce::WavAudioFormat wavFormat;
                                    if (auto reader = std::unique_ptr<juce::AudioFormatReader>(wavFormat.createReaderFor(memIn.release(), true)))
                                    {
                                        int numChans = (int)reader->numChannels;
                                        int numSamps = (int)reader->lengthInSamples;
                                        if (numChans > 0 && numSamps > 0)
                                        {
                                            sample.buffer.setSize(numChans, numSamps);
                                            reader->read(&sample.buffer, 0, numSamps, 0, true, numChans > 1);
                                            bufferLoaded = true;
                                        }
                                    }
                                }
                            }
                        }

                        // Fallback: If audioData wasn't present or failed to read, try loading from filePath
                        if (!bufferLoaded && sample.filePath.isNotEmpty())
                        {
                            juce::File file(sample.filePath);
                            if (file.existsAsFile())
                            {
                                if (auto reader = std::unique_ptr<juce::AudioFormatReader>(formatManager.createReaderFor(file)))
                                {
                                    int numChans = (int)reader->numChannels;
                                    int numSamps = (int)reader->lengthInSamples;
                                    if (numChans > 0 && numSamps > 0)
                                    {
                                        sample.buffer.setSize(numChans, numSamps);
                                        reader->read(&sample.buffer, 0, numSamps, 0, true, numChans > 1);
                                        bufferLoaded = true;
                                    }
                                }
                            }
                        }

                        if (bufferLoaded)
                        {
                            sample.isLoaded = true;

                            // Restore markers
                            auto markersNode = tabNode.getChildWithName("Markers");
                            if (markersNode.isValid())
                            {
                                std::vector<SliceMarker> loadedMarkers;
                                for (int mIdx = 0; mIdx < markersNode.getNumChildren(); ++mIdx)
                                {
                                    auto mNode = markersNode.getChild(mIdx);
                                    if (mNode.isValid() && mNode.hasType("Marker"))
                                    {
                                        SliceMarker sm;
                                        sm.sampleIndex = mNode.getProperty("sampleIndex", 0);
                                        sm.isSelected = mNode.getProperty("isSelected", false);
                                        sm.params.volume = mNode.getProperty("volume", 1.0f);
                                        sm.params.pan = mNode.getProperty("pan", 0.0f);
                                        sm.params.pitchSemi = mNode.getProperty("pitchSemi", 0.0f);
                                        sm.params.reverse = mNode.getProperty("reverse", false);
                                        sm.params.startTrimMs = mNode.getProperty("startTrimMs", 0.0f);
                                        sm.params.endTrimMs = mNode.getProperty("endTrimMs", 0.0f);
                                        sm.params.attackMs = mNode.getProperty("attackMs", 10.0f);
                                        sm.params.decayMs = mNode.getProperty("decayMs", 100.0f);
                                        sm.params.sustainLevel = mNode.getProperty("sustainLevel", 1.0f);
                                        sm.params.releaseMs = mNode.getProperty("releaseMs", 100.0f);
                                        sm.params.crossfadeMs = mNode.getProperty("crossfadeMs", 20.0f);
                                        sm.params.filterCutoff = mNode.getProperty("filterCutoff", 20000.0f);
                                        sm.params.lpfCutoff = mNode.getProperty("lpfCutoff", 20000.0f);
                                        sm.params.hpfCutoff = mNode.getProperty("hpfCutoff", 20.0f);
                                        sm.params.delayMix = mNode.getProperty("delayMix", 0.0f);
                                        sm.params.reverbMix = mNode.getProperty("reverbMix", 0.0f);
                                        sm.params.sliceName = mNode.getProperty("sliceName", "");
                                        sm.params.colorARGB = (juce::uint32)(juce::int64)mNode.getProperty("colorARGB", 0);
                                        loadedMarkers.push_back(sm);
                                    }
                                }
                                if (!loadedMarkers.empty()) {
                                    sample.markers = std::move(loadedMarkers);
                                } else {
                                    sample.markers.push_back({ 0, false });
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    // 3. Notify Editor if open to refresh all UI components asynchronously on the message thread
    if (auto* editor = getActiveEditor())
    {
        juce::Component::SafePointer<juce::AudioProcessorEditor> safeEditor(editor);
        juce::MessageManager::callAsync([safeEditor]() {
            if (safeEditor != nullptr)
            {
                if (auto* ed = dynamic_cast<ChopSampAudioProcessorEditor*>(safeEditor.getComponent()))
                    ed->updateUIFromProcessorState();
            }
        });
    }
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new ChopSampAudioProcessor();
}
