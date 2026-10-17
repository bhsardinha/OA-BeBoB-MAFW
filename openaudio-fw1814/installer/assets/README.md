# installer/assets

`DeviceIcons/` (the M-Audio device icons the CoreMIDI plug-in looks up by `MAIconName`) is included.

**`MAudioFireWireMIDI.plugin` is not included.** Take it, unmodified, from your own copy of the official *M-Audio FireWire 1.11.2* installer
(`Library/Audio/MIDI Drivers/`) and put it here before running `installer/make-pkg.sh`. The driver reuses that plug-in as is (docs/system-map/A3-midi).
