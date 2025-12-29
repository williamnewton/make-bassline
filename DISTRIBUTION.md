# Make Bassline - Distribution Guide

## Overview
This guide covers packaging Make Bassline for professional distribution on macOS and Windows.

## Current Build Artifacts

After building, you have these formats:
- **AU** (Audio Unit): `/Library/Audio/Plug-Ins/Components/Make Bassline.component`
- **VST3**: `/Library/Audio/Plug-Ins/VST3/Make Bassline.vst3`
- **CLAP**: `/Library/Audio/Plug-Ins/CLAP/Make Bassline.clap`
- **Standalone**: `build/MakeBassline_artefacts/Release/Standalone/Make Bassline.app`

## Step 1: Code Signing (macOS)

### Why Code Sign?
- Required for macOS Gatekeeper
- Prevents "unidentified developer" warnings
- Essential for professional distribution

### Get Apple Developer Account
1. Enroll at https://developer.apple.com ($99/year)
2. Download signing certificates from Apple Developer portal
3. Install certificates in Keychain Access

### Sign Your Plugins
```bash
# Sign AU plugin
codesign --force --deep --sign "Developer ID Application: Your Name (TEAM_ID)" \
  "/Library/Audio/Plug-Ins/Components/Make Bassline.component"

# Sign VST3 plugin
codesign --force --deep --sign "Developer ID Application: Your Name (TEAM_ID)" \
  "/Library/Audio/Plug-Ins/VST3/Make Bassline.vst3"

# Sign CLAP plugin
codesign --force --deep --sign "Developer ID Application: Your Name (TEAM_ID)" \
  "/Library/Audio/Plug-Ins/CLAP/Make Bassline.clap"

# Sign Standalone app
codesign --force --deep --sign "Developer ID Application: Your Name (TEAM_ID)" \
  "build/MakeBassline_artefacts/Release/Standalone/Make Bassline.app"

# Verify signatures
codesign --verify --deep --strict "/Library/Audio/Plug-Ins/VST3/Make Bassline.vst3"
```

### Notarize for macOS (Required for macOS 10.15+)
```bash
# Create a zip for notarization
ditto -c -k --keepParent "/Library/Audio/Plug-Ins/VST3/Make Bassline.vst3" MakeBassline-VST3.zip

# Submit for notarization (requires app-specific password)
xcrun notarytool submit MakeBassline-VST3.zip \
  --apple-id "your@email.com" \
  --password "app-specific-password" \
  --team-id "TEAM_ID" \
  --wait

# Staple the notarization ticket
xcrun stapler staple "/Library/Audio/Plug-Ins/VST3/Make Bassline.vst3"
```

## Step 2: Create Professional Installer

### Option A: Simple DMG (macOS) - FREE

Create a drag-and-drop installer:

```bash
# Create staging directory
mkdir -p MakeBassline-Installer
cp -R "/Library/Audio/Plug-Ins/VST3/Make Bassline.vst3" MakeBassline-Installer/
cp -R "/Library/Audio/Plug-Ins/Components/Make Bassline.component" MakeBassline-Installer/
cp -R "/Library/Audio/Plug-Ins/CLAP/Make Bassline.clap" MakeBassline-Installer/
cp README.md MakeBassline-Installer/
cp LICENSE MakeBassline-Installer/

# Create DMG
hdiutil create -volname "Make Bassline Installer" \
  -srcfolder MakeBassline-Installer \
  -ov -format UDZO \
  MakeBassline-v1.0.0-macOS.dmg

# Sign the DMG
codesign --force --sign "Developer ID Application: Your Name (TEAM_ID)" \
  MakeBassline-v1.0.0-macOS.dmg
```

### Option B: Packages Installer (macOS) - FREE

