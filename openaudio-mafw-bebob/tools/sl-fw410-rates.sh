#!/bin/bash
# FW410 rate sweep on the SL rig with the official panel (engine idle, no client): 96 -> 176.4 -> 192 -> 96 -> 88.2 -> 48 -> 44.1 -> 96.
# At each rate: registry dump of the FW410 device subtree, CoreAudio device list (slhal.py) and a panel shot. DTrace records the panel's writes.
cd ~/fw1814-captures/tools
O=$HOME/fw1814-captures/103/rates; mkdir -p $O
P=$(ps axo pid,comm | grep "MacOS/Panel" | grep -v grep | awk '{print $1}')
R="sudo launchctl bsexec $P sudo -u $USER"
snap() { sleep 8; ioreg -r -c com_m_audio_FW410Device -l -w0 > $O/ioreg-$1.txt; python slhal.py 2>/dev/null | grep "FireWire 410" > $O/hal-$1.txt; $R python slpanel.py shot $O/hw-$1.png >/dev/null; echo "$(python -c 'import time;print int(time.time()*1000)') snapshot $1 $(grep -o '"MASampleRate" = [0-9]*' $O/ioreg-$1.txt) $(cat $O/hal-$1.txt | tr -s ' ' | cut -d' ' -f1-12)"; }
sudo dtrace -qs slsetprops.d -o $O/setprops-rates.txt > $O/dtrace.err 2>&1 &
sleep 4
$R python slpanel.py front; $R python slpanel.py seq sl-rate-tab.seq; snap 96000-start
$R python slpanel.py seq sl-rate-up.seq; snap 176400
$R python slpanel.py seq sl-rate-up.seq; snap 192000
$R python slpanel.py seq sl-rate-down.seq; $R python slpanel.py seq sl-rate-down.seq; snap 96000
$R python slpanel.py seq sl-rate-down.seq; snap 88200
$R python slpanel.py seq sl-rate-down.seq; snap 48000
$R python slpanel.py seq sl-rate-down.seq; snap 44100
for i in 1 2 3 4; do $R python slpanel.py seq sl-rate-up.seq; sleep 3; done; snap 96000-end
sleep 2; sudo kill -TERM $(ps -axo pid,command | grep "dtrace -qs slsetprops.d" | grep -v grep | awk '{print $1}')
