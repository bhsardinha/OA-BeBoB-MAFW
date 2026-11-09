#!/usr/bin/env python3
"""OpenAudio Glue app icon from the logo (docs/images/glue_logo.jpg, white background) - user, 2026-10-28.

Makes the white background transparent: near-white, low-saturation pixels connected to the image border are removed (a flood fill,
so light parts inside the design stay); a 3-pixel band along the cut is feathered and the white is taken out of its colour (no white
fringe on dark backgrounds). The logo is then placed on the macOS icon grid (about 10 % margin) and written as AppIcon.png (1024) and
AppIcon.icns (every size, through iconutil).

    python3 make-icon.py <logo.jpg> <output folder>
"""
import os
import subprocess
import sys
import tempfile

import numpy as np
from PIL import Image

WHITE_MIN = 228          # all three channels at least this bright ...
GREY_SPREAD = 22         # ... and nearly neutral, to count as background
FEATHER = 3              # pixels of soft edge
CONTENT = 0.88           # share of the 1024 grid the logo's bounding box fills


def flood_from_border(candidate):
    """Background = candidate pixels connected to the border (4-neighbour), by repeated constrained dilation."""
    bg = np.zeros_like(candidate)
    bg[0, :] = candidate[0, :]; bg[-1, :] = candidate[-1, :]; bg[:, 0] = candidate[:, 0]; bg[:, -1] = candidate[:, -1]
    while True:
        grown = bg.copy()
        grown[1:, :] |= bg[:-1, :]; grown[:-1, :] |= bg[1:, :]; grown[:, 1:] |= bg[:, :-1]; grown[:, :-1] |= bg[:, 1:]
        grown &= candidate
        if (grown == bg).all():
            return bg
        bg = grown


def dilate(mask, n):
    m = mask.copy()
    for _ in range(n):
        g = m.copy()
        g[1:, :] |= m[:-1, :]; g[:-1, :] |= m[1:, :]; g[:, 1:] |= m[:, :-1]; g[:, :-1] |= m[:, 1:]
        m = g
    return m


def main(src, out):
    img = np.asarray(Image.open(src).convert("RGB")).astype(np.float64)
    lo, hi = img.min(axis=2), img.max(axis=2)
    candidate = (lo >= WHITE_MIN) & ((hi - lo) <= GREY_SPREAD)
    # The ring: its circle from the logo's left / right edges and its top (the lettering only sticks out below and to the sides
    # inside that width). Near-white outside the circle is background even where a letter encloses it (the counter of the G's loop);
    # inside it, the metal's highlights stay.
    solid = ~candidate
    cols = np.where(solid.any(axis=0))[0]; rows = np.where(solid.any(axis=1))[0]
    r = (cols[-1] - cols[0]) / 2.0
    cx, cy = (cols[0] + cols[-1]) / 2.0, rows[0] + r
    yy, xx = np.mgrid[0:lo.shape[0], 0:lo.shape[1]]
    outside = (xx - cx) ** 2 + (yy - cy) ** 2 > (r + 1.0) ** 2
    bg = flood_from_border(candidate) | (candidate & outside)

    alpha = np.ones(lo.shape)
    alpha[bg] = 0.0
    band = dilate(bg, FEATHER) & ~bg
    # in the band: the closer to white, the more transparent
    a = np.clip((250.0 - lo) / (250.0 - 170.0), 0.0, 1.0)
    alpha[band] = np.maximum(a[band], 0.0)
    # take the white out of the soft edge: C = (C - (1 - a) * 255) / a
    rgb = img.copy()
    edge = band & (alpha > 0.02)
    for c in range(3):
        ch = rgb[:, :, c]
        ch[edge] = np.clip((ch[edge] - (1.0 - alpha[edge]) * 255.0) / alpha[edge], 0, 255)
    rgba = np.dstack([rgb, alpha * 255.0]).round().astype(np.uint8)
    logo = Image.fromarray(rgba)

    # crop to the visible logo, then fit it on the 1024 icon grid
    box = logo.getchannel("A").point(lambda v: 255 if v > 8 else 0).getbbox()
    logo = logo.crop(box)
    side = int(1024 * CONTENT)
    scale = side / max(logo.size)
    logo = logo.resize((max(1, round(logo.width * scale)), max(1, round(logo.height * scale))), Image.LANCZOS)
    icon = Image.new("RGBA", (1024, 1024), (0, 0, 0, 0))
    icon.paste(logo, ((1024 - logo.width) // 2, (1024 - logo.height) // 2), logo)
    os.makedirs(out, exist_ok=True)
    icon.save(os.path.join(out, "AppIcon.png"))

    with tempfile.TemporaryDirectory() as tmp:
        iconset = os.path.join(tmp, "AppIcon.iconset")
        os.makedirs(iconset)
        for size in (16, 32, 128, 256, 512):
            icon.resize((size, size), Image.LANCZOS).save(os.path.join(iconset, f"icon_{size}x{size}.png"))
            icon.resize((size * 2, size * 2), Image.LANCZOS).save(os.path.join(iconset, f"icon_{size}x{size}@2x.png"))
        subprocess.run(["iconutil", "-c", "icns", iconset, "-o", os.path.join(out, "AppIcon.icns")], check=True)
    print(f"ring centre ({cx:.0f}, {cy:.0f}) radius {r:.0f}; background removed: {int(bg.sum())} of {bg.size}; feathered: {int(band.sum())}; box {box}")


if __name__ == "__main__":
    if len(sys.argv) != 3:
        sys.exit(__doc__)
    main(sys.argv[1], sys.argv[2])
