# dmgbuild settings (https://dmgbuild.readthedocs.io): layout of the OpenAudio BeBoB MAFW disk image. Paths come from `-D` defines set by make-dmg.sh.
import os
pkg = defines["pkg"]
app = defines["app"]
art = defines["art"]

format = "UDZO"                                   # zlib-compressed, HFS+: opens on 10.13
filesystem = "HFS+"
size = None
icon = os.path.join(art, "VolumeIcon.icns")       # the disk's own icon
background = os.path.join(art, "dmg-background.tiff")
files = [(pkg, "Install OA BeBoB MAFW.pkg"), (app, "Uninstall OA BeBoB MAFW.app")]
symlinks = {}
icon_locations = {"Install OA BeBoB MAFW.pkg": (170, 232), "Uninstall OA BeBoB MAFW.app": (470, 232)}
window_rect = ((200, 140), (640, 420))
default_view = "icon-view"
show_status_bar = False
show_tab_view = False
show_toolbar = False
show_pathbar = False
show_sidebar = False
sidebar_width = 0
arrange_by = None
grid_offset = (0, 0)
grid_spacing = 100
scroll_position = (0, 0)
label_pos = "bottom"
text_size = 13
icon_size = 112
include_icon_view_settings = "auto"
include_list_view_settings = "auto"
