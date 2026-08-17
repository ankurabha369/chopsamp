#include "WaveformComponent.h"
#include "CustomLookAndFeel.h"

WaveformComponent::WaveformComponent(ChopSampAudioProcessor &p)
    : audioProcessor(p) {
  logoImage = juce::ImageCache::getFromMemory(BinaryData::logo_png,
                                              BinaryData::logo_pngSize);
  setTooltip("Waveform Screen: Double-Click anywhere to Add/Delete slice marker | Drag top handle to move marker | Scroll wheel to zoom | Right-drag to pan");
}

WaveformComponent::~WaveformComponent() {}

float WaveformComponent::sampleToX(int sampleIndex, int numSamples, float width,
                                   float startX) const {
  double relPos = (double)sampleIndex / numSamples;
  double zoomedPos = relPos * zoomRatio - scrollOffset;
  return startX + (float)(zoomedPos * width);
}

int WaveformComponent::xToSample(float x, int numSamples, float width,
                                 float startX) const {
  if (numSamples <= 0)
    return 0;
  double zoomedPos = (x - startX) / width;
  double relPos = (zoomedPos + scrollOffset) / zoomRatio;
  relPos = juce::jlimit(0.0, 1.0, relPos);
  int s = (int)(relPos * numSamples);
  return juce::jlimit(0, numSamples - 1, s);
}

juce::Rectangle<int> WaveformComponent::getScreenBounds() const {
  auto fullBounds = getLocalBounds();
  fullBounds.removeFromBottom(54);
  return fullBounds.reduced(14, 10);
}

void WaveformComponent::mouseWheelMove(const juce::MouseEvent &e,
                                       const juce::MouseWheelDetails &wheel) {
  if (audioProcessor.samples[audioProcessor.currentTab].isLoaded) {
    auto waveformArea = getScreenBounds();
    if (waveformArea.contains(e.getPosition())) {
      int numSamples = audioProcessor.samples[audioProcessor.currentTab]
                           .buffer.getNumSamples();

      double sampleUnderMouse =
          xToSample(e.getPosition().x, numSamples, waveformArea.getWidth(),
                    waveformArea.getX());
      double relSampleUnderMouse = sampleUnderMouse / numSamples;

      double zoomAmount = wheel.deltaY * 2.0;
      zoomRatio *= (1.0 + zoomAmount);
      zoomRatio = juce::jlimit(1.0, 100.0, zoomRatio);

      if (wheel.deltaX != 0.0) {
        scrollOffset -= wheel.deltaX * 2.0;
      }

      if (wheel.deltaY != 0.0) {
        double mouseRelX = (double)(e.getPosition().x - waveformArea.getX()) /
                           waveformArea.getWidth();
        scrollOffset = relSampleUnderMouse * zoomRatio - mouseRelX;
      }

      scrollOffset =
          juce::jlimit(0.0, juce::jmax(0.0, zoomRatio - 1.0), scrollOffset);
      repaint();
    }
  }
}

static juce::Font getMinecraftFont(float height) {
  static auto typeface = juce::Typeface::createSystemTypefaceFor(
      BinaryData::Minecraft_ttf, BinaryData::Minecraft_ttfSize);
  juce::Font f(typeface);
  f.setHeight(height);
  return f;
}

