# Make Bassline

**A generative bassline MIDI FX plugin for Logic Pro and DAWs**

Make Bassline is an algorithmic MIDI generator that creates instant bassline patterns using Euclidean rhythm generation and scale-quantized pitch. Perfect for electronic music production, live performance, and creative inspiration.

![Make Bassline Plugin](assets/images/make-bassline-screenshot.png)

## Features

### 🎵 Euclidean Rhythm Generation
- Uses Bjorklund's algorithm to create mathematically interesting rhythmic patterns
- Adjust **Steps** (4-16), **Hits** (1-16), and **Rotation** for infinite pattern variations
- Real-time visual feedback with step sequencer grid

### 🎹 Musical Pitch Control
- **Root Note** selection (C1-C3) to set your key
- **5 Scale Types**: Minor Pentatonic, Major, Minor, Dorian, Chromatic
- Intelligent pitch generation with 70% probability of root note on first step
- Deterministic randomization for reproducible patterns

### 🎛️ Groove & Feel
- **Swing** (0-75%) for that classic shuffle feel
- **Velocity** control (1-127) for dynamics
- Clean, focused interface with 7 essential controls

### 📤 MIDI Export
- Drag-and-drop MIDI patterns directly into your DAW
- Export 1, 2, 4, or 8 bar patterns
- Includes tempo and time signature metadata

### 🎨 Comic-Book UI
- Bold yellow/red/black color scheme
- Inspired by retro comic book aesthetics
- Custom JUCE LookAndFeel with thick outlines and gradients

## Installation

### macOS
The plugin is automatically installed to:
- **Audio Unit**: `~/Library/Audio/Plug-Ins/Components/Make Bassline.component`
- **VST3**: `~/Library/Audio/Plug-Ins/VST3/Make Bassline.vst3`
- **CLAP**: `~/Library/Audio/Plug-Ins/CLAP/Make Bassline.clap`

### Standalone App
Launch the standalone version from:
`build/MakeBassline_artefacts/Release/Standalone/Make Bassline.app`

## Building from Source

### Prerequisites
- CMake 3.25 or higher
- Xcode (macOS)
- JUCE 7+ (included as submodule)

### Build Steps
```bash
# Clone the repository
git clone https://github.com/YOUR_USERNAME/make-bassline.git
cd make-bassline

# Initialize JUCE submodule
git submodule update --init --recursive

# Build Release version
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```

### Validate Audio Unit
```bash
auval -v aumi MkBl MkBs
```

## Usage

### In Your DAW
1. Insert **Make Bassline** as a MIDI FX on a track
2. Route the output to your favorite bass synth
3. Press play and adjust the parameters in real-time
4. All parameters are automatable via your DAW

### Controls

| Control | Range | Description |
|---------|-------|-------------|
| **Steps** | 4-16 | Total steps in the pattern |
| **Hits** | 1-16 | Number of active notes |
| **Rotation** | 0-15 | Shifts the pattern timing |
| **Root Note** | C1-C3 | Sets the key center |
| **Scale** | 5 types | Musical scale for pitch |
| **Swing** | 0-75% | Groove feel (delays even steps) |
| **Velocity** | 1-127 | Note dynamics |

### Hidden Advanced Parameters
These parameters work under the hood with sensible defaults:
- Octave Range (default: 1)
- Note Length (default: 50%)
- Humanize (default: 0)
- Random Seed (default: 42)

## Technical Details

### Plugin Type
- **Audio Unit MIDI Effect** (aumi)
- No audio processing - generates MIDI only
- Syncs to host DAW tempo and transport

### Architecture
- **Euclidean Rhythm**: Bjorklund's algorithm implementation
- **Pitch Generation**: Deterministic random with scale quantization
- **Timing**: Sample-accurate MIDI generation with swing
- **UI Sync**: Lock-free atomic variables for thread safety

### File Structure
```
source/
├── PluginProcessor.h/cpp        # Main MIDI generation
├── PluginEditor.h/cpp           # UI and controls
├── generator/
│   ├── EuclideanRhythm.h       # Bjorklund's algorithm
│   ├── PitchGenerator.h        # Scale-based pitch
│   └── PatternState.h          # Thread-safe state
├── ui/
│   ├── ComicBookLookAndFeel.h  # Custom styling
│   ├── StepSequencerGrid.h     # Pattern visualizer
│   └── MidiDragComponent.h     # MIDI export
└── utils/
    └── MidiPatternExporter.h   # File generation
```

## Development

See [DEVELOPMENT.md](DEVELOPMENT.md) for detailed development log and changelog.

### Running Tests
```bash
cmake --build build --target Tests
./build/MakeBassline_artefacts/Release/Tests
```

### Code Formatting
```bash
clang-format -i source/**/*.{h,cpp}
```

## Credits

Built with:
- [JUCE](https://juce.com/) - Audio plugin framework
- [Pamplejuce](https://github.com/sudara/pamplejuce) - CMake template
- [Melatonin Inspector](https://github.com/sudara/melatonin_inspector) - UI debugging

Inspired by Euclidean rhythm research by Godfried Toussaint.

## License

See [LICENSE](LICENSE) for details.

## Version

Current version: **0.0.1** (Production Release)

---

**Make Bassline** - Instant algorithmic basslines for your productions
