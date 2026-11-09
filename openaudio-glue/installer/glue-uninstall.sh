#!/bin/bash
# OpenAudio Glue: removes every file any Glue release installed (old paths are never taken off this list) and restarts Core Audio.
# Lives in /Library/Application Support/OpenAudio Glue (the main package's uninstaller removes /Library/Application Support/OpenAudio).
# --update: leave Core Audio alone (the new package restarts it). Never touches the OpenAudio driver package's files.
FILES=("/Library/Audio/Plug-Ins/HAL/OpenAudioGlue.driver" "/Applications/OpenAudio Glue.app")
for f in "${FILES[@]}"; do /bin/rm -rf "$f"; done
/usr/sbin/pkgutil --forget com.openaudio.glue.pkg >/dev/null 2>&1
[ "${1:-}" = "--update" ] && exit 0
/bin/rm -rf "/Library/Application Support/OpenAudio Glue"
/usr/bin/killall coreaudiod 2>/dev/null
exit 0