Use the free **Packages** app (http://s.sudre.free.fr/Software/Packages/about.html):

1. Download and install Packages.app
2. Create new project → Distribution
3. Add post-installation scripts to copy plugins to correct locations:
   - AU → `/Library/Audio/Plug-Ins/Components/`
   - VST3 → `/Library/Audio/Plug-Ins/VST3/`
   - CLAP → `/Library/Audio/Plug-Ins/CLAP/`
4. Build → Creates signed `.pkg` installer
5. Sign and notarize the PKG

### Option C: InnoSetup (Windows) - FREE

For Windows distribution, use InnoSetup:

```ini
[Setup]
AppName=Make Bassline
AppVersion=1.0.0
DefaultDirName={commonpf64}\VSTPlugins\Make Bassline
OutputBaseFilename=MakeBassline-v1.0.0-Windows

[Files]
Source: "build\Release\VST3\Make Bassline.vst3"; DestDir: "{commoncf64}\VST3"
Source: "build\Release\CLAP\Make Bassline.clap"; DestDir: "{commoncf64}\CLAP"
Source: "README.md"; DestDir: "{app}"

[Icons]
Name: "{group}\Make Bassline"; Filename: "{app}\README.md"
```

### Option D: PACE iLok (Commercial) - PAID

For copy protection and professional licensing:
- Sign up at https://www.paceap.com
- Integrate PACE SDK into JUCE project
- Supports license management, trials, subscriptions
- Industry-standard for commercial plugins

## Step 3: Plugin Validation

Test your plugin before distribution:

```bash
# Install pluginval (free validation tool)
brew install pluginval

# Test VST3
pluginval --validate "/Library/Audio/Plug-Ins/VST3/Make Bassline.vst3"

# Test AU
pluginval --validate "/Library/Audio/Plug-Ins/Components/Make Bassline.component"

# Run strict tests
pluginval --strictness-level 10 --validate-in-process \
  "/Library/Audio/Plug-Ins/VST3/Make Bassline.vst3"
```

## Step 4: Version Management

Update version in `CMakeLists.txt`:

```cmake
set(PROJECT_VERSION "1.0.0")
```

Use semantic versioning:
- **1.0.0** - Initial release
- **1.0.1** - Bug fixes
- **1.1.0** - New features (backwards compatible)
- **2.0.0** - Breaking changes

## Step 5: Create User Documentation

### Quick Start Guide (README.md)
- Installation instructions
- System requirements
- Quick workflow example
- Parameter descriptions
- Troubleshooting

### User Manual (PDF)
- Detailed feature explanations
- Tutorial examples
- Tips and tricks
- MIDI export workflow
- Screenshots/diagrams

## Step 6: Distribution Platforms

### Free Distribution
1. **GitHub Releases**
   - Tag your release: `git tag v1.0.0`
   - Upload DMG/PKG/ZIP to releases
   - Free hosting, version control

2. **Your Website**
   - Direct download links
   - Landing page with demos
   - Email collection for updates

### Paid Distribution
1. **Plugin Boutique** (https://www.pluginboutique.com)
   - 50/50 revenue split
   - Large customer base
   - Marketing support

2. **Gumroad** (https://gumroad.com)
   - 10% fee (or $10/month for 0% fee)
   - Easy setup
   - Flexible pricing

3. **Your Own Site + Stripe/PayPal**
   - Keep most revenue
   - Requires license management
   - Use PACE iLok or custom system

## Step 7: Cross-Platform Build Script

Create `build-release.sh`:

```bash
#!/bin/bash

VERSION="1.0.0"
BUILD_DIR="build"
DIST_DIR="dist/MakeBassline-v${VERSION}"

# Clean and build
rm -rf $BUILD_DIR
cmake -B $BUILD_DIR -DCMAKE_BUILD_TYPE=Release
cmake --build $BUILD_DIR --config Release

# Create distribution directory
mkdir -p $DIST_DIR

# Copy plugins
cp -R "/Library/Audio/Plug-Ins/VST3/Make Bassline.vst3" $DIST_DIR/
cp -R "/Library/Audio/Plug-Ins/Components/Make Bassline.component" $DIST_DIR/
cp -R "/Library/Audio/Plug-Ins/CLAP/Make Bassline.clap" $DIST_DIR/

# Copy documentation
cp README.md $DIST_DIR/
cp LICENSE $DIST_DIR/

# Sign all plugins (replace with your identity)
# codesign --force --deep --sign "Developer ID Application: Your Name" $DIST_DIR/*.vst3
# codesign --force --deep --sign "Developer ID Application: Your Name" $DIST_DIR/*.component
# codesign --force --deep --sign "Developer ID Application: Your Name" $DIST_DIR/*.clap

# Create DMG
hdiutil create -volname "Make Bassline v${VERSION}" \
  -srcfolder $DIST_DIR \
  -ov -format UDZO \
  "dist/MakeBassline-v${VERSION}-macOS.dmg"

echo "Distribution package created at dist/MakeBassline-v${VERSION}-macOS.dmg"
```

## Recommended Quick Start (Free)

For your first release, I recommend:

1. **Build Release Version**
   ```bash
   cmake -B build -DCMAKE_BUILD_TYPE=Release
   cmake --build build --config Release
   ```

2. **Create Simple ZIP Package**
   ```bash
   mkdir -p dist/MakeBassline-v1.0.0
   cp -R "/Library/Audio/Plug-Ins/VST3/Make Bassline.vst3" dist/MakeBassline-v1.0.0/
   cp -R "/Library/Audio/Plug-Ins/Components/Make Bassline.component" dist/MakeBassline-v1.0.0/
   cp -R "/Library/Audio/Plug-Ins/CLAP/Make Bassline.clap" dist/MakeBassline-v1.0.0/
   cp README.md dist/MakeBassline-v1.0.0/

   cd dist
   zip -r MakeBassline-v1.0.0-macOS.zip MakeBassline-v1.0.0/
   ```

3. **Upload to GitHub Releases**
   - Tag: `v1.0.0`
   - Attach ZIP file
   - Write release notes

4. **Later: Add Code Signing**
   - Get Apple Developer account when ready
   - Sign and notarize for professional release

## System Requirements to Document

**macOS:**
- macOS 10.13 or later
- 64-bit Intel or Apple Silicon
- AU/VST3/CLAP compatible DAW

**Windows:**
- Windows 10 or later
- 64-bit
- VST3/CLAP compatible DAW

**DAW Compatibility:**
- Logic Pro X
- Ableton Live
- FL Studio
- Bitwig Studio
- Reaper
- Any DAW supporting AU/VST3/CLAP MIDI FX

## License Considerations

Current license: (Check your LICENSE file)

Options:
- **Free/Open Source**: MIT, GPL, Apache
- **Freemium**: Free base version + paid pro version
- **Paid**: Single purchase or subscription
- **Donationware**: Free with optional donation

## Support & Updates

Set up:
1. **Email**: support@yourdomain.com
2. **Issue Tracker**: GitHub Issues
3. **Update Notifications**: Email list or website
4. **Changelog**: Keep detailed version history

---

## Quick Checklist Before Release

- [ ] Test in Logic Pro, Ableton, other DAWs
- [ ] Run pluginval validation
- [ ] Update version numbers
- [ ] Write user documentation
- [ ] Create screenshots/demo video
- [ ] Code sign (if distributing commercially)
- [ ] Test installation process
- [ ] Prepare marketing materials
- [ ] Set up support channel
- [ ] Create changelog
- [ ] Tag GitHub release
- [ ] Upload distribution packages