void WaveformComponent::paint(juce::Graphics &g) {
  auto fullBounds = getLocalBounds();

  int activeTab = audioProcessor.currentTab;
  auto &activeSample = audioProcessor.samples[activeTab];

  // 1. Draw Main Large Black Box (Encloses Inner Display + Logo)
  g.setColour(juce::Colour::fromRGB(0, 0, 0));
  g.fillRect(fullBounds);

  // 2. Draw ChopSamp Logo centered inside bottom area of the Main Black Box
  auto logoArea = fullBounds.removeFromBottom(54);
  if (logoImage.isValid()) {
    g.drawImageWithin(logoImage, logoArea.getX(), logoArea.getY() + 8,
                      logoArea.getWidth(), logoArea.getHeight() - 16,
                      juce::RectanglePlacement::centred |
                          juce::RectanglePlacement::onlyReduceInSize);
  } else {
    g.setFont(
        juce::Font("Georgia", 24.0f, juce::Font::bold | juce::Font::italic));
    g.setColour(juce::Colours::white);
    g.drawText("ChopSamp", logoArea.removeFromTop(24),
               juce::Justification::centred);
    g.setFont(juce::Font("Arial", 10.0f, juce::Font::italic));
    g.setColour(juce::Colours::white.withAlpha(0.7f));
    g.drawText("by Ankur", logoArea, juce::Justification::centred);
  }

  // 3. Inner Display Screen (recessed with thicker, authentic black linings)
  auto screenArea = fullBounds.reduced(14, 10);
  g.setColour(juce::Colour::fromRGB(20, 22, 31));
  g.fillRect(screenArea);

  // Bottom metadata strip inside the unified screen
  auto infoStrip = screenArea.removeFromBottom(28);
  auto waveformArea = screenArea;
  float availableH = (float)waveformArea.getHeight();

  if (audioProcessor.isRecording) {
    // Top banner for recording indicator (compact and clear, leaving whole display for live wave)
    auto topBanner = waveformArea.removeFromTop(20).reduced(8, 2);
    
    // Glowing red indicator dot
    float dotSize = 8.0f;
    g.setColour(juce::Colour::fromRGB(255, 50, 50));
    g.fillEllipse((float)topBanner.getX(), topBanner.getCentreY() - dotSize * 0.5f, dotSize, dotSize);
    
    // Crisp small recording text at top
    g.setFont(CustomLookAndFeel::getRobotoFont(10.5f, true));
    g.setColour(juce::Colour::fromRGB(255, 120, 120));
    g.drawText("REC AUDIO FROM DAW...", topBanner.withTrimmedLeft((int)dotSize + 6),
               juce::Justification::centredLeft, true);

    int numRecorded = audioProcessor.recordingSampleIndex;
    if (numRecorded > 0 &&
        audioProcessor.tempRecordBuffer.getNumSamples() > 0) {
      const float *readPtr = audioProcessor.tempRecordBuffer.getReadPointer(0);
      juce::Path recPath;
      int width = waveformArea.getWidth();
      bool firstPoint = true;
      for (int i = 0; i < width; ++i) {
        int sampleIdx = (int)((float)i / (float)width * numRecorded);
        if (sampleIdx >= 0 && sampleIdx < numRecorded) {
          float val = readPtr[sampleIdx];
          float px = waveformArea.getX() + i;
          float py = waveformArea.getCentreY() +
                     val * (waveformArea.getHeight() / 2.3f);
          if (firstPoint) {
            recPath.startNewSubPath(px, py);
            firstPoint = false;
          } else {
            recPath.lineTo(px, py);
          }
        }
      }
      g.setColour(juce::Colour::fromRGB(240, 115, 125));
      g.strokePath(recPath, juce::PathStrokeType(1.5f));
    }
  } else if (activeSample.isLoaded && activeSample.buffer.getNumSamples() > 0 &&
             activeSample.buffer.getNumChannels() > 0) {
    int numSamples = activeSample.buffer.getNumSamples();
    int numChannels = activeSample.buffer.getNumChannels();
    if (numSamples <= 0 || numChannels <= 0)
      return;

    const float *leftPtr = activeSample.buffer.getReadPointer(0);
    if (!leftPtr)
      return;
    const float *rightPtr =
        numChannels > 1 ? activeSample.buffer.getReadPointer(1) : leftPtr;
    if (!rightPtr)
      rightPtr = leftPtr;

    float halfH = availableH / 2.0f;
    float topCentreY = (float)waveformArea.getY() + halfH * 0.5f;
    float botCentreY = (float)waveformArea.getY() + halfH * 1.5f;

    // Accurate Rich Saturated Backgrounds & Pastel Foreground Waveforms
    // matching user design
    struct SliceTheme {
      juce::uint32 bg;
      juce::uint32 wave;
    };
    static const SliceTheme sliceThemes[] = {
        {0xFF4E1A33, 0xFFECA4C2}, // 1: Plum Purple -> Pastel Lilac Wave
        {0xFF4D441D, 0xFFF5E89E}, // 2: Olive Gold   -> Pastel Cream Wave
        {0xFF282D5E, 0xFFB8C2F8}, // 3: Deep Indigo  -> Pastel Periwinkle Wave
        {0xFF274A27, 0xFFB0EAA8}, // 4: Forest Green -> Pastel Mint Wave
        {0xFF204555, 0xFF9EE0EA}, // 5: Ocean Teal   -> Pastel Cyan Wave
        {0xFF422055, 0xFFD8ACF8}, // 6: Royal Violet -> Pastel Lavender Wave
        {0xFF552026, 0xFFF5A4A8}, // 7: Crimson      -> Pastel Rose Wave
        {0xFF232D55, 0xFFA4C0F8}  // 8: Slate Navy   -> Pastel Ice Blue Wave
    };
    const int numThemes = sizeof(sliceThemes) / sizeof(sliceThemes[0]);

    // Draw Slices with Rich Saturated Blocks + Chunky Pixel-Art Stereo Peak
    // Waveforms
    for (int i = 0; i < activeSample.markers.size(); ++i) {
      const auto &marker = activeSample.markers[i];
      int startSamp = marker.sampleIndex;
      int endSamp = (i + 1 < activeSample.markers.size())
                        ? activeSample.markers[i + 1].sampleIndex
                        : numSamples;

      float startX = sampleToX(startSamp, numSamples, waveformArea.getWidth(),
                               waveformArea.getX());
      float endX = sampleToX(endSamp, numSamples, waveformArea.getWidth(),
                             waveformArea.getX());

      float renderStartX = juce::jmax((float)waveformArea.getX(), startX);
      float renderEndX = juce::jmin((float)waveformArea.getRight(), endX);

      if (renderEndX > renderStartX) {
        int sliceWidthPixels = (int)std::ceil(renderEndX - renderStartX);
        bool isPlaying = false;
        for (const auto &v : audioProcessor.voices) {
          if (v.isActive && v.tabIndex == activeTab && v.sliceIndex == i) {
            isPlaying = true;
            break;
          }
        }

        auto themeColor = sliceThemes[i % numThemes];
        juce::Colour baseColor = (marker.params.colorARGB != 0)
                                     ? juce::Colour(marker.params.colorARGB)
                                     : juce::Colour(themeColor.bg);
        juce::Colour waveColor = (marker.params.colorARGB != 0)
                                     ? baseColor.withMultipliedSaturation(0.35f)
                                           .withMultipliedBrightness(2.2f)
                                     : juce::Colour(themeColor.wave);

        // 1. Draw Rich Saturated Slice Background Block
        if (isPlaying) {
          g.setColour(
              baseColor.withMultipliedBrightness(1.35f).withAlpha(0.95f));
        } else if (i == selectedSliceIndex) {
          g.setColour(
              baseColor.withMultipliedBrightness(1.15f).withAlpha(0.85f));
        } else {
          g.setColour(
              baseColor.withMultipliedBrightness(0.85f).withAlpha(0.75f));
        }
        g.fillRect(renderStartX, (float)waveformArea.getY(),
                   (float)sliceWidthPixels, availableH);

        // 2. Draw Subtle Dotted Cue Lines inside slice
        if (sliceWidthPixels > 55) {
          float dashX1 = renderStartX + sliceWidthPixels * 0.38f;
          float dashX2 = renderStartX + sliceWidthPixels * 0.72f;
          g.setColour(juce::Colours::white.withAlpha(0.20f));
          for (float dy = (float)waveformArea.getY() + 16.0f;
               dy < (float)waveformArea.getY() + availableH; dy += 6.0f) {
            g.fillRect(dashX1, dy, 1.5f, 3.0f);
            if (sliceWidthPixels > 95)
              g.fillRect(dashX2, dy, 1.5f, 3.0f);
          }
        }

        // 3. Draw Chunky Quantized Retro Digital Pixel Stereo Waveform
        g.setColour(waveColor);

        int stepX = 2; // 2px chunky pixel blocks
        for (int x = 0; x < sliceWidthPixels; x += stepX) {
          float px = renderStartX + x;
          int sStart = xToSample(px, numSamples, waveformArea.getWidth(),
                                 waveformArea.getX());
          int sEnd = xToSample(px + stepX, numSamples, waveformArea.getWidth(),
                               waveformArea.getX());
          if (sEnd <= sStart)
            sEnd = sStart + 1;

          if (marker.params.reverse) {
            float sliceWidth = endX - startX;
            if (sliceWidth > 0.001f) {
              float frac1 = (px - startX) / sliceWidth;
              float frac2 = (px + stepX - startX) / sliceWidth;
              sStart = endSamp - 1 - (int)(frac1 * (endSamp - startSamp));
              sEnd = endSamp - 1 - (int)(frac2 * (endSamp - startSamp));
              if (sStart > sEnd)
                std::swap(sStart, sEnd);
            }
          }

          sStart = juce::jlimit(0, numSamples - 1, sStart);
          sEnd = juce::jlimit(0, numSamples, sEnd);

          float minL = 0.0f, maxL = 0.0f, minR = 0.0f, maxR = 0.0f;
          int step = juce::jmax(1, (sEnd - sStart) / 16);
          for (int s = sStart; s < sEnd; s += step) {
            float l = leftPtr[s];
            float r = rightPtr[s];
            if (l < minL)
              minL = l;
            if (l > maxL)
              maxL = l;
            if (r < minR)
              minR = r;
            if (r > maxR)
              maxR = r;
          }

          // Enforce solid visible baseline
          if (std::abs(maxL - minL) < 0.04f) {
            minL = -0.015f;
            maxL = 0.015f;
          }
          if (std::abs(maxR - minR) < 0.04f) {
            minR = -0.015f;
            maxR = 0.015f;
          }

          float ampScale = halfH * 0.45f;
          float yTopL = topCentreY + minL * ampScale;
          float yBotL = topCentreY + maxL * ampScale;
          float yTopR = botCentreY + minR * ampScale;
          float yBotR = botCentreY + maxR * ampScale;

          // Quantize to pixel grid
          yTopL = std::floor(yTopL);
          yBotL = std::ceil(yBotL);
          yTopR = std::floor(yTopR);
          yBotR = std::ceil(yBotR);

          g.fillRect(px, yTopL, (float)stepX, juce::jmax(2.0f, yBotL - yTopL));
          g.fillRect(px, yTopR, (float)stepX, juce::jmax(2.0f, yBotR - yTopR));
        }

        // 4. Draw Top Header Tag ("S1", "S2") in Minecraft Font
        if (sliceWidthPixels > 14) {
          juce::Font mcFont = getMinecraftFont(8.0f);
          g.setFont(mcFont);

          juce::String tag = "S" + juce::String(i + 1);
          int tagW = mcFont.getStringWidth(tag) + 4;

          g.setColour(
              juce::Colour::fromRGB(254, 164, 28)); // Bright Orange S1 tag
          g.drawText(tag, (int)renderStartX + 5, (int)waveformArea.getY() + 4,
                     tagW, 14, juce::Justification::topLeft, false);

          juce::String sName = marker.params.sliceName;
          if (sName.isEmpty() && i == 0)
            sName = "One no";
          if (sName.isNotEmpty() && sliceWidthPixels > (tagW + 20)) {
            g.setColour(juce::Colours::white);
            g.drawText(sName, (int)renderStartX + 5 + tagW,
                       (int)waveformArea.getY() + 4,
                       sliceWidthPixels - tagW - 6, 14,
                       juce::Justification::topLeft, true);
          }
        }
      }
    }

    // Horizontal Channel Center Divider
    g.setColour(juce::Colour::fromRGB(10, 12, 16));
    g.drawLine((float)waveformArea.getX(), (float)waveformArea.getY() + halfH,
               (float)waveformArea.getRight(),
               (float)waveformArea.getY() + halfH, 1.5f);

    // Slice Boundary Lines (Bright Orange / Gold like reference)
    juce::Colour markerOrange = juce::Colour::fromRGB(254, 164, 28);
    for (int i = 0; i < activeSample.markers.size(); ++i) {
      const auto &marker = activeSample.markers[i];
      float mx = sampleToX(marker.sampleIndex, numSamples,
                           waveformArea.getWidth(), waveformArea.getX());

      if (mx >= waveformArea.getX() && mx <= waveformArea.getRight()) {
        juce::Colour markerCol = (marker.params.colorARGB != 0)
                                     ? juce::Colour(marker.params.colorARGB)
                                     : markerOrange;
        g.setColour(markerCol);
        g.drawLine(mx, (float)waveformArea.getY(), mx,
                   (float)waveformArea.getY() + availableH, 2.0f);

        // Inverted Triangle Flag
        juce::Path tri;
        tri.addTriangle(mx - 5.0f, (float)waveformArea.getY(), mx + 5.0f,
                        (float)waveformArea.getY(), mx,
                        (float)waveformArea.getY() + 8.0f);
        g.fillPath(tri);
      }
    }

    // Playhead indicators
    g.setColour(juce::Colours::white);
    for (const auto &v : audioProcessor.voices) {
      if (v.isActive && v.tabIndex == activeTab) {
        bool drawPlayhead = true;
        if (v.sliceIndex >= 0 && v.sliceIndex < activeSample.markers.size()) {
          if (!audioProcessor.playThroughMode) {
            int endSamp =
                (v.sliceIndex + 1 < activeSample.markers.size())
                    ? activeSample.markers[v.sliceIndex + 1].sampleIndex
                    : numSamples;
            if (v.currentPosition >= endSamp || v.isChoked) {
              drawPlayhead = false;
            }
          } else {
            if (v.currentPosition >= numSamples || v.isChoked) {
              drawPlayhead = false;
            }
          }
        }

        if (drawPlayhead) {
          float px = sampleToX(v.currentPosition, numSamples,
                               waveformArea.getWidth(), waveformArea.getX());
          if (px >= waveformArea.getX() && px <= waveformArea.getRight()) {
            g.drawLine(px, waveformArea.getY(), px,
                       (float)waveformArea.getBottom(), 2.0f);
          }
        }
      }
    }
  }

  // 4. Draw Bottom LCD Metadata Text directly on unified screen
  juce::Font mcFontMeta = getMinecraftFont(11.5f);
  g.setFont(mcFontMeta);
  g.setColour(juce::Colours::white);

  juce::String sampleName =
      activeSample.isLoaded ? activeSample.name : "Drag a Sample to Start";

  juce::String sampleText = "Sample: " + sampleName;
  g.drawText(sampleText, infoStrip.getX() + 12, infoStrip.getY(),
             infoStrip.getWidth() - 320, infoStrip.getHeight(),
             juce::Justification::centredLeft, true);

  if (activeSample.isLoaded && activeSample.buffer.getNumSamples() > 0) {
    double sr = activeSample.sampleRate > 0 ? activeSample.sampleRate : 44100.0;
    juce::String fmtStr =
        activeSample.format.isNotEmpty() ? activeSample.format : ".wav";
    if (!fmtStr.startsWith("."))
      fmtStr = "." + fmtStr;

    juce::String bitStr = juce::String(activeSample.bitDepth) + "bit";

    int srK = (int)std::round(sr / 1000.0);
    juce::String srStr =
        (sr == 44100.0) ? "44Khz" : (juce::String(srK) + "Khz");

    double totalSec = (double)activeSample.buffer.getNumSamples() / sr;
    int mins = (int)totalSec / 60;
    double secs = totalSec - (mins * 60);
    juce::String durStr = juce::String::formatted("[%d:%05.2fs]", mins, secs);

    // Format right side: [0:04.23s]       .wav       16bit       44Khz
    int rightX = infoStrip.getRight() - 14;

    int srW = mcFontMeta.getStringWidth(srStr) + 6;
    g.drawText(srStr, rightX - srW, infoStrip.getY(), srW,
               infoStrip.getHeight(), juce::Justification::centredRight, false);
    rightX -= (srW + 20);

    int bitW = mcFontMeta.getStringWidth(bitStr) + 6;
    g.drawText(bitStr, rightX - bitW, infoStrip.getY(), bitW,
               infoStrip.getHeight(), juce::Justification::centredRight, false);
    rightX -= (bitW + 20);

    int fmtW = mcFontMeta.getStringWidth(fmtStr) + 6;
    g.drawText(fmtStr, rightX - fmtW, infoStrip.getY(), fmtW,
               infoStrip.getHeight(), juce::Justification::centredRight, false);
    rightX -= (fmtW + 24);

    int durW = mcFontMeta.getStringWidth(durStr) + 6;
    g.setColour(
        juce::Colour::fromRGB(240, 115, 125)); // High-visibility duration tag
    g.drawText(durStr, rightX - durW, infoStrip.getY(), durW,
               infoStrip.getHeight(), juce::Justification::centredRight, false);
  }
}

