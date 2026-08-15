# ChopSamp — User Manual & Reference Guide
**Developed by Ankur Rabha**  
*Version 1.0.0 | VST3 & Standalone 64-Bit*

---

## Table of Contents
1. [Introduction & Overview](#1-introduction--overview)
2. [Installation & Setup](#2-installation--setup)
3. [User Interface Walkthrough](#3-user-interface-walkthrough)
   - [Top Header Bar](#top-header-bar)
   - [Waveform LCD Display](#waveform-lcd-display)
   - [Sample Slicing Engine](#sample-slicing-engine)
   - [Sound Sculpting & Per-Slice Parameters](#sound-sculpting--per-slice-parameters)
   - [ADSR Envelope Visualizer & Controls](#adsr-envelope-visualizer--controls)
   - [20-Slot Multi-Sample Matrix](#20-slot-multi-sample-matrix)
   - [DAW Drag & Drop Integration (MIDI & Audio)](#daw-drag--drop-integration-midi--audio)
   - [Live Audio Recording](#live-audio-recording)
   - [Master Output & VU Meter](#master-output--vu-meter)
4. [MIDI Key Mapping Modes](#4-midi-key-mapping-modes)
5. [Keyboard & Mouse Shortcuts](#5-keyboard--mouse-shortcuts)
6. [Troubleshooting & FAQ](#6-troubleshooting--faq)

---

## 1. Introduction & Overview

**ChopSamp** is a professional, tactile sampler and slicer plugin designed for beatmakers, music producers, and sound designers. Combining the vintage tactile aesthetic of classic hardware samplers (MPC, Digitakt, SP-404) with the blazing speed of modern DAW workflows, ChopSamp allows you to slice, manipulate, filter, record, and export samples seamlessly.

### Key Highlights:
- **Zero-Crossing Snapped Slicing**: Transient, Grid (1/2 to 1/16 Triplet), Random, and Manual slicing.
- **20 Independent Sample Tabs**: Load, chop, and manage up to 20 full audio tracks simultaneously.
- **Instant DAW Drag & Drop**: Drag generated slice MIDI sequences (`.mid`) or individual chopped audio clips (`.wav`) straight onto your DAW timeline.
- **Hardware-Inspired Sound Engine**: Monophonic MPC choke groups with anti-click crossfades, per-slice Pitch, Pan, Volume, HPF, and LPF.
- **Live DAW Audio Recording**: Sample directly from any track or microphone input inside your DAW.

---

## 2. Installation & Setup

### File Locations:
- **VST3 Plugin Path**:  
  `C:\Program Files\Common Files\VST3\ChopSamp.vst3`
- **Standalone App**:  
  `d:\VSCode\chopsamp\build\ChopSamp_artefacts\Debug\Standalone\ChopSamp.exe`

### Adding to Your DAW:
1. Open your DAW (FL Studio, Reaper, Ableton Live, Cubase, Studio One, Logic, etc.).
2. Go to your DAW's **Plugin Manager** and click **Rescan Plugins**.
3. Insert **ChopSamp** as an Instrument track (`VST3i: ChopSamp (Ankur Rabha)`).

---

## 3. User Interface Walkthrough

```
+-------------------------------------------------------------------------------+
|  (LED) Playing                                     [✓] Helper   [GUI Scale 100%]|
+-------------------------------------------------------------------------------+
|  +-------------------------------------------------------------------------+  |
|  |  WAVEFORM LCD DISPLAY SCREEN & SLICE MARKERS                            |  |
|  |  [Sample.wav] [16bit] [44.1kHz]                             [0:04.23s]  |  |
|  |  ======================== ChopSamp (by Ankur) ========================  |  |
|  +-------------------------------------------------------------------------+  |
|                                                                               |
|  [XFade] [Start] [End] [REV] [Color]     [=== ADSR GRAPH ===]    [Slice Mode] |
|  [Vol]   [Pan]   [Pitch] [HPF] [LPF]     [  APPLY TO ALL    ]    [Grid / Sens]|
|                                          [Atk] [Dec] [Sus] [Rel] [  SLICE!  ] |
|  +---------------------------------+                             [DRAG MIDI ] |
|  | 20 SAMPLE SLOTS MATRIX (1 - 20) |     [PB Range] [Root Key]   [DRAG AUDIO] |
|  +---------------------------------+                             [Play Mode ] |
|                                                                  [Key Map   ] |
|                                                                  (REC) [M.Vol]|
+-------------------------------------------------------------------------------+
```

### Top Header Bar
- **Active Trigger LED**: Default dark; illuminates neon green dynamically whenever any slice or MIDI note is triggered.
- **Helper Checkbox (`[✓] Helper`)**: Turns hover tooltips on or off across the entire interface.
- **GUI Scaling Dropdown**: Instantly scales the entire plugin window between `50%`, `75%`, `100%`, `125%`, and `150%` to fit 1080p, 2K, and 4K displays.

---

### Waveform LCD Display
- **Dual-Channel Visualization**: Real-time interactive waveform rendering with high-contrast slice markers.
- **Mouse Navigation**:
  - **Zoom**: Scroll wheel up/down (anchored directly to mouse cursor location).
  - **Pan / Scroll**: Right-click and drag horizontally across the zoomed waveform.
  - **Add Slice Marker**: Double-click anywhere on the waveform.
  - **Delete Slice Marker**: Double-click directly on an existing slice marker line.
  - **Move Marker**: Hover cursor over the top marker handle (cursor turns into `↔`) and drag left/right. All movements snap automatically to zero crossings to prevent clicks.
- **Metadata LCD Strip**: Displays active file name, bit depth, sample rate, file format, and total sample duration in seconds.

---

### Sample Slicing Engine
Located on the right panel:
1. **Slice Grid**: Slices audio evenly into musical note lengths:
   - `1/2 Note`, `1/4 Note`, `1/4 Triplet`, `1/8 Note`, `1/8 Triplet`, `1/16 Note`.
2. **Slice Transients**: Intelligent peak detection algorithm. Use the **Sensitivity** slider (`0%` to `100%`) to control detection threshold.
3. **Slice Random**: Procedural algorithmic slicing. Set the slice count slider (`2` to `32` slices).
4. **Clear Slices**: Removes all slice markers from the current sample.
5. **SLICE Button**: Executes the selected slicing algorithm.

---

### Sound Sculpting & Per-Slice Parameters
Each individual slice retains independent sound-shaping settings:
- **XFade (Crossfade)**: `0.0ms` to `50.0ms` micro-fade preventing clicks when choking voices.
- **Start Trim & End Trim**: Fine offset trimming for the active slice's start and end boundaries.
- **Reverse Button (`REV`)**: Toggles reverse audio playback for the active slice.
- **Color Theme Dropdown**: Switches visual theme palette (Neon, MPC Retro Amber, Cyberpunk Violet, Minimalist Slate).
- **Vol**: `-inf` to `+6.0 dB` per-slice volume gain.
- **Pan**: `100% Left` to `100% Right` stereo positioning.
- **Pitch**: `-24` to `+24` semitones (±2 octaves) pitch shifting.
- **HPF (High-Pass Filter)**: `20 Hz` to `2,000 Hz` low-end cutoff.
- **LPF (Low-Pass Filter)**: `200 Hz` to `20,000 Hz` high-end cutoff.

---

### ADSR Envelope Visualizer & Controls
- **Interactive Oscilloscope Display**: Visualizes the amplitude envelope curve against oscilloscope grid lines.
- **Knobs**:
  - **Attack (`0ms` to `1000ms`)**: Controls initial fade-in time.
  - **Decay (`0ms` to `2000ms`)**: Controls time taken to drop from peak to sustain level.
  - **Sustain (`0.0` to `1.0`)**: Steady volume level held while key is pressed.
  - **Release (`10ms` to `5000ms`)**: Fade-out time after key is released.
- **APPLY TO ALL Button**: Single click to copy current slice's ADSR envelope to all other slices in the active sample.
- **PB Range**: Configures pitch bend wheel range (`±2`, `±7`, `±12`, `±24` semitones).
- **Root Key**: Sets starting trigger note (`C1`, `C2`, `C3`, `C4`, `C5`).

---

### 20-Slot Multi-Sample Matrix
- 20 numbered tactile slot buttons (`1` to `20`).
- Each slot acts as an independent sampler engine holding its own file, slices, and parameter settings.
- Simply click a tab number to switch between loaded samples instantly during performance.
- Drag and drop any `.wav`, `.mp3`, `.flac`, `.aiff`, or `.ogg` file onto the window to load it into the active slot.

---

### DAW Drag & Drop Integration (MIDI & Audio)
- **DRAG MIDI**:
  - Click and drag the **DRAG MIDI** button onto your DAW playlist or piano roll.
  - Generates a `.mid` pattern file triggering all your slices sequentially in rhythm, mapped to your active Key Map mode and Root Note.
- **DRAG AUDIO**:
  - Click and drag the **DRAG AUDIO** button onto your DAW playlist or sampler channel.
  - Exports the isolated `.wav` audio clip of the currently selected slice.

---

### Live Audio Recording
1. Route live audio or a microphone track into ChopSamp's track in your DAW.
2. Click the circular **Record** button (glows red, and top banner shows `REC AUDIO FROM DAW...`).
3. Play or stream audio from your DAW. The LCD screen visualizes the live incoming waveform.
4. Click **Record** again to stop. ChopSamp immediately maps the recorded audio into the active slot and auto-chops it by grid.

---

### Master Output & VU Meter
- **Master Volume**: Global output gain knob (`-inf` to `+6.0 dB`).
- **Master VU Peak Meter**: Real-time tri-color LED level meter (Green = Safe, Yellow = Warm, Red = Peaking/Clipping).

---

## 4. MIDI Key Mapping Modes

Toggle between mapping modes using the **Key Map** button:

| Mode | Trigger Layout | Best Used For |
| :--- | :--- | :--- |
| **White Keys Only** (Default) | Triggers slices on white natural piano keys (`C, D, E, F, G, A, B`) starting from Root Key. | Playing slices like MPC pads on standard keyboards without awkward finger jumps across black keys. |
| **Chromatic** | Triggers slices sequentially across every semitone key (`C, C#, D, D#...`). | Standard linear MIDI controller pad layouts & keyboard mapping. |

---

## 5. Keyboard & Mouse Shortcuts

| Action | Control |
| :--- | :--- |
| **Add Marker** | Double-Click on waveform |
| **Delete Marker** | Double-Click on marker line |
| **Move Marker Handle** | Click & drag top handle `↔` (snaps to Zero Crossings) |
| **Zoom Waveform** | Mouse Scroll Wheel (up = zoom in, down = zoom out) |
| **Pan / Scroll Waveform** | Right-Click + Drag horizontally |
| **Drag MIDI to DAW** | Click & Drag **DRAG MIDI** button to DAW track |
| **Drag Slice Audio to DAW** | Click & Drag **DRAG AUDIO** button to DAW track |
| **Load Sample File** | Drag & drop audio file anywhere on plugin window |

---

## 6. Troubleshooting & FAQ

**Q: My DAW displays two names for ChopSamp in the plugin list.**  
*A: Rescan your plugins in your DAW (or clear plugin cache). The plugin is registered under `ChopSamp` by `Ankur Rabha`.*

**Q: Slices click or pop when retriggered rapidly.**  
*A: Increase the **XFade** knob (e.g. 5ms to 15ms) to apply smooth anti-click voice crossfading.*

**Q: Drag MIDI isn't placing notes in FL Studio / Reaper.**  
*A: Ensure you click and drag your mouse cursor out of the plugin window before releasing.*

---
*© 2026 Ankur Rabha. All rights reserved.*
