#!/usr/bin/env python3
"""Artwork for the disk image: the uninstaller's icon (the app icon in steel with a red x badge), the dmg background (1x + 2x -> one TIFF) and the
volume icon. Run with the repo venv:  installer/.venv/bin/python installer/make-art.py   (needs Pillow; iconutil/tiffutil from macOS)."""
import os, subprocess, sys, shutil, tempfile
from PIL import Image, ImageDraw, ImageFont, ImageFilter, ImageEnhance

HERE = os.path.dirname(os.path.abspath(__file__))
ART = os.path.join(HERE, "art")
APP_ICNS = os.path.join(HERE, "..", "control-panel", "Resources", "AppIcon.icns")
RED = (200, 28, 28)
INK = (36, 40, 46)
GREY = (104, 110, 120)

def font(size, bold=False):
    for p, idx in (("/System/Library/Fonts/HelveticaNeue.ttc", 1 if bold else 0), ("/System/Library/Fonts/Helvetica.ttc", 1 if bold else 0),
                   ("/Library/Fonts/Arial.ttf", 0)):
        if os.path.exists(p):
            try: return ImageFont.truetype(p, size, index=idx)
            except Exception: pass
    return ImageFont.load_default()

def iconset_from_icns(icns, out):
    subprocess.check_call(["iconutil", "-c", "iconset", icns, "-o", out])

def uninstall_icon(src_png):
    """Steel-grey version of the app icon with a red round badge carrying a white x."""
    im = Image.open(src_png).convert("RGBA")
    r, g, b, a = im.split()
    grey = ImageEnhance.Contrast(Image.merge("RGB", (r, g, b)).convert("L").convert("RGB")).enhance(1.05)
    grey = ImageEnhance.Brightness(grey).enhance(1.05)
    out = Image.merge("RGBA", (*grey.split(), a))
    w, h = out.size
    d = int(w * 0.38)
    badge = Image.new("RGBA", (w, h), (0, 0, 0, 0))
    bd = ImageDraw.Draw(badge)
    x0, y0 = int(w * 0.60), int(h * 0.60)
    bd.ellipse((x0 + 4, y0 + 8, x0 + d + 4, y0 + d + 8), fill=(0, 0, 0, 110))          # shadow
    badge = badge.filter(ImageFilter.GaussianBlur(w * 0.012))
    bd = ImageDraw.Draw(badge)
    bd.ellipse((x0, y0, x0 + d, y0 + d), fill=(255, 255, 255, 255))
    pad = int(d * 0.06)
    bd.ellipse((x0 + pad, y0 + pad, x0 + d - pad, y0 + d - pad), fill=RED + (255,))
    c = (x0 + d / 2, y0 + d / 2); k = d * 0.2; t = max(2, int(d * 0.09))
    bd.line((c[0] - k, c[1] - k, c[0] + k, c[1] + k), fill=(255, 255, 255, 255), width=t)
    bd.line((c[0] - k, c[1] + k, c[0] + k, c[1] - k), fill=(255, 255, 255, 255), width=t)
    return Image.alpha_composite(out, badge)

def background(scale):
    W, H = 640 * scale, 420 * scale
    im = Image.new("RGB", (W, H))
    px = im.load()
    for y in range(H):                                   # soft silver gradient, slightly lighter at the top
        t = y / (H - 1)
        base = int(238 - 20 * t)
        for x in range(W):
            px[x, y] = (base, base + 1, base + 4)
    d = ImageDraw.Draw(im, "RGBA")
    s = scale
    d.rectangle((0, 0, W, 6 * s), fill=RED + (255,))     # red accent bar like the icon's glass
    d.text((36 * s, 28 * s), "OpenAudio FW1814", font=font(30 * s, True), fill=INK)
    d.text((36 * s, 68 * s), "macOS High Sierra (10.13.6) driver for the M-Audio FireWire 1814", font=font(14 * s), fill=GREY)
    # arrow between the two icon positions is NOT drawn (the two items are independent); a soft plate sits under each
    for cx in (170, 470):
        d.rounded_rectangle(((cx - 112) * s, 140 * s, (cx + 112) * s, 334 * s), radius=18 * s, fill=(255, 255, 255, 120), outline=(0, 0, 0, 26), width=s)
    d.text((170 * s, 322 * s), "", font=font(12 * s))
    d.text((170 * s, 146 * s), "1  INSTALL", font=font(12 * s, True), fill=RED, anchor="mt")
    d.text((470 * s, 146 * s), "REMOVE", font=font(12 * s, True), fill=GREY, anchor="mt")
    note = "Experimental and unsigned: it needs System Integrity Protection disabled (csrutil disable)."
    d.text((W // 2, 376 * s), note, font=font(12 * s), fill=GREY, anchor="mt")
    d.text((W // 2, 396 * s), "Quit audio and MIDI applications before installing or removing.", font=font(12 * s), fill=GREY, anchor="mt")
    return im

def main():
    os.makedirs(ART, exist_ok=True)
    tmp = tempfile.mkdtemp()
    src = os.path.join(tmp, "app.iconset")
    iconset_from_icns(APP_ICNS, src)
    dst = os.path.join(tmp, "UninstallIcon.iconset")
    os.makedirs(dst)
    for name in os.listdir(src):
        if name.endswith(".png"): uninstall_icon(os.path.join(src, name)).save(os.path.join(dst, name))
    subprocess.check_call(["iconutil", "-c", "icns", dst, "-o", os.path.join(ART, "UninstallIcon.icns")])
    shutil.copy(APP_ICNS, os.path.join(ART, "VolumeIcon.icns"))
    b1, b2 = os.path.join(tmp, "bg.png"), os.path.join(tmp, "bg@2x.png")
    background(1).save(b1); background(2).save(b2)
    for p in (b1, b2): subprocess.check_call(["sips", "-s", "dpiHeight", "72" if p == b1 else "144", "-s", "dpiWidth", "72" if p == b1 else "144", p], stdout=subprocess.DEVNULL)
    subprocess.check_call(["tiffutil", "-cathidpicheck", b1, b2, "-out", os.path.join(ART, "dmg-background.tiff")])
    shutil.copy(b2, os.path.join(ART, "dmg-background-preview@2x.png"))
    shutil.rmtree(tmp)
    print("art written to", ART)

if __name__ == "__main__":
    main()