void WaveformComponent::resized() {}

void WaveformComponent::mouseDown(const juce::MouseEvent &e) {
  auto waveformArea = getScreenBounds();
  if (waveformArea.contains(e.getPosition())) {
    if (e.mods.isRightButtonDown() || e.mods.isMiddleButtonDown() ||
        e.mods.isShiftDown()) {
      isPanning = true;
      lastPanMousePos = e.getPosition();
      setMouseCursor(juce::MouseCursor::DraggingHandCursor);
      return;
    }

    auto &sample = audioProcessor.samples[audioProcessor.currentTab];
    if (sample.isLoaded && sample.markers.size() > 0) {
      int numSamples = sample.buffer.getNumSamples();
      int samplePos = xToSample(e.getPosition().x, numSamples,
                                waveformArea.getWidth(), waveformArea.getX());

      int clickedMarker = -1;
      // Marker drag is ONLY permitted if clicking near top handle (within 25px
      // of top) and marker is > 0
      if (e.getPosition().y <= (waveformArea.getY() + 25)) {
        for (int i = 1; i < sample.markers.size(); ++i) {
          float mx = sampleToX(sample.markers[i].sampleIndex, numSamples,
                               waveformArea.getWidth(), waveformArea.getX());
          if (std::abs(e.getPosition().x - mx) <= 8.0f) {
            clickedMarker = i;
            break;
          }
        }
      }

      if (clickedMarker > 0) {
        draggingMarkerIndex = clickedMarker;
      } else {
        int foundIdx = -1;
        for (int i = 0; i < sample.markers.size(); ++i) {
          if (sample.markers[i].sampleIndex <= samplePos) {
            foundIdx = i;
          } else {
            break;
          }
        }
        if (foundIdx != -1) {
          selectedSliceIndex = foundIdx;
          if (onSliceSelected)
            onSliceSelected(selectedSliceIndex);
          repaint();
        }
      }
    }
  }
}

