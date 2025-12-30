#!/bin/bash

# Quick Package for Beta Testing
# Creates a simple ZIP file from already-built plugins

echo "========================================="
echo "Make Bassline - Quick Beta Package"
echo "========================================="

VERSION="1.0.0-beta"
DIST_DIR="beta-test/MakeBassline-v${VERSION}"

# Create distribution directory
echo "Creating beta package..."
mkdir -p "$DIST_DIR"

# Copy plugins from build directory
echo "Copying plugins from build..."
BUILD_DIR="build/MakeBassline_artefacts/Release"

if [ -d "$BUILD_DIR/VST3/Make Bassline.vst3" ]; then
    cp -R "$BUILD_DIR/VST3/Make Bassline.vst3" "$DIST_DIR/"
    echo "✓ VST3 copied"
else
    echo "✗ VST3 not found - did you build the project?"
fi

if [ -d "$BUILD_DIR/AU/Make Bassline.component" ]; then
    cp -R "$BUILD_DIR/AU/Make Bassline.component" "$DIST_DIR/"
    echo "✓ AU copied"
else
    echo "✗ AU not found - did you build the project?"
fi

if [ -d "$BUILD_DIR/CLAP/Make Bassline.clap" ]; then
    cp -R "$BUILD_DIR/CLAP/Make Bassline.clap" "$DIST_DIR/"
    echo "✓ CLAP copied"
else
    echo "✗ CLAP not found - did you build the project?"
fi

# Create simple installation instructions for beta testers
cat > "$DIST_DIR/HOW_TO_INSTALL.txt" << 'EOF'
MAKE BASSLINE - BETA TEST INSTALLATION
======================================

IMPORTANT: This is a beta version for testing only!

QUICK INSTALL (macOS):
----------------------
1. Run the install.sh script (easiest way)
   - Open Terminal
   - Drag install.sh into Terminal
   - Press Enter

OR MANUAL INSTALL:
------------------
Copy the plugin files to these folders:

VST3:
  Make Bassline.vst3 → /Library/Audio/Plug-Ins/VST3/

AU (for Logic Pro):
  Make Bassline.component → /Library/Audio/Plug-Ins/Components/

CLAP:
  Make Bassline.clap → /Library/Audio/Plug-Ins/CLAP/

AFTER INSTALLING:
-----------------
1. Restart your DAW
2. Rescan plugins in DAW preferences
3. Look for "Make Bassline" in MIDI FX section

USAGE:
------
1. Add Make Bassline as MIDI FX on a track
2. Route MIDI to a synth/bass plugin
3. Adjust knobs to shape your groove
4. Click steps in the sequencer to toggle them
5. Use Randomize for instant variations
6. Drag "Drag to export" area into timeline for MIDI clips

TESTING CHECKLIST:
------------------
Please test:
[ ] Plugin loads without crashing
[ ] All knobs work smoothly
[ ] Pattern plays in sync with DAW
[ ] Clicking sequencer steps toggles them
[ ] Randomize button generates new patterns
[ ] MIDI export works (drag to timeline)
[ ] UI looks good (no graphical glitches)
[ ] Hover effects work on sequencer

REPORT ISSUES:
--------------
If you find bugs or have feedback, let me know:
- What happened?
- What DAW are you using?
- What were you doing when it happened?

KNOWN LIMITATIONS:
------------------
- Beta version - not code signed (you may see warnings)
- If you get "damaged" warning on macOS:
  Right-click plugin → Open
  Or run: xattr -cr "/Library/Audio/Plug-Ins/VST3/Make Bassline.vst3"

Thanks for testing! 🎵
EOF

# Create installer script
cat > "$DIST_DIR/install.sh" << 'EOF'
#!/bin/bash

echo "Installing Make Bassline (Beta)..."
echo ""

# Create directories if needed
sudo mkdir -p "/Library/Audio/Plug-Ins/Components"
sudo mkdir -p "/Library/Audio/Plug-Ins/VST3"
sudo mkdir -p "/Library/Audio/Plug-Ins/CLAP"

# Copy plugins
if [ -d "Make Bassline.component" ]; then
    echo "Installing AU plugin..."
    sudo cp -R "Make Bassline.component" "/Library/Audio/Plug-Ins/Components/"
fi

if [ -d "Make Bassline.vst3" ]; then
    echo "Installing VST3 plugin..."
    sudo cp -R "Make Bassline.vst3" "/Library/Audio/Plug-Ins/VST3/"
fi

if [ -d "Make Bassline.clap" ]; then
    echo "Installing CLAP plugin..."
    sudo cp -R "Make Bassline.clap" "/Library/Audio/Plug-Ins/CLAP/"
fi

echo ""
echo "✅ Installation complete!"
echo ""
echo "Next steps:"
echo "1. Restart your DAW"
echo "2. Rescan plugins"
echo "3. Look for 'Make Bassline' in MIDI FX"
echo ""
EOF

chmod +x "$DIST_DIR/install.sh"

# Create quick feature list
cat > "$DIST_DIR/FEATURES.txt" << 'EOF'
MAKE BASSLINE - FEATURES
========================

CORE FEATURES:
--------------
✨ Euclidean Rhythm Generator - Mathematical groove patterns
🎨 Pop-Art Graffiti UI - Bold, colorful interface
🎹 Interactive Step Sequencer - Click to toggle steps
🎲 Smart Randomization - Instant pattern variations
🎵 MIDI Export - Drag patterns into your DAW
⚡ Real-time Editing - Adjust while playing

PARAMETERS:
-----------
- Steps (4-16): Pattern length
- Density (1-16): How many notes in pattern
- Swing (0-75%): Groove amount
- Root (C1-C3): Starting note
- Scale: Minor Pent, Major, Minor, Dorian, Chromatic

WORKFLOW:
---------
1. Generate algorithmic basslines
2. Customize by clicking sequencer steps
3. Randomize for instant variations
4. Export MIDI clips (1/2/4/8 bars)
5. Layer multiple instances for complex grooves
EOF

# Create ZIP
echo "Creating ZIP file..."
cd beta-test
zip -r "MakeBassline-v${VERSION}-macOS.zip" "MakeBassline-v${VERSION}" -x "*.DS_Store" >/dev/null 2>&1
cd ..

echo ""
echo "========================================="
echo "SUCCESS! Beta package created:"
echo "========================================="
echo "  beta-test/MakeBassline-v${VERSION}-macOS.zip"
echo ""
echo "ZIP size:"
du -sh "beta-test/MakeBassline-v${VERSION}-macOS.zip"
echo ""
echo "HOW TO SEND TO YOUR FRIEND:"
echo "========================================="
echo ""
echo "Option 1 - AirDrop (easiest):"
echo "  - Right-click the ZIP file"
echo "  - Share → AirDrop"
echo "  - Select your friend's device"
echo ""
echo "Option 2 - Cloud Services:"
echo "  - Upload ZIP to Dropbox/Google Drive/iCloud"
echo "  - Send them the share link"
echo ""
echo "Option 3 - Email (if under 25MB):"
echo "  - Attach ZIP to email"
echo "  - Send to your friend"
echo ""
echo "Option 4 - WeTransfer (free, up to 2GB):"
echo "  - Go to wetransfer.com"
echo "  - Upload ZIP"
echo "  - Enter friend's email"
echo "  - Send"
echo ""
echo "TELL YOUR FRIEND:"
echo "  1. Download and unzip the file"
echo "  2. Read HOW_TO_INSTALL.txt"
echo "  3. Run install.sh OR manually copy plugins"
echo "  4. Restart DAW and test!"
echo ""
echo "========================================="
