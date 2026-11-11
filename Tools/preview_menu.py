#!/usr/bin/env python3
"""Render the menu screens with the exact bitmaps used by the C code."""

from __future__ import annotations

import json
from pathlib import Path

from PIL import Image, ImageDraw, ImageFont

ROOT = Path(__file__).resolve().parents[1]
DATA = json.loads((ROOT / "tools" / "font_data.json").read_text(encoding="utf-8"))
ASCII = DATA["ascii"]
CHINESE = DATA["chinese"]
CN = DATA["cn_order"]


class OLED:
    def __init__(self) -> None:
        self.fb = bytearray(128 * 64)

    def pixel(self, x: int, y: int, color: int) -> None:
        if 0 <= x < 128 and 0 <= y < 64:
            self.fb[y * 128 + x] = 1 if color else 0

    def clear(self) -> None:
        self.fb = bytearray(128 * 64)

    def fill_rect(self, x: int, y: int, w: int, h: int, color: int) -> None:
        for yy in range(y, y + h):
            for xx in range(x, x + w):
                self.pixel(xx, yy, color)

    def hline(self, x: int, y: int, w: int, color: int) -> None:
        for xx in range(x, x + w):
            self.pixel(xx, y, color)

    def char(self, x: int, y: int, ch: str, color: int) -> None:
        glyph = ASCII.get(ch.upper() if ch.isalpha() else ch, ASCII["?"])
        for col in range(6):
            bits = glyph[col]
            for row in range(8):
                self.pixel(x + col, y + row, color if bits & (1 << row) else 1 - color)

    def string(self, x: int, y: int, text: str, color: int) -> int:
        for ch in text:
            self.char(x, y, ch, color)
            x += 6
        return x

    def cn(self, x: int, y: int, ch: str, color: int) -> None:
        glyph = CHINESE[ch]
        for row in range(11):
            bits = glyph[row * 2] | (glyph[row * 2 + 1] << 8)
            for col in range(11):
                self.pixel(x + col, y + row, color if bits & (1 << col) else 1 - color)

    def cn_string(self, x: int, y: int, text: str, color: int) -> int:
        for ch in text:
            self.cn(x, y, ch, color)
            x += 11
        return x

    def unsigned(self, x: int, y: int, value: int, digits: int, color: int) -> None:
        started = False
        for i in range(digits):
            place = 10 ** (digits - 1 - i)
            digit = (value // place) % 10
            if digit or i == digits - 1:
                started = True
            self.char(x + i * 6, y, str(digit) if started else " ", color)

    def arrow(self, x: int, y: int, color: int) -> None:
        cols = (0x40, 0x60, 0x70, 0x78, 0x70, 0x60, 0x40)
        for col in range(7):
            for row in range(7):
                lit = cols[col] & (1 << row)
                self.pixel(x + col, y + row, color if lit else 1 - color)

    def image(self, scale: int = 4) -> Image.Image:
        image = Image.new("1", (128, 64), 0)
        pixels = image.load()
        for y in range(64):
            for x in range(128):
                pixels[x, y] = self.fb[y * 128 + x]
        return image.resize((128 * scale, 64 * scale), Image.Resampling.NEAREST)


MENU = [
    ("LED ", ("控", "制")),
    ("", ("信", "息", "显", "示")),
    ("", ("巡", "线", "功", "能")),
    ("", ("拓", "展", "功", "能")),
]


def main_menu(cursor: int) -> OLED:
    o = OLED()
    o.cn_string(4, 0, "主菜单", 1)
    o.string(98, 2, "MENU", 1)
    o.hline(0, 12, 128, 1)
    for i, (prefix, ctext) in enumerate(MENU):
        y = 13 + i * 13
        selected = i == cursor
        if selected:
            o.fill_rect(0, y, 128, 12, 1)
        color = 0 if selected else 1
        if selected:
            o.arrow(3, y + 2, color)
        x = o.string(14, y + 2, prefix, color)
        o.cn_string(x, y, ctext, color)
    return o


def submenu(title_ascii: str, title_cn: tuple[str, ...], lines: list[tuple[str, str, int]]) -> OLED:
    o = OLED()
    x = o.string(2, 2, title_ascii, 1)
    o.cn_string(x, 0, title_cn, 1)
    o.hline(0, 13, 128, 1)
    y = 18
    for kind, text, value in lines:
        if kind == "s":
            o.string(2, y, text, 1)
        elif kind == "num":
            o.string(2, y, text, 1)
            o.unsigned(2 + len(text) * 6, y, value, 5, 1)
        y += 10
    return o


def screens() -> list[tuple[str, OLED]]:
    led = submenu(
        "LED ",
        ("控", "制"),
        [
            ("s", "STATE: ON", 0),
            ("s", "OUTPUT: PC13", 0),
            ("s", "KEY3: TOGGLE", 0),
            ("s", "KEY4: BACK", 0),
        ],
    )
    info = submenu(
        "",
        ("信", "息", "显", "示"),
        [
            ("s", "MCU: STM32F103", 0),
            ("s", "OLED: SSD1306", 0),
            ("num", "UPTIME:", 12),
            ("s", "KEY4: BACK", 0),
        ],
    )
    line = submenu(
        "",
        ("巡", "线", "功", "能"),
        [
            ("s", "LINE FOLLOWER", 0),
            ("s", "SENSOR: N/A", 0),
            ("s", "STATUS: IDLE", 0),
            ("s", "KEY4: BACK", 0),
        ],
    )
    extra = submenu(
        "",
        ("拓", "展", "功", "能"),
        [
            ("s", "EXTRA FEATURES", 0),
            ("num", "COUNT:", 0),
            ("s", "KEY3: +1", 0),
            ("s", "KEY4: BACK", 0),
        ],
    )
    return [
        ("power-on / KEY1 first item", main_menu(0)),
        ("after KEY2 / second item", main_menu(1)),
        ("KEY3 on LED", led),
        ("KEY3 on INFO", info),
        ("KEY3 on LINE", line),
        ("KEY3 on EXTRA", extra),
    ]


def main() -> None:
    scale = 4
    items = screens()
    pad = 18
    caption_h = 22
    width = 128 * scale * 2 + pad * 3
    height = (64 * scale + caption_h) * 3 + pad * 4
    sheet = Image.new("RGB", (width, height), (31, 35, 42))
    draw = ImageDraw.Draw(sheet)
    font = ImageFont.truetype(r"C:\Windows\Fonts\consola.ttf", 13)
    for index, (caption, oled) in enumerate(items):
        row, col = divmod(index, 2)
        x = pad + col * (128 * scale + pad)
        y = pad + row * (64 * scale + caption_h + pad)
        screen = oled.image(scale).convert("RGB")
        # A tiny bezel makes the 128x64 outline obvious.
        draw.rectangle((x - 3, y - 3, x + 128 * scale + 2, y + 64 * scale + 2), fill=(8, 9, 12))
        sheet.paste(screen, (x, y))
        draw.text((x, y + 64 * scale + 3), caption, font=font, fill=(190, 198, 210))
    out = ROOT / "docs" / "menu_preview.png"
    out.parent.mkdir(exist_ok=True)
    sheet.save(out)
    print(out)


if __name__ == "__main__":
    main()
