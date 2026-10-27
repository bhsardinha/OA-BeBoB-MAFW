# Capture 101 - SL rig, FW1814 + official v1.9: system sleep, interface power lost during sleep, wake (2026-10-22)

Probes: `tools/slwakewatch.d` (IOLog format strings + IOSleep, as capture 73). `pmset sleepnow` over ssh (first try from a background subshell did nothing), the user switched the interface off and on while the Mac Pro slept, then woke it by the power button. Panel 1.4 was open (client of the FWUserClient), engine idle. Files: `dtrace-sleep-wake.txt` (raw), `ioreg-before.txt`, `ioreg-final.txt`. Times below are the dtrace wall clock ms (1792719xxx.xxx).

## Timeline (official v1.9)
- 491.4-492.4 pre-sleep IOSleep chain 500/300/300/150/2500/150... (`SystemWillSleep`: blank-slate clock, pause, disconnect).
- 494.7 System Sleep / Wake (IOLog `Wake reason`), 532.0 `Previous Sleep Cause`.
- 532.35 `M-Audio FireWire device found` + `metanub` (the BOOTLOADER nub, GUID ...041).
- 536.20 `Firnware active and we already started matching.` (sic) = the OLD runtime metanub gets the start message; 536.21 `Bootloader active: starting firmware` (new bootloader metanub).
- 536.37 bus reset started ... 539.455/539.582 bus reset completed (~3.2 s after the firmware start).
- 538.97 `Write for %p done`, IOSleep 2500 + 300 (SetBlankSlateClockSource); 539.27 twenty `IOAudioControl::hardwareValueChanged - invalid value`.
- 539.455 IOSleep(500) (= `SystemWokeUp` first step, starts when the bus reset completes); 539.955 `Trying to resend mixer settings too often` + IOSleep(1000); 540.98 IOSleep(300) ... (SetClockSource chain, then ConnectAllPlugs).
- 564.96 `Reregister: device node vanished` (bootloader metanub goes away).

## Result (ioreg-final vs ioreg-before)
**The same objects survive**: IOFireWireDevice@d6c040012337f id 0x1000006e4, FWMetaNub 0x1000006e7, FW1814Device 0x1000006f5, FWEngine2 0x1000006fb. No new runtime metanub/device/engine is created. The temporary bootloader device (...041) and its metanub are removed. So after sleep + power loss the official driver recovers IN PLACE (the IOFireWireFamily resumes the old device object), not through the cold-boot path of capture 73.

## Not captured / unverified
Whether sound works afterwards on the SL rig (no audio client was run; the user did not report). The order "mixer resend after bus-reset-completed + 500 ms" is read from the IOSleep/IOLog sequence only.
