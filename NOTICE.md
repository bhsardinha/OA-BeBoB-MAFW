# Notice — M-Audio ownership, intended use, third-party material

## What this project is for
OpenAudio FW1814 is an independent, non-commercial project. Its only purpose is to let **people who already own M-Audio FireWire hardware** keep using it on current macOS versions, after the manufacturer stopped supporting these interfaces more than a decade ago. The software does nothing without that hardware: it only drives an M-Audio FireWire 1814 (the only interface it has been written for and tested on). It is not for sale, earns nothing for anyone, and is not meant for anyone who does not own the interface.

## M-Audio's ownership
The original M-Audio FireWire driver software for these interfaces — the kernel extension, control panel, preference pane, CoreMIDI plug-in, helper/firmware loader and the firmware itself (the interface firmware is by BridgeCo), together with the device icons, product photographs and manuals — **is the property of M-Audio / inMusic Brands, Inc. and their licensors**. This project claims no rights in any of it and is grateful for the free software M-Audio published for these products.

Material of that kind that does appear in this project:
- the **device icons** (`openaudio-fw1814/installer/assets/DeviceIcons/`) and a **product photograph** of the FireWire 1814 used in the panel's About page;
- the **CoreMIDI plug-in** (`MAudioFireWireMIDI.plugin`, version 1.11.2), **unmodified**, inside the released installer package and disk image — it is not in the source repository;
- **disassembly, decompiled text and notes** about the original software under `docs/`, published to document how the hardware and the original driver work so that the interfaces stay usable. The original binaries, firmware images and installers themselves are not in the repository.

These items stay under their owners' terms; the project license does **not** apply to them.

## What the project license covers
The **PolyForm Noncommercial License 1.0.0** (see [`LICENSE`](LICENSE)) covers the original work of this project — the new driver, helper, control panel, installer, tools and the author's own notes. It allows personal and non-profit use, changes and sharing, and does not allow commercial use. It does not cover M-Audio's material listed above, nor the third-party references below.

## Third-party references
The wire protocol and register meanings were cross-checked with [FFADO](https://github.com/ojsl1/libffado), the Linux [snd-bebob](https://github.com/torvalds/linux/tree/master/sound/firewire/bebob) driver, [snd-firewire-ctl-services](https://github.com/alsa-project/snd-firewire-ctl-services) and [macfw](https://github.com/mbprado/macfw), and the analysis used [Ghidra](https://ghidra-sre.org). Those projects have their own licenses; their sources are not included here.

## Not affiliated
Independent and unofficial: not made, endorsed or supported by M-Audio, inMusic, Avid, Apple or BridgeCo. *M-Audio*, *FireWire*, *macOS* and other names are trademarks of their owners.
