#!/usr/bin/env python3
"""Generate the default skin images (wallpaper and placeholder cover).

Usage: python3 scripts/gen_default_images.py
Requires Pillow (Fedora: sudo dnf install python3-pillow).
Colors follow assets/config/skin_default.json.
"""
import math
from pathlib import Path

from PIL import Image, ImageDraw

IMAGES_DIR = Path(__file__).resolve().parent.parent / "assets" / "images"
SUPERSAMPLE = 4  # draw large and downscale for smooth edges

BACKGROUND = (17, 17, 17)
ACCENT = (0, 255, 0)


def lerp(a, b, t):
    return tuple(round(x + (y - x) * t) for x, y in zip(a, b))


def wallpaper(width=240, height=320):
    """Dark vertical gradient with scanlines and a faint perspective grid at the bottom."""
    img = Image.new("RGB", (width, height))
    draw = ImageDraw.Draw(img)

    top, bottom = (8, 20, 10), BACKGROUND
    for y in range(height):
        color = lerp(top, bottom, y / (height - 1))
        if y % 2:  # CRT scanline
            color = tuple(max(c - 4, 0) for c in color)
        draw.line([(0, y), (width - 1, y)], fill=color)

    # Status bar band (layout.status is y 2..18).
    for y in range(0, 20):
        draw.line([(0, y), (width - 1, y)], fill=(22, 30, 22))
    draw.line([(0, 20), (width - 1, 20)], fill=(0, 70, 0))

    # Perspective grid behind the progress/time area, kept dim so text stays readable.
    grid = (0, 34, 0)
    horizon, vanish_x = 250, width / 2
    for i in range(1, 9):
        y = horizon + (height - horizon) * (i / 8) ** 2
        draw.line([(0, y), (width - 1, y)], fill=grid)
    for i in range(-8, 9):
        draw.line([(vanish_x + i * 6, horizon), (vanish_x + i * 40, height)], fill=grid)
    return img


def default_cover(size=160):
    """Vinyl record with an accent label, used when a track has no cover art."""
    s = size * SUPERSAMPLE
    img = Image.new("RGB", (s, s), (26, 26, 26))
    draw = ImageDraw.Draw(img)
    c = s / 2

    def circle(r, **kw):
        draw.ellipse([c - r, c - r, c + r, c + r], **kw)

    circle(s * 0.46, fill=(8, 8, 8))
    # Faint light reflection, under the grooves.
    for start, color in ((200, (16, 16, 16)), (208, (20, 20, 20)), (20, (14, 14, 14))):
        draw.pieslice([c - s * 0.45, c - s * 0.45, c + s * 0.45, c + s * 0.45],
                      start=start, end=start + 14, fill=color)
    for r in range(int(s * 0.20), int(s * 0.45), 3 * SUPERSAMPLE):  # grooves
        circle(r, outline=(30, 30, 30), width=SUPERSAMPLE)
    circle(s * 0.18, fill=(0, 150, 0))
    circle(s * 0.17, outline=ACCENT, width=SUPERSAMPLE)

    # Eighth note on the label.
    note = (12, 12, 12)
    head_r = s * 0.035
    hx, hy = c - s * 0.03, c + s * 0.06
    draw.ellipse([hx - head_r * 1.3, hy - head_r, hx + head_r * 1.3, hy + head_r], fill=note)
    stem_x = hx + head_r * 1.15
    draw.rectangle([stem_x - SUPERSAMPLE * 1.5, c - s * 0.10, stem_x + SUPERSAMPLE * 1.5, hy], fill=note)
    flag = [(stem_x + s * 0.06 * math.sin(t / 20 * math.pi * 0.8), c - s * 0.10 + s * 0.09 * t / 20)
            for t in range(21)]
    draw.line(flag, fill=note, width=SUPERSAMPLE * 3)

    circle(s * 0.012, fill=(26, 26, 26))  # spindle hole
    return img.resize((size, size), Image.LANCZOS)


def main():
    IMAGES_DIR.mkdir(parents=True, exist_ok=True)
    wallpaper().save(IMAGES_DIR / "bg_retro.png", optimize=True)
    default_cover().save(IMAGES_DIR / "cover_default.png", optimize=True)
    print(f"Wrote images to {IMAGES_DIR}")


if __name__ == "__main__":
    main()
