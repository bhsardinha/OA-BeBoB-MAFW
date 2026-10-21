#!/bin/bash
# Rate sweep on the SL rig with the official panel (engine idle, no client): 48 -> 44.1 -> 48 -> 88.2 -> 96 -> 48.
# At each rate: registry dump of the Audiophile device subtree + panel window shot. DTrace records the panel's writes.
cd ~/fw1814-captures/tools
O=$HOME/fw1814-captures/2026-10-19-audiophile/rates; mkdir -p $O
R="sudo launchctl bsexec 169 sudo -u $USER"
snap() { sleep 8; ioreg -r -c com_m_audio_FWAudiophileDevice -l > $O/ioreg-$1.txt; $R python slpanel.py shot $O/hw-$1.png >/dev/null; echo "$(python -c 'import time;print int(time.time()*1000)') snapshot $1 $(grep -o '"MASampleRate" = [0-9]*' $O/ioreg-$1.txt)"; }
sudo dtrace -qs slsetprops.d -o $O/setprops-rates.txt > $O/dtrace.err 2>&1 &
sleep 4
$R python slpanel.py seq sl-rate-tab.seq; snap 48000-start
$R python slpanel.py seq sl-rate-down.seq; snap 44100
$R python slpanel.py seq sl-rate-up.seq; snap 48000
$R python slpanel.py seq sl-rate-up.seq; snap 88200
$R python slpanel.py seq sl-rate-up.seq; snap 96000
$R python slpanel.py seq sl-rate-down.seq; snap 88200-back
$R python slpanel.py seq sl-rate-down.seq; snap 48000-end
sleep 2; sudo kill -TERM $(ps -axo pid,command | grep "dtrace -qs slsetprops.d" | grep -v grep | awk '{print $1}')
