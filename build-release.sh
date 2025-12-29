#!/bin/bash

# Make Bassline - Release Build Script
# Creates distribution-ready packages for macOS

set -e  # Exit on error

VERSION="1.0.0"
BUILD_DIR="build"
DIST_DIR="dist/MakeBassline-v${VERSION}"

echo "========================================="
echo "Make Bassline v${VERSION} - Release Build"
echo "========================================="

# Clean previous build
echo "Cleaning previous build..."
rm -rf $BUILD_DIR
rm -rf dist

# Configure and build Release version
echo "Configuring CMake..."
/opt/homebrew/bin/cmake -B $BUILD_DIR -DCMAKE_BUILD_TYPE=Release

echo "Building plugins..."
/opt/homebrew/bin/cmake --build $BUILD_DIR --config Release

# Wait for build to complete
if [ ! -f "$BUILD_DIR/MakeBassline_artefacts/Release/VST3/Make Bassline.vst3/Contents/MacOS/Make Bassline" ]; then
    echo "ERROR: Build failed - VST3 not found"
    exit 1
fi

echo "Build successful!"

# Create distribution directory
echo "Creating distribution package..."
mkdir -p "$DIST_DIR"

# Copy plugins from build output
cp -R "$BUILD_DIR/MakeBassline_artefacts/Release/VST3/Make Bassline.vst3" "$DIST_DIR/"
cp -R "$BUILD_DIR/MakeBassline_artefacts/Release/AU/Make Bassline.component" "$DIST_DIR/"
cp -R "$BUILD_DIR/MakeBassline_artefacts/Release/CLAP/Make Bassline.clap" "$DIST_DIR/"

# Copy standalone app (optional)
if [ -d "$BUILD_DIR/MakeBassline_artefacts/Release/Standalone/Make Bassline.app" ]; then
    cp -R "$BUILD_DIR/MakeBassline_artefacts/Release/Standalone/Make Bassline.app" "$DIST_DIR/"
fi

# Copy documentation
cp README.md "$DIST_DIR/" 2>/dev/null || echo "No README.md found"
cp LICENSE "$DIST_DIR/" 2>/dev/null || echo "No LICENSE found"

# Create installation instructions
cat > "$DIST_DIR/INSTALL.txt" << 'EOF'
Make Bassline Installation Instructions
========================================

AUTOMATIC INSTALLATION (Recommended):
--------------------------------------
Run the install-plugins.sh script included in this package.


MANUAL INSTALLATION:
--------------------

macOS:
1. Copy the plugin files to these locations:

   AU Plugin:
   Make Bassline.component → /Library/Audio/Plug-Ins/Components/

   VST3 Plugin:
   Make Bassline.vst3 → /Library/Audio/Plug-Ins/VST3/

   CLAP Plugin:
   Make Bassline.clap → /Library/Audio/Plug-Ins/CLAP/

2. Restart your DAW

3. Look for "Make Bassline" in your DAW's MIDI FX or instrument list


TROUBLESHOOTING:
----------------
- macOS "damaged" warning: Right-click plugin → Open, then confirm
- Not showing in DAW: Rescan plugins in DAW preferences
- Still issues: Check that your DAW supports MIDI FX plugins


SYSTEM REQUIREMENTS:
--------------------
- macOS 10.13 or later (Intel or Apple Silicon)
- Compatible DAW (Logic Pro, Ableton Live, FL Studio, etc.)


SUPPORT:
--------
GitHub: https://github.com/yourusername/make-bassline
Issues: https://github.com/yourusername/make-bassline/issues
EOF

# Create installer script
cat > "$DIST_DIR/install-plugins.sh" << 'EOF'
#!/bin/bash

echo "Installing Make Bassline plugins..."

# Create directories if they don't exist
mkdir -p "/Library/Audio/Plug-Ins/Components"
mkdir -p "/Library/Audio/Plug-Ins/VST3"
mkdir -p "/Library/Audio/Plug-Ins/CLAP"

# Copy plugins
if [ -d "Make Bassline.component" ]; then
    echo "Installing AU plugin..."
    cp -R "Make Bassline.component" "/Library/Audio/Plug-Ins/Components/"
fi

if [ -d "Make Bassline.vst3" ]; then
    echo "Installing VST3 plugin..."
    cp -R "Make Bassline.vst3" "/Library/Audio/Plug-Ins/VST3/"
fi

if [ -d "Make Bassline.clap" ]; then
    echo "Installing CLAP plugin..."
    cp -R "Make Bassline.clap" "/Library/Audio/Plug-Ins/CLAP/"
fi

echo ""
echo "Installation complete!"
echo "Please restart your DAW and rescan plugins."
echo ""
EOF

chmod +x "$DIST_DIR/install-plugins.sh"

# Create ZIP archive
echo "Creating ZIP archive..."
cd dist
zip -r "MakeBassline-v${VERSION}-macOS.zip" "MakeBassline-v${VERSION}" -x "*.DS_Store"
cd ..

echo ""
echo "========================================="
echo "SUCCESS!"
echo "========================================="
echo "Distribution package created:"
echo "  dist/MakeBassline-v${VERSION}-macOS.zip"
echo ""
echo "Package contents:"
ls -lh "$DIST_DIR"
echo ""
echo "Package size:"
du -sh "$DIST_DIR"
echo ""
echo "ZIP size:"
du -sh "dist/MakeBassline-v${VERSION}-macOS.zip"
echo ""
echo "Next steps:"
echo "1. Test installation on clean machine"
echo "2. Run: pluginval --validate (if installed)"
echo "3. Upload to GitHub releases or distribution platform"
echo ""
echo "For code signing & notarization, see DISTRIBUTION.md"
echo "========================================="