void WaveformComponent::mouseDrag(const juce::MouseEvent &e) {
  if (isPanning) {
    auto deltaX = e.getPosition().x - lastPanMousePos.x;
    lastPanMousePos = e.getPosition();

    auto waveformArea = getScreenBounds();
    double offsetDelta = (double)deltaX / waveformArea.getWidth();
    scrollOffset -= offsetDelta;
    scrollOffset =
        juce::jlimit(0.0, juce::jmax(0.0, zoomRatio - 1.0), scrollOffset);
    repaint();
    return;
  }

  if (draggingMarkerIndex > 0) {
    auto waveformArea = getScreenBounds();
    auto &sample = audioProcessor.samples[audioProcessor.currentTab];
    int numSamples = sample.buffer.getNumSamples();

    int newPos = xToSample(e.getPosition().x, numSamples,
                           waveformArea.getWidth(), waveformArea.getX());
    int zcPos = audioProcessor.findNearestZeroCrossing(audioProcessor.currentTab, newPos);
    zcPos = juce::jlimit(0, numSamples - 1, zcPos);

    int minPos = sample.markers[draggingMarkerIndex - 1].sampleIndex + 1;
    int maxPos = (draggingMarkerIndex < sample.markers.size() - 1)
                     ? sample.markers[draggingMarkerIndex + 1].sampleIndex - 1
                     : sample.buffer.getNumSamples() - 1;

    sample.markers[draggingMarkerIndex].sampleIndex =
        juce::jlimit(minPos, maxPos, zcPos);
    repaint();
  }
}

