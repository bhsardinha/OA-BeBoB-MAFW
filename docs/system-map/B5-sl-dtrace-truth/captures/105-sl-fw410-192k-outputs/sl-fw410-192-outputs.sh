#!/bin/bash
# FW410 at 192 kHz: which outputs carry audio (official v1.9, SL rig; capture 105). For each rate (96 kHz reference, then 192 kHz) and each
# output channel: openaudio-fw1814/tools/chtone plays a -40 dBFS 1 kHz tone on that device channel only (default device = FireWire 410
# Multichannel; afplay folds multichannel files to 2 channels, first attempt), and the status packet's hardware output peaks (quadlets 4-17)
# are read. Rate set with the panel arrows (engine idle between plays).
cd ~/fw1814-captures/tools; O=$HOME/fw1814-captures/105; mkdir -p $O
P=$(ps axo pid,comm | grep "MacOS/Panel" | grep -v grep | awk '{print $1}'); R="sudo launchctl bsexec $P sudo -u $USER python slpanel.py"
mk() { python - "$1" "$2" <<'PY'
import wave, struct, math, sys
ch = int(sys.argv[1]); rate = int(sys.argv[2]); a = int(32767 * 10 ** (-40 / 20.0))
w = wave.open('/Users/bernardosardinha/fw1814-captures/105/tone-%d-%d.wav' % (rate, ch), 'w'); w.setnchannels(8); w.setsampwidth(2); w.setframerate(rate)
fr = []
for i in range(rate * 6):
    s = int(a * math.sin(2 * math.pi * 1000 * i / float(rate))); fr.append(struct.pack('<8h', *[s if k == ch - 1 else 0 for k in range(8)]))
w.writeframes(''.join(fr)); w.close()
PY
}
pk() { for k in 4 5 6 7 8 9 10 11 12 13 14 15 16 17; do printf "%s " $(./slfwread quad ffc7 $(printf "%08x" $((0x00600000 + 4*k))) | tail -1 | sed "s/.*bytes //" | tr -d " " | cut -c1-4); done; echo; }
run() { rate=$1; echo "== rate $rate (MASampleRate $(ioreg -r -c com_m_audio_FW410Device -l -w0 | grep -o '"MASampleRate" = [0-9]*'))"; echo "        q4   q5   q6   q7   q8   q9   q10  q11  q12  q13  q14  q15  q16  q17"; echo -n "idle    "; pk
  for c in $2; do ./chtone $c 4 >/dev/null & AP=$!; sleep 2; printf "ch %-5s " $c; pk; wait $AP; sleep 1; done; }
$R front; $R seq sl-rate-tab.seq >/dev/null
run 96000 "1 2 3 4 5 6 7 8 9 10"
$R seq sl-rate-up.seq >/dev/null; sleep 6; $R seq sl-rate-up.seq >/dev/null; sleep 8
run 192000 "1 2 3 4 5 6 7 8"
$R seq sl-rate-down.seq >/dev/null; sleep 6; $R seq sl-rate-down.seq >/dev/null; sleep 8
echo "end: $(ioreg -r -c com_m_audio_FW410Device -l -w0 | grep -o '"MASampleRate" = [0-9]*')"
