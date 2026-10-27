#!/usr/bin/env python3
"""Artwork for the disk image: the uninstaller's icon (the app icon in steel with a red x badge), the dmg background (1x + 2x -> one TIFF) and the
volume icon. Run with the repo venv:  installer/.venv/bin/python installer/make-art.py [classic|modern]   (needs Pillow; iconutil/tiffutil
from macOS). One background per edition (R128): art/dmg-background-<edition>.tiff."""
import os, subprocess, sys, shutil, tempfile
from PIL import Image, ImageDraw, ImageFont, ImageFilter, ImageEnhance

HERE = os.path.dirname(os.path.abspath(__file__))
ART = os.path.join(HERE, "art")
VERSION = open(os.path.join(HERE, "..", "VERSION")).read().strip()      # the package version (CHANGELOG.md)
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

# Per-edition text (R128): title chip, subtitle, the step after the install, the notes at the bottom.
EDITIONS = {
    "classic": dict(chip="CLASSIC", chip_fill=(96, 104, 116),
                    sub="Driver for the M-Audio FireWire 1814, Audiophile and 410  \u00b7  macOS High Sierra 10.13.6",
                    step2="Restart the Mac, then power on the interface",
                    reqs=(("macOS", "10.13 High Sierra (tested on 10.13.6); 10.14 and 10.15 allowed, not tested"),
                          ("Mac", "Intel, with FireWire, or Thunderbolt + Apple Thunderbolt to FireWire Adapter"),
                          ("Interfaces", "FireWire 1814 and Audiophile (tested up to 0.2.2); FireWire 410 (tested, 0.4.0)"),
                          ("Security", "System Integrity Protection disabled (csrutil disable)"))),
    "modern": dict(chip="MODERN", chip_fill=RED,
                   sub="Driver for the M-Audio FireWire 1814, Audiophile and 410  \u00b7  macOS 11 to 15, Intel Macs",
                   step2="Allow it in System Settings \u203a Privacy & Security, then restart",
                   reqs=(("macOS", "11 Big Sur to 15 Sequoia, Intel only (tested on 14.8.9); no Apple Silicon"),
                         ("Mac", "Intel, with FireWire, or Thunderbolt + Apple Thunderbolt to FireWire Adapter"),
                         ("Interfaces", "Audiophile (tested on 0.3.1); FireWire 1814 (panel only); FireWire 410 (not tried)"),
                         ("Security", "SIP allowing unsigned kexts (csrutil disable, or enable --without kext)"))),
}
FOOT = "Experimental and unsigned. Quit audio and MIDI applications before installing or removing."
PHOTOS = [os.path.join(HERE, "..", "control-panel", "Resources", n) for n in ("fw1814-front.png", "fwap-front.png")]
PLATES = (170, 470)                                      # icon centres (dmg-settings.py icon_locations), plate captions above them