void WaveformComponent::mouseUp(const juce::MouseEvent &e) {
  isPanning = false;
  draggingMarkerIndex = -1;
  setMouseCursor(juce::MouseCursor::NormalCursor);
}

void WaveformComponent::mouseDoubleClick(const juce::MouseEvent &e) {
  auto waveformArea = getScreenBounds();
  if (waveformArea.contains(e.getPosition())) {
    auto &sample = audioProcessor.samples[audioProcessor.currentTab];
    if (sample.isLoaded) {
      int numSamples = sample.buffer.getNumSamples();
      int rawSamplePos = xToSample(e.getPosition().x, numSamples,
                                   waveformArea.getWidth(), waveformArea.getX());
      int samplePos = audioProcessor.findNearestZeroCrossing(audioProcessor.currentTab, rawSamplePos);

      int clickedMarker = -1;
      for (int i = 0; i < sample.markers.size(); ++i) {
        float mx = sampleToX(sample.markers[i].sampleIndex, numSamples,
                             waveformArea.getWidth(), waveformArea.getX());
        if (std::abs(e.getPosition().x - mx) <= 5.0f) {
          clickedMarker = i;
          break;
        }
      }

      if (clickedMarker != -1) {
        sample.markers.erase(sample.markers.begin() + clickedMarker);
        if (selectedSliceIndex == clickedMarker)
          selectedSliceIndex = -1;
        else if (selectedSliceIndex > clickedMarker)
          selectedSliceIndex--;

        if (onSliceSelected)
          onSliceSelected(selectedSliceIndex);
      } else {
        SliceMarker newMarker;
        newMarker.sampleIndex = samplePos;
        int foundIdx = -1;
        for (int i = 0; i < sample.markers.size(); ++i) {
          if (sample.markers[i].sampleIndex <= samplePos) {
            foundIdx = i;
          }
        }
        if (foundIdx != -1) {
          newMarker.params = sample.markers[foundIdx].params;
        }
        sample.markers.push_back(newMarker);

        std::sort(sample.markers.begin(), sample.markers.end(),
                  [](const SliceMarker &a, const SliceMarker &b) {
                    return a.sampleIndex < b.sampleIndex;
                  });

        for (int i = 0; i < sample.markers.size(); ++i) {
          if (sample.markers[i].sampleIndex == samplePos) {
            selectedSliceIndex = i;
            break;
          }
        }

        if (onSliceSelected)
          onSliceSelected(selectedSliceIndex);
      }
      repaint();
    }
  }
}

