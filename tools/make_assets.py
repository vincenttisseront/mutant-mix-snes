#!/usr/bin/env python3
"""Extract hero idles and paint level 1 (Égouts-Venin) for gfx4snes."""
from PIL import Image

SRC = "gfx/heroes-sheet.png"
HERO_ROWS = [
    (14, 63),    # Azu
    (142, 191),  # Kaeru
    (262, 319),  # Quilla
    (398, 447),  # Paddo
]


def is_bg(c):
    r, g, b = c
    return r > 220 and b > 220 and g < 40


def extract_hero(im, y0, y1, size=64):
    px = im.load()
    w, _ = im.size
    xs = [x for x in range(w) if any(not is_bg(px[x, y]) for y in range(y0, y1 + 1))]
    if not xs:
        raise SystemExit("empty hero crop")
    # First frame: stop at the first vertical gap after content starts.
    start = xs[0]
    end = start
    gap = 0
    for x in range(start, min(w, start + 72)):
        col = any(not is_bg(px[x, y]) for y in range(y0, y1 + 1))
        if col:
            end = x
            gap = 0
        else:
            gap += 1
            if gap > 3 and end > start + 8:
                break
    crop = im.crop((start, y0, end + 1, y1 + 1))
    canvas = Image.new("RGB", (size, size), (0, 0, 0))
    cw, ch = crop.size
    # Bottom-align, horizontally center.
    ox = max(0, (size - cw) // 2)
    oy = max(0, size - ch - 2)
    src = crop.load()
    dst = canvas.load()
    for y in range(ch):
        for x in range(cw):
            if y + oy >= size or x + ox >= size:
                continue
            if not is_bg(src[x, y]):
                dst[x + ox, y + oy] = src[x, y]
    return to_indexed(canvas)


def to_indexed(rgb):
    colors = {}
    px = rgb.load()
    w, h = rgb.size
    for y in range(h):
        for x in range(w):
            c = px[x, y]
            if c == (0, 0, 0):
                continue
            colors[c] = colors.get(c, 0) + 1
    top = [c for c, _ in sorted(colors.items(), key=lambda kv: -kv[1])[:15]]
    pal = [(0, 0, 0)] + top
    nearest = {}

    def quant(c):
        if c in nearest:
            return nearest[c]
        if c == (0, 0, 0):
            nearest[c] = 0
            return 0
        best, bd = 0, 1 << 30
        for i, p in enumerate(pal):
            if i == 0:
                continue
            d = (c[0] - p[0]) ** 2 + (c[1] - p[1]) ** 2 + (c[2] - p[2]) ** 2
            if d < bd:
                best, bd = i, d
        nearest[c] = best
        return best

    img = Image.new("P", rgb.size)
    img.putpalette([v for rgb in pal for v in rgb] + [0, 0, 0] * (256 - len(pal)))
    out = img.load()
    for y in range(h):
        for x in range(w):
            out[x, y] = quant(px[x, y])
    return img


def paint_level():
    tw, th = 64, 32
    # 0 empty, 1 solid, 2 hurt, 3 exit
    col = [[0] * tw for _ in range(th)]
    img = Image.new("RGB", (tw * 8, th * 8), (12, 8, 28))
    px = img.load()

    def brick(x, y, alt=False):
        base = (58, 42, 72) if not alt else (42, 32, 58)
        mortar = (28, 20, 36)
        for yy in range(8):
            for xx in range(8):
                c = mortar if yy == 7 or (xx == 0 and (y % 2 == 0)) or (xx == 4 and (y % 2)) else base
                if yy == 0:
                    c = (90, 70, 100)
                px[x * 8 + xx, y * 8 + yy] = c

    def venom(x, y):
        for yy in range(8):
            for xx in range(8):
                g = 180 + ((x * 3 + y + xx) % 5) * 12
                px[x * 8 + xx, y * 8 + yy] = (40, min(255, g), 30)

    def grate(x, y):
        for yy in range(8):
            for xx in range(8):
                c = (210, 180, 60) if xx % 3 == 0 else (20, 16, 32)
                px[x * 8 + xx, y * 8 + yy] = c

    def pipe(x, y):
        for yy in range(8):
            for xx in range(8):
                c = (40, 110, 70) if 1 <= xx <= 6 else (20, 60, 40)
                px[x * 8 + xx, y * 8 + yy] = c

    for y in range(24, 32):
        for x in range(tw):
            if 16 <= x <= 19 or 36 <= x <= 39:
                venom(x, y)
                col[y][x] = 2
            else:
                brick(x, y, alt=(x + y) % 2 == 0)
                col[y][x] = 1

    platforms = [
        (8, 14, 18),
        (22, 32, 15),
        (42, 52, 17),
        (54, 61, 13),
    ]
    for x0, x1, y in platforms:
        for x in range(x0, x1 + 1):
            brick(x, y, alt=False)
            col[y][x] = 1
            if y + 1 < 24:
                pipe(x, y + 1)
                col[y + 1][x] = 1

    for y in range(10, 24):
        grate(61, y)
        col[y][61] = 3
        brick(62, y)
        col[y][62] = 1
        brick(63, y)
        col[y][63] = 1

    # Ceiling pipes
    for x in range(0, tw, 6):
        pipe(x, 2)
        col[2][x] = 1

    img = to_indexed(img)
    return img, col


def paint_rat():
    img = Image.new("RGB", (32, 32), (0, 0, 0))
    px = img.load()
    body = (120, 80, 60)
    ear = (160, 90, 90)
    for y in range(14, 26):
        for x in range(6, 26):
            if (x - 16) ** 2 // 2 + (y - 20) ** 2 < 40:
                px[x, y] = body
    for y in range(8, 14):
        for x in range(18, 28):
            if (x - 23) ** 2 + (y - 12) ** 2 < 16:
                px[x, y] = body
    px[8, 12] = ear
    px[9, 11] = ear
    px[10, 12] = ear
    px[24, 12] = (20, 20, 20)
    return to_indexed(img)


def main():
    sheet = Image.open(SRC).convert("RGB")
    for i, (y0, y1) in enumerate(HERO_ROWS):
        hero = extract_hero(sheet, y0, y1)
        hero.save(f"gfx/hero{i}.png")
        print("hero", i, "colors", len({c for c in hero.getdata() if c}))
    level, col = paint_level()
    level.save("gfx/level1.png")
    flat = [col[y][x] for y in range(32) for x in range(64)]
    with open("src/level1_col.c", "w", encoding="utf-8") as f:
        f.write("/* generated by tools/make_assets.py */\n")
        f.write("const unsigned char level1_col[2048] = {\n")
        for i in range(0, len(flat), 32):
            f.write("  " + ",".join(str(v) for v in flat[i : i + 32]) + ",\n")
        f.write("};\n")
    paint_rat().save("gfx/rat.png")
    print("level + rat ok", sum(1 for v in flat if v))


if __name__ == "__main__":
    main()
