<div align="center">

# 🎛️ ChopSamp
### *Tactile Hardware-Inspired Sampler & Slicer Audio Plugin*

<img src="assets/chopsamp-logo.png" width="300" alt="ChopSamp Logo">

[![License: GPL v3](https://img.shields.io/badge/License-GPLv3-blue.svg)](https://www.gnu.org/licenses/gpl-3.0)
[![JUCE Framework](https://img.shields.io/badge/JUCE-v8.0.4-orange.svg)](https://juce.com/)
[![C++ Standard](https://img.shields.io/badge/C%2B%2B-20-purple.svg)](https://en.wikipedia.org/wiki/C%2B%2B20)
[![Platform](https://img.shields.io/badge/Platform-Windows%20x64-brightgreen.svg)]()
[![Format](https://img.shields.io/badge/Format-VST3%20%7C%20Standalone-red.svg)]()

**Developed by Ankur Rabha**

<br/>

> **ChopSamp** is an open-source, retro hardware-inspired audio sampler and slicer plugin (VST3 / Standalone 64-Bit). It bridges the tactile workflow of iconic hardware grooveboxes (MPC, Digitakt, SP-[...]

</div>

---

## 🎨 User Interface

<div align="center">
  <img src="assets/chopsamp-ui.png" width="100%" alt="ChopSamp UI Screenshot">
</div>

---

## ⚡ Features at a Glance

- **🎯 Zero-Crossing Snapped Slicing**:
  - Auto-slicing engines: **Grid** (1/2 to 1/16 Triplet), **Transients** (dynamic peak detector with sensitivity slider), and **Random** (2 to 32 slices).
  - Manual slicing: Double-click to add or remove markers; drag top handles (`↔`) with automated zero-crossing detection for click-free playback.
- **💾 20-Slot Multi-Sample Matrix**:
  - Load and manage up to 20 independent audio samples simultaneously with dedicated sample buffers, markers, and parameters.
- **🚀 Seamless DAW Drag & Drop**:
  - **Drag MIDI**: Drag your rhythmically sequenced slice pattern directly onto your DAW's timeline or piano roll as a `.mid` file.
  - **Drag Audio**: Drag individual isolated slice waveforms directly onto your arrangement playlist as a `.wav` file.
- **🎙️ Real-Time DAW Audio Recording**:
  - Route audio from any DAW track or microphone input, arm Record, monitor live oscilloscope waveform rendering, and auto-chop on stop.
- **🎚️ Sound Sculpting & Voice Choking**:
  - Per-slice **Pitch** (±24 semitones / 4 octaves), **HPF** (20Hz–2kHz), **LPF** (200Hz–20kHz), **Pan**, **Volume**, and **Reverse (`REV`)**.
  - **Mono Choke Mode** with customizable **XFade** (0–50ms anti-click crossfade) for authentic hip-hop sample bounce.
- **📈 Real-Time ADSR Envelope Oscilloscope**:
  - Pure knob-driven Attack, Decay, Sustain, and Release controls with live curve oscilloscope visualizer and 1-click **Apply to All** feature.
- **🎹 Intuitive Key Mapping**:
  - **White Keys Only Mode**: Maps slices directly to natural piano keys (`C, D, E, F, G, A, B`) starting from your chosen Root Key for effortless MPC pad-style playing.
  - **Chromatic Mode**: Maps slices sequentially across all semitones.
- **🖥️ Modern Adaptive Interface**:
  - GUI scaling dropdown (`50%` to `150%`) with high-DPI support.
  - Optional `[✓] Helper` toggle to enable/disable descriptive hover tooltips.

---

## 📥 Installation

### Windows (VST3 & Standalone)
1. Download the latest binary release from the [Releases](https://github.com/) page.
2. Copy `ChopSamp.vst3` into your system's VST3 directory:
   ```
   C:\Program Files\Common Files\VST3\
   ```
3. Open your DAW (FL Studio, Ableton Live, Reaper, Cubase, Studio One, Bitwig, etc.), rescan your plugins, and insert **ChopSamp** (`VST3i: ChopSamp (Ankur Rabha)`).

---

## 🛠️ Building from Source

### Prerequisites
- **CMake** (v3.22 or higher)
- **Visual Studio 2022** (MSVC C++ v143 or higher with *Desktop development with C++*)
- **Git**

### Build Steps

1. Clone the repository:
   ```bash
   git clone https://github.com/your-username/chopsamp.git
   cd chopsamp
   ```

2. Generate the build configuration (JUCE will be fetched automatically via CMake FetchContent):
   ```bash
   cmake -B build -G "Visual Studio 17 2022" -A x64
   ```

3. Compile the Release VST3 and Standalone binaries:
   ```bash
   cmake --build build --config Release
   ```

4. Built binaries will be available in:
   - **VST3 Plugin**: `build/ChopSamp_artefacts/Release/VST3/ChopSamp.vst3`
   - **Standalone App**: `build/ChopSamp_artefacts/Release/Standalone/ChopSamp.exe`

---

## 📖 Documentation

- Full Reference Manual: [USER_MANUAL.md](USER_MANUAL.md)
- Video Showcase & Tutorial Script: [VIDEO_SCRIPT.md](VIDEO_SCRIPT.md)

---

## ⌨️ Mouse & Keyboard Controls

| Action | Control |
| :--- | :--- |
| **Add Marker** | Double-Click on waveform |
| **Delete Marker** | Double-Click on marker line |
| **Move Marker Handle** | Click & drag top handle `↔` (snaps to Zero Crossings) |
| **Zoom Waveform** | Mouse Scroll Wheel (up = zoom in, down = zoom out) |
| **Pan / Scroll Waveform** | Right-Click + Drag horizontally |
| **Drag MIDI to DAW** | Click & Drag **DRAG MIDI** button to DAW playlist |
| **Drag Audio Clip to DAW** | Click & Drag **DRAG AUDIO** button to DAW playlist |
| **Load Sample File** | Drag & drop audio file anywhere on plugin window |

---

## 📄 License

This project is licensed under the **GNU General Public License v3.0 (GPL-3.0)**.  
See the [LICENSE](LICENSE) file for full details.

---

## 👨‍💻 Author & Acknowledgements

Created with ❤️ by **Ankur Rabha**.  
Built with the [JUCE Framework](https://juce.com/).