void WaveformComponent::mouseMove(const juce::MouseEvent &e) {
  auto waveformArea = getScreenBounds();
  if (waveformArea.contains(e.getPosition())) {
    auto &sample = audioProcessor.samples[audioProcessor.currentTab];
    if (sample.isLoaded && sample.markers.size() > 0) {
      int numSamples = sample.buffer.getNumSamples();
      bool nearTopHandle = false;
      bool nearMarkerLine = false;

      for (int i = 1; i < sample.markers.size(); ++i) {
        float mx = sampleToX(sample.markers[i].sampleIndex, numSamples,
                             waveformArea.getWidth(), waveformArea.getX());
        if (std::abs(e.getPosition().x - mx) <= 8.0f) {
          if (e.getPosition().y <= (waveformArea.getY() + 25)) {
            nearTopHandle = true;
          } else {
            nearMarkerLine = true;
          }
          break;
        }
      }

      if (nearTopHandle) {
        setMouseCursor(juce::MouseCursor::LeftRightResizeCursor);
      } else if (nearMarkerLine) {
        setMouseCursor(juce::MouseCursor::PointingHandCursor);
      } else {
        setMouseCursor(juce::MouseCursor::NormalCursor);
      }
      return;
    }
  }
  setMouseCursor(juce::MouseCursor::NormalCursor);
}
