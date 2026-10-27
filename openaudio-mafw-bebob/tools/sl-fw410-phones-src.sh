#!/bin/bash
# FW410: which bus is MAHeadphoneSourceV2 bit 5 (0x20)? (SL rig, official v1.9; capture 104). A -40 dBFS 1 kHz tone plays on SW return 1/2
# (routed to out 1/2 only); for each phones source mask the 76-byte status packet is read and quadlets 4-17 (hardware output peaks) printed.
# Then the tone is also sent to the aux bus (aux sends of SW return 1/2 at 0 dB) and masks 32 / 1 are read again. Everything restored after.
cd ~/fw1814-captures/tools; O=$HOME/fw1814-captures/104; mkdir -p $O
python - <<'PY'
import wave, struct, math
w = wave.open('/Users/bernardosardinha/fw1814-captures/104/tone.wav', 'w'); w.setnchannels(2); w.setsampwidth(2); w.setframerate(96000)
a = 32767 * 10 ** (-40 / 20.0)
w.writeframes(''.join(struct.pack('<hh', int(a * math.sin(2 * math.pi * 1000 * i / 96000.0)), int(a * math.sin(2 * math.pi * 1000 * i / 96000.0))) for i in range(96000 * 40)))
w.close()
PY
peaks() { for k in 0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18; do printf "%s " $(./slfwread quad ffc7 $(printf "%08x" $((0x00600000 + 4*k))) | tail -1 | sed 's/.*bytes //' | tr -d ' '); done; echo; }
afplay $O/tone.wav & AP=$!; sleep 3
echo "baseline (mask 1)"; peaks
for m in 0 1 2 4 8 16 32; do python slprops.py set MAHeadphoneSourceV2 $m >/dev/null; sleep 1; echo "mask $m"; peaks; done
python slprops.py setarr MAAuxSendLevels -255,-255,-255,-255,0,0,-255,-255,-255,-255,-255,-255,-255,-255 >/dev/null; sleep 1
for m in 32 1 0; do python slprops.py set MAHeadphoneSourceV2 $m >/dev/null; sleep 1; echo "aux on, mask $m"; peaks; done
python slprops.py setarr MAMonitorAuxOnOutput 0,0,0,0,0,1 >/dev/null; sleep 1; echo "aux on, phones MAIN/AUX = aux, mask 1"; python slprops.py set MAHeadphoneSourceV2 1 >/dev/null; sleep 1; peaks
kill $AP; wait $AP 2>/dev/null
python slprops.py restore ~/fw1814-captures/103/settings-backup.plist D6C010006B8C0 | grep -v "0x0$"
cp ~/fw1814-captures/103/settings-backup.plist ~/Library/Preferences/com.m-audio.device-settings.firewire.plist; echo restored
