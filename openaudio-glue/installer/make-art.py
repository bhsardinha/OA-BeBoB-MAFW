#!/usr/bin/env python3
"""Disk-image artwork for OpenAudio Glue: uninstaller icon, volume icon and one background per edition, in the style of the main project's image (its
installer/make-art.py supplies the fonts and the uninstall badge). Run with the main project's venv:
  ../openaudio-mafw-bebob/installer/.venv/bin/python installer/make-art.py [classic|modern]"""
import importlib.util, os, shutil, subprocess, sys, tempfile
from PIL import Image, ImageDraw, ImageFilter

HERE = os.path.dirname(os.path.abspath(__file__))
ART = os.path.join(HERE, "art")
VERSION = open(os.path.join(HERE, "..", "VERSION")).read().strip()
APP_ICNS = os.path.join(HERE, "..", "panel", "Resources", "AppIcon.icns")
_spec = importlib.util.spec_from_file_location("mainart", os.path.join(HERE, "..", "..", "openaudio-mafw-bebob", "installer", "make-art.py"))
_main = importlib.util.module_from_spec(_spec); _spec.loader.exec_module(_main)
font, uninstall_icon, iconset_from_icns, RED = _main.font, _main.uninstall_icon, _main.iconset_from_icns, _main.RED

EDITIONS = {
    "classic": dict(chip="CLASSIC", chip_fill=(96, 104, 116), macos="10.13 High Sierra to 10.15 Catalina", driver="OA BeBoB MAFW Classic 0.6.0 or later, installed and the Mac restarted"),
    "modern": dict(chip="MODERN", chip_fill=RED, macos="11 Big Sur to 15 Sequoia, Intel only; no Apple Silicon", driver="OA BeBoB MAFW Modern 0.6.0 or later, installed and the Mac restarted"),
}
PLATES = (170, 470)     # icon centres (dmg-settings.py)

def background(scale, edition, logo):
    e = EDITIONS[edition]
    s = scale
    W, H = 640 * s, 500 * s
    top, bot = (36, 39, 45), (13, 14, 16)
    grad = Image.new("RGB", (1, H))
    for y in range(H):
        t = y / (H - 1)
        grad.putpixel((0, y), tuple(int(top[i] + (bot[i] - top[i]) * t) for i in range(3)))
    im = grad.resize((W, H)).convert("RGBA")
    glow = Image.new("RGBA", (W, H), (0, 0, 0, 0))
    ImageDraw.Draw(glow).ellipse((230 * s, 60 * s, 410 * s, 150 * s), fill=(255, 255, 255, 22))
    im = Image.alpha_composite(im, glow.filter(ImageFilter.GaussianBlur(24 * s)))
    src = Image.open(logo).convert("RGBA").resize((54 * s, 54 * s), Image.LANCZOS)
    im.alpha_composite(src, ((W - src.width) // 2, 84 * s))
    im = im.convert("RGB")
    d = ImageDraw.Draw(im, "RGBA")
    d.rectangle((0, 0, W, 4 * s), fill=RED + (255,))
    ft, fv, fc = font(27 * s, True), font(27 * s), font(11 * s, True)
    x, y = 32 * s, 22 * s
    d.text((x, y), "OpenAudio Glue", font=ft, fill=(242, 244, 247))
    x += d.textlength("OpenAudio Glue ", font=ft)
    d.text((x, y), VERSION, font=fv, fill=(150, 156, 166))
    x += d.textlength(VERSION, font=fv) + 12 * s
    cw = d.textlength(e["chip"], font=fc) + 18 * s
    d.rounded_rectangle((x, y + 8 * s, x + cw, y + 28 * s), radius=10 * s, fill=e["chip_fill"] + (255,))
    d.text((x + cw / 2, y + 18 * s), e["chip"], font=fc, fill=(255, 255, 255), anchor="mm")
    d.text((32 * s, 60 * s), "An analog-style input device for each M-Audio FireWire 1814", font=font(13 * s), fill=(160, 166, 176))
    for cx, cap, col in ((PLATES[0], "INSTALL", RED), (PLATES[1], "REMOVE", (110, 116, 126))):
        d.rounded_rectangle(((cx - 108) * s, 146 * s, (cx + 108) * s, 330 * s), radius=16 * s, fill=(244, 245, 248, 240), outline=(255, 255, 255, 60), width=s)
        d.text((cx * s, 156 * s), cap, font=font(12 * s, True), fill=col, anchor="mt")
    d.text((W // 2, 344 * s), "Core Audio restarts during the install: audio stops for a moment", font=font(12 * s, True), fill=(236, 238, 242), anchor="mt")
    d.line(((PLATES[0] - 108) * s, 365 * s, (PLATES[1] + 108) * s, 365 * s), fill=(255, 255, 255, 30), width=s)
    x0, xv = (PLATES[0] - 108) * s, (PLATES[0] - 108 + 84) * s
    d.text((x0, 378 * s), "REQUIREMENTS", font=font(10 * s, True), fill=RED)
    fl, fv2 = font(11 * s, True), font(11 * s)
    reqs = (("macOS", e["macos"]), ("Driver", e["driver"]), ("Interfaces", "FireWire 1814 only"))
    for i, (k, v) in enumerate(reqs):
        y = (396 + 17 * i) * s
        d.text((x0, y), k, font=fl, fill=(214, 218, 224))
        assert xv + d.textlength(v, font=fv2) <= (PLATES[1] + 108) * s, "requirement line too long: " + v
        d.text((xv, y), v, font=fv2, fill=(164, 170, 180))
    d.text((W // 2, 472 * s), "Experimental and unsigned. Only for owners of M-Audio FireWire hardware.", font=font(11 * s), fill=(120, 126, 136), anchor="mt")
    return im

def main():
    editions = sys.argv[1:] or list(EDITIONS)
    os.makedirs(ART, exist_ok=True)
    tmp = tempfile.mkdtemp()
    src = os.path.join(tmp, "app.iconset"); iconset_from_icns(APP_ICNS, src)
    dst = os.path.join(tmp, "UninstallIcon.iconset"); os.makedirs(dst)
    for name in os.listdir(src):
        if name.endswith(".png"): uninstall_icon(os.path.join(src, name)).save(os.path.join(dst, name))
    subprocess.check_call(["iconutil", "-c", "icns", dst, "-o", os.path.join(ART, "UninstallIcon.icns")])
    shutil.copy(APP_ICNS, os.path.join(ART, "VolumeIcon.icns"))
    logo = os.path.join(src, "icon_512x512.png")
    for ed in editions:
        b1, b2 = os.path.join(tmp, ed + "-bg.png"), os.path.join(tmp, ed + "-bg@2x.png")
        background(1, ed, logo).save(b1); background(2, ed, logo).save(b2)
        for p, dpi in ((b1, "72"), (b2, "144")): subprocess.check_call(["sips", "-s", "dpiHeight", dpi, "-s", "dpiWidth", dpi, p], stdout=subprocess.DEVNULL)
        subprocess.check_call(["tiffutil", "-cathidpicheck", b1, b2, "-out", os.path.join(ART, "dmg-background-" + ed + ".tiff")])
        shutil.copy(b2, os.path.join(ART, "dmg-background-" + ed + "-preview@2x.png"))
    shutil.rmtree(tmp)
    print("art written to", ART)

if __name__ == "__main__":
    main()
