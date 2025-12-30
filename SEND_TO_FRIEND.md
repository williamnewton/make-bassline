# How to Send Make Bassline to Your Friend for Testing

## ✅ Package Created!

Your beta test package is ready:
```
beta-test/MakeBassline-v1.0.0-beta-macOS.zip (2.0 MB)
```

---

## 📤 How to Send (Pick One Method)

### Option 1: AirDrop (Easiest - if nearby)
1. Right-click **MakeBassline-v1.0.0-beta-macOS.zip**
2. Click **Share → AirDrop**
3. Select your friend's Mac/iPhone
4. Done! ✨

### Option 2: iCloud/Dropbox/Google Drive
1. Upload **MakeBassline-v1.0.0-beta-macOS.zip** to your cloud storage
2. Right-click → Get Share Link
3. Send link to your friend via text/email
4. They download from cloud

### Option 3: WeTransfer (Best for large files)
1. Go to **wetransfer.com**
2. Click **Add files**
3. Upload **MakeBassline-v1.0.0-beta-macOS.zip**
4. Enter friend's email
5. Click **Transfer**
6. They'll receive download link via email

### Option 4: Email (if your email allows 2MB attachments)
1. Compose new email
2. Attach **MakeBassline-v1.0.0-beta-macOS.zip**
3. Send to friend

---

## 📝 What to Tell Your Friend

Copy/paste this message:

```
Hey! I built a bassline generator plugin called Make Bassline.
Can you test it for me?

WHAT IT IS:
- MIDI FX plugin for generating bassline grooves
- Uses Euclidean rhythms (math-based patterns)
- Click sequencer steps to customize patterns
- Works in Logic Pro, Ableton, FL Studio, etc.

HOW TO INSTALL:
1. Download and unzip the file
2. Read "HOW_TO_INSTALL.txt" inside
3. Run install.sh (easiest way)
   OR manually copy plugins to your plugin folders
4. Restart your DAW
5. Look for "Make Bassline" in MIDI FX

HOW TO USE:
1. Add "Make Bassline" as MIDI FX on a track
2. Route MIDI output to a bass synth
3. Play your DAW - you'll hear a pattern
4. Tweak the knobs to change the groove
5. Click steps in the sequencer to toggle notes
6. Hit "Randomize" for instant new patterns
7. Drag "Drag to export" area into timeline for MIDI

WHAT TO TEST:
[ ] Does it load without crashing?
[ ] Do the knobs work smoothly?
[ ] Does clicking sequencer steps work?
[ ] Does the pattern play in sync?
[ ] Does MIDI export work (drag to timeline)?
[ ] Any bugs or weird behavior?

Let me know what you think! 🎵
```

---

## 🐛 Common Issues & Solutions

### "Plugin is damaged" warning on macOS
This is because the plugin isn't code signed. Tell your friend:

**Solution 1 (easiest):**
- Right-click the plugin file
- Click "Open"
- Confirm when asked

**Solution 2 (terminal):**
```bash
# For VST3
xattr -cr "/Library/Audio/Plug-Ins/VST3/Make Bassline.vst3"

# For AU
xattr -cr "/Library/Audio/Plug-Ins/Components/Make Bassline.component"
```

### Plugin not showing in DAW
- Restart DAW completely
- Go to DAW preferences → Rescan plugins
- Check that it's in the right folder

### No sound coming out
- Make sure DAW transport is playing
- Check MIDI routing (Make Bassline → Synth)
- Make Bassline only generates MIDI, not audio

---

## 📊 What to Ask Your Friend

Good questions to ask for feedback:

1. **First Impressions**
   - Was it easy to install?
   - Is the UI clear and understandable?
   - Does it look good/professional?

2. **Functionality**
   - Do all the knobs work as expected?
   - Is clicking sequencer steps intuitive?
   - Does randomize create good patterns?
   - Does MIDI export work smoothly?

3. **Performance**
   - Any crashes or freezes?
   - Does it stay in sync with DAW?
   - Any lag or glitches?

4. **Bugs**
   - What DAW are they using?
   - What macOS version?
   - Any error messages?
   - Can they reproduce the issue?

5. **Creative Feedback**
   - Are the patterns musical?
   - What features would they want?
   - What's confusing or unclear?

---

## 🔄 If You Need to Send Updates

If you fix bugs or add features:

1. Rebuild the project:
   ```bash
   /opt/homebrew/bin/cmake --build build --config Release
   ```

2. Re-run the packaging script:
   ```bash
   ./quick-package-for-testing.sh
   ```

3. Send new ZIP to your friend

---

## ⚠️ Known Limitations (Beta Version)

Tell your friend these are known issues:

- ❌ **Not code signed** - will show security warnings
- ❌ **No auto-updater** - manual updates only
- ❌ **Beta testing** - expect some rough edges
- ✅ **Core features work** - ready for testing!

---

## 🎯 Next Steps After Testing

Once your friend tests it:

1. **Collect feedback** - bugs, features, improvements
2. **Fix critical issues** - crashes, major bugs
3. **Decide on pricing** - free, paid, freemium?
4. **Get Apple Developer account** - for code signing ($99/year)
5. **Official release** - when you're ready!

---

## 📁 Package Contents

Your friend will receive:
```
MakeBassline-v1.0.0-beta/
├── Make Bassline.vst3        (VST3 plugin)
├── Make Bassline.component   (AU plugin for Logic)
├── Make Bassline.clap        (CLAP plugin)
├── install.sh                (Auto-installer)
├── HOW_TO_INSTALL.txt        (Installation guide)
└── FEATURES.txt              (Feature list)
```

---

## ✨ You're All Set!

The ZIP file is in:
```
/Users/williamnewton/Documents/GitHub/make-bassline/beta-test/
```

Pick a delivery method above and send it to your friend. Good luck with testing! 🚀