def background(scale, edition):
    """Dark panel-like background (the About banner's gradient), red accent, the two interfaces, light plates under the icons so Finder's
    dark labels stay readable, the step after the install, the requirements of the edition (user, 2026-10-21) and a footer.
    640 x 500 pt (dmg-settings.py window_rect)."""
    e = EDITIONS[edition]
    s = scale
    W, H = 640 * s, 500 * s
    top, bot = (36, 39, 45), (13, 14, 16)
    im = Image.new("RGB", (W, H))
    grad = Image.new("RGB", (1, H))
    for y in range(H):
        t = y / (H - 1)
        grad.putpixel((0, y), tuple(int(top[i] + (bot[i] - top[i]) * t) for i in range(3)))
    im.paste(grad.resize((W, H)))
    glow = Image.new("RGBA", (W, H), (0, 0, 0, 0))       # soft light behind the interfaces
    ImageDraw.Draw(glow).ellipse((150 * s, 70 * s, 490 * s, 160 * s), fill=(255, 255, 255, 22))
    im = Image.alpha_composite(im.convert("RGBA"), glow.filter(ImageFilter.GaussianBlur(28 * s)))
    d = ImageDraw.Draw(im, "RGBA")
    d.rectangle((0, 0, W, 4 * s), fill=RED + (255,))
    # title: name, version, edition chip
    ft, fv, fc = font(27 * s, True), font(27 * s), font(11 * s, True)
    x, y = 32 * s, 22 * s
    d.text((x, y), "OA BeBoB MAFW", font=ft, fill=(242, 244, 247))
    x += d.textlength("OA BeBoB MAFW ", font=ft)
    d.text((x, y), VERSION, font=fv, fill=(150, 156, 166))
    x += d.textlength(VERSION, font=fv) + 12 * s
    cw = d.textlength(e["chip"], font=fc) + 18 * s
    d.rounded_rectangle((x, y + 8 * s, x + cw, y + 28 * s), radius=10 * s, fill=e["chip_fill"] + (255,))
    d.text((x + cw / 2, y + 18 * s), e["chip"], font=fc, fill=(255, 255, 255), anchor="mm")
    d.text((32 * s, 60 * s), e["sub"], font=font(13 * s), fill=(160, 166, 176))
    # the two interfaces, scaled to one height, centred
    ph = 34 * s
    imgs = []
    for p in PHOTOS:
        if os.path.exists(p):
            src = Image.open(p).convert("RGBA")
            imgs.append(src.resize((int(src.width * ph / src.height), ph), Image.LANCZOS))
    gap = 26 * s
    total = sum(i.width for i in imgs) + gap * (len(imgs) - 1)
    px = (W - total) // 2
    for i in imgs:
        shadow = Image.new("RGBA", (W, H), (0, 0, 0, 0))
        ImageDraw.Draw(shadow).rounded_rectangle((px + 6 * s, 98 * s + ph - 4 * s, px + i.width - 6 * s, 98 * s + ph + 6 * s), radius=6 * s, fill=(0, 0, 0, 150))
        im = Image.alpha_composite(im, shadow.filter(ImageFilter.GaussianBlur(6 * s)))
        im.alpha_composite(i, (px, 96 * s))
        px += i.width + gap
    im = im.convert("RGB")                               # drawing with alpha blends only on an RGB image
    d = ImageDraw.Draw(im, "RGBA")
    # plates under the two icons (light, for Finder's dark labels), with their captions
    for cx, cap, col in ((PLATES[0], "1   INSTALL", RED), (PLATES[1], "REMOVE", (110, 116, 126))):
        d.rounded_rectangle(((cx - 108) * s, 146 * s, (cx + 108) * s, 330 * s), radius=16 * s, fill=(244, 245, 248, 240), outline=(255, 255, 255, 60), width=s)
        d.text((cx * s, 156 * s), cap, font=font(12 * s, True), fill=col, anchor="mt")
    # step 2, centred under the plates: the number in red like step 1
    fb = font(12 * s, True)
    num, gap2 = "2", 10 * s
    w2 = d.textlength(num, font=fb) + gap2 + d.textlength(e["step2"], font=fb)
    x2 = (W - w2) / 2
    d.text((x2, 344 * s), num, font=fb, fill=RED)
    d.text((x2 + d.textlength(num, font=fb) + gap2, 344 * s), e["step2"], font=fb, fill=(236, 238, 242))
    d.line(((PLATES[0] - 108) * s, 365 * s, (PLATES[1] + 108) * s, 365 * s), fill=(255, 255, 255, 30), width=s)
    # requirements: OS range, Mac, interfaces, security (what is tested is said in the line)
    x0, xv = (PLATES[0] - 108) * s, (PLATES[0] - 108 + 84) * s
    d.text((x0, 378 * s), "REQUIREMENTS", font=font(10 * s, True), fill=RED)
    fl, fv2 = font(11 * s, True), font(11 * s)
    for i, (k, v) in enumerate(e["reqs"]):
        y = (396 + 17 * i) * s
        d.text((x0, y), k, font=fl, fill=(214, 218, 224))
        assert xv + d.textlength(v, font=fv2) <= (PLATES[1] + 108) * s, "requirement line too long: " + v
        d.text((xv, y), v, font=fv2, fill=(164, 170, 180))
    d.text((W // 2, 472 * s), FOOT, font=font(11 * s), fill=(120, 126, 136), anchor="mt")
    return im

def main():
    editions = sys.argv[1:] or list(EDITIONS)          # make-art.py [classic|modern ...]; default both
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
    for ed in editions:
        b1, b2 = os.path.join(tmp, ed + "-bg.png"), os.path.join(tmp, ed + "-bg@2x.png")
        background(1, ed).save(b1); background(2, ed).save(b2)
        for p in (b1, b2): subprocess.check_call(["sips", "-s", "dpiHeight", "72" if p == b1 else "144", "-s", "dpiWidth", "72" if p == b1 else "144", p], stdout=subprocess.DEVNULL)
        subprocess.check_call(["tiffutil", "-cathidpicheck", b1, b2, "-out", os.path.join(ART, "dmg-background-" + ed + ".tiff")])
        shutil.copy(b2, os.path.join(ART, "dmg-background-" + ed + "-preview@2x.png"))
    shutil.rmtree(tmp)
    print("art written to", ART)

if __name__ == "__main__":
    main()
