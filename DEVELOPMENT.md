# Make Bassline - Development Log

## December 29, 2025 - Production Release and UI Refinement

### Branding Update (v0.0.1)
Successfully rebranded the plugin from "Bassline Generator" to "Make Bassline" for production release.

**Changed:**
- Product name: `MakeBassline` → `Make Bassline`
- Company name: Updated to `Make Bassline`
- Bundle ID: `com.makebassline.makebassline`
- Plugin manufacturer code: `MkBs`
- Plugin code: `MkBl`
- UI title: `MAKE BASSLINE` with comic-book styling

### Visual Design Overhaul
Implemented a cohesive yellow/red/black color scheme throughout the entire UI.

**Color Palette:**
- Primary highlight: Bright yellow `#FFDD00`
- Primary accent: Red `#FF0000`
- Background: Black to dark red gradient `#000000` → `#1A0000`
- Secondary accent: Orange `#FF8800`

**Updated Components:**
- `ComicBookLookAndFeel.h` - Custom JUCE LookAndFeel with comic-book styling
  - Rotary sliders with red-to-yellow gradient
  - Bold black outlines on all controls
  - Yellow highlights and accents
- `StepSequencerGrid.h` - Linear step visualization
  - Yellow active steps, red hits, dark red empty steps
  - Yellow border with black outline
- `MidiDragComponent.h` - MIDI export drag area
  - Updated from cyan/blue to yellow/red/black scheme
  - Comic-book style borders and typography
- `PluginEditor.cpp` - Main editor window
  - Title gradient: Black outline → Red fill → Yellow highlight
  - Background gradient: Black to dark red

### UI Simplification
Streamlined the interface by hiding less important controls, reducing cognitive load.

**Visible Controls (7 essential parameters):**
1. **Steps** - Pattern length (4-16)
2. **Hits** - Pattern density (1-16)
3. **Rotation** - Pattern offset (0-15)
4. **Root Note** - Key selection (C1-C3)
5. **Scale** - Tonality (Minor Pent, Major, Minor, Dorian, Chromatic)
6. **Swing** - Groove feel (0-75%)
7. **Velocity** - Note dynamics (1-127)

**Hidden Controls (5 advanced parameters):**
- Octave Range - Default: 1 octave
- Note Length - Default: 50%
- Humanize - Default: 0 (off)
- Random Seed - Default: 42
- Regenerate Button - Redundant with seed control

**Layout Changes:**
- Reduced from 2 rows to 1 row of controls
- Cleaner, more focused interface
- All essential controls perfectly aligned
- More breathing room in the UI

### MIDI Export Enhancement
Fixed MIDI drag-and-drop functionality to work outside the plugin window.

**Technical Improvements:**
- Replaced `startDragging()` with `performExternalDragDropOfFiles()`
- Added `isDragging` flag to prevent multiple concurrent drags
- Updated temp file name to `MakeBasslinePattern.mid`
- Export area layout cleaned up and centered
- Bar length selector positioned next to drag area

### Build Configuration
- Built and validated Release version
- All plugin formats working: AU, VST3, CLAP, Standalone
- AU validation passed: `auval -v aumi MkBl MkBs` ✅
- Plugin installed to system directories

### File Structure
```
source/
├── PluginProcessor.h/cpp        # Main processor with MIDI generation
├── PluginEditor.h/cpp           # UI editor (streamlined layout)
├── generator/
│   ├── EuclideanRhythm.h       # Bjorklund's algorithm
│   ├── PitchGenerator.h        # Scale-quantized pitch generation
│   └── PatternState.h          # Lock-free UI sync
├── ui/
│   ├── ComicBookLookAndFeel.h  # Yellow/red/black custom styling
│   ├── StepSequencerGrid.h     # Linear step visualizer (updated colors)
│   ├── CircularVisualizer.h    # Circular Euclidean display
│   └── MidiDragComponent.h     # MIDI export (fixed drag, new colors)
└── utils/
    └── MidiPatternExporter.h   # MIDI file generation
```

### Production Status
✅ Production-ready Release v0.0.1
- Branding complete
- Visual design cohesive
- UI streamlined for usability
- MIDI export fully functional
- AU validation passed
- All formats built and installed

---

## Initial Development - Generative Bassline MIDI FX Plugin

### Core Features Implemented
- **Euclidean Rhythm Generation** - Bjorklund's algorithm for musical step patterns
- **Scale-Quantized Pitch** - 5 scale types with deterministic randomization
- **Real-time MIDI Generation** - Synced to DAW tempo and transport
- **Groove Controls** - Swing and humanization for natural feel
- **Pattern Visualization** - Step sequencer grid and circular visualizer
- **MIDI Export** - Drag-and-drop patterns (1, 2, 4, or 8 bars) to DAW

### Plugin Type
- Audio Unit MIDI Effect (aumi)
- For Logic Pro and compatible DAWs
- Generates MIDI, no audio processing

### Parameters (12 total)
All parameters exposed via APVTS for DAW automation:
1. Steps (4-16)
2. Hits (1-16)
3. Rotation (0-15)
4. Root Note (24-48)
5. Scale (0-4: Minor Pent, Major, Minor, Dorian, Chromatic)
6. Octave Range (1-2)
7. Note Length (10-100%)
8. Velocity (1-127)
9. Swing (0-75%)
10. Humanize (0-30)
11. Random Seed (0-9999)
12. Regenerate (trigger)

### Technical Stack
- JUCE 7+ framework
- CMake build system
- Pamplejuce template
- C++20
- Lock-free audio/UI communication with atomics
