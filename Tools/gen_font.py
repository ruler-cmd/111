#!/usr/bin/env python3
"""Generate the small bitmap fonts used by the SSD1306 menu demo.

Outputs:
  Hardware/OLED_Font.c   - 5x7 ASCII (stored in 6x8 cells) + 12x12 Chinese
  Hardware/OLED_Font.h   - glyph count / Chinese glyph IDs
  tools/font_data.json   - same bitmaps for the Python preview renderer
"""

from __future__ import annotations

import json
from pathlib import Path

from PIL import Image, ImageDraw, ImageFont

ROOT = Path(__file__).resolve().parents[1]
FONT_PATH = Path(r"C:\Windows\Fonts\simhei.ttf")
CN_CHARS = "主菜单控制信息显示巡线功能拓展"
CN_SIZE = 11

# 5x7 glyphs.  '1' means a lit pixel.  Every glyph is exactly 5 columns x 7 rows.
ASCII_5X7 = {
    " ": ("00000",) * 7,
    "A": ("01110", "10001", "10001", "11111", "10001", "10001", "10001"),
    "B": ("11110", "10001", "10001", "11110", "10001", "10001", "11110"),
    "C": ("01110", "10001", "10000", "10000", "10000", "10001", "01110"),
    "D": ("11110", "10001", "10001", "10001", "10001", "10001", "11110"),
    "E": ("11111", "10000", "10000", "11110", "10000", "10000", "11111"),
    "F": ("11111", "10000", "10000", "11110", "10000", "10000", "10000"),
    "G": ("01110", "10001", "10000", "10111", "10001", "10001", "01111"),
    "H": ("10001", "10001", "10001", "11111", "10001", "10001", "10001"),
    "I": ("01110", "00100", "00100", "00100", "00100", "00100", "01110"),
    "J": ("00111", "00010", "00010", "00010", "00010", "10010", "01100"),
    "K": ("10001", "10010", "10100", "11000", "10100", "10010", "10001"),
    "L": ("10000", "10000", "10000", "10000", "10000", "10000", "11111"),
    "M": ("10001", "11011", "10101", "10101", "10001", "10001", "10001"),
    "N": ("10001", "11001", "10101", "10011", "10001", "10001", "10001"),
    "O": ("01110", "10001", "10001", "10001", "10001", "10001", "01110"),
    "P": ("11110", "10001", "10001", "11110", "10000", "10000", "10000"),
    "Q": ("01110", "10001", "10001", "10001", "10101", "10010", "01101"),
    "R": ("11110", "10001", "10001", "11110", "10100", "10010", "10001"),
    "S": ("01111", "10000", "10000", "01110", "00001", "00001", "11110"),
    "T": ("11111", "00100", "00100", "00100", "00100", "00100", "00100"),
    "U": ("10001", "10001", "10001", "10001", "10001", "10001", "01110"),
    "V": ("10001", "10001", "10001", "10001", "10001", "01010", "00100"),
    "W": ("10001", "10001", "10001", "10101", "10101", "11011", "10001"),
    "X": ("10001", "10001", "01010", "00100", "01010", "10001", "10001"),
    "Y": ("10001", "10001", "01010", "00100", "00100", "00100", "00100"),
    "Z": ("11111", "00001", "00010", "00100", "01000", "10000", "11111"),
    "0": ("01110", "10001", "10011", "10101", "11001", "10001", "01110"),
    "1": ("00100", "01100", "00100", "00100", "00100", "00100", "01110"),
    "2": ("01110", "10001", "00001", "00010", "00100", "01000", "11111"),
    "3": ("11110", "00001", "00001", "01110", "00001", "00001", "11110"),
    "4": ("00010", "00110", "01010", "10010", "11111", "00010", "00010"),
    "5": ("11111", "10000", "11110", "00001", "00001", "10001", "01110"),
    "6": ("00110", "01000", "10000", "11110", "10001", "10001", "01110"),
    "7": ("11111", "00001", "00010", "00100", "01000", "01000", "01000"),
    "8": ("01110", "10001", "10001", "01110", "10001", "10001", "01110"),
    "9": ("01110", "10001", "10001", "01111", "00001", "00010", "01100"),
    ":": ("00000", "00100", "00100", "00000", "00100", "00100", "00000"),
    "/": ("00001", "00010", "00010", "00100", "01000", "01000", "10000"),
    "+": ("00000", "00100", "00100", "11111", "00100", "00100", "00000"),
    "-": ("00000", "00000", "00000", "11111", "00000", "00000", "00000"),
    ".": ("00000", "00000", "00000", "00000", "00000", "00110", "00110"),
    "?": ("01110", "10001", "00001", "00010", "00100", "00000", "00100"),
}

CN_ENUM = (
    "CN_ZHU",
    "CN_CAI",
    "CN_DAN",
    "CN_KONG",
    "CN_ZHI",
    "CN_XIN",
    "CN_XI",
    "CN_XIAN_SHI",
    "CN_SHI",
    "CN_XUN",
    "CN_XIAN",
    "CN_GONG",
    "CN_NENG",
    "CN_TUO",
    "CN_ZHAN",
)


def ascii_bitmap(ch: str) -> list[int]:
    """Return 6 column bytes, LSB = top row."""
    key = ch.upper() if ch.isalpha() else ch
    rows = ASCII_5X7.get(key, ASCII_5X7["?"])
    cols: list[int] = []
    for col in range(5):
        value = 0
        for row in range(7):
            if rows[row][col] == "1":
                value |= 1 << row
        cols.append(value)
    cols.append(0)  # one blank column => 6 px advance
    return cols


def chinese_bitmap(ch: str) -> list[int]:
    """Return 22 bytes, row-major, two bytes per row, bit 0 = left pixel."""
    font = ImageFont.truetype(str(FONT_PATH), CN_SIZE)
    image = Image.new("L", (CN_SIZE, CN_SIZE), 0)
    draw = ImageDraw.Draw(image)
    draw.text((0, -1), ch, font=font, fill=255)
    data: list[int] = []
    for y in range(CN_SIZE):
        bits = 0
        for x in range(CN_SIZE):
            if image.getpixel((x, y)) >= 80:
                bits |= 1 << x
        data.append(bits & 0xFF)
        data.append((bits >> 8) & 0xFF)
    return data


def c_array(values: list[int], per_line: int = 12) -> str:
    lines = []
    for i in range(0, len(values), per_line):
        lines.append("    " + ", ".join(f"0x{v:02X}" for v in values[i : i + per_line]) + ",")
    return "\n".join(lines)


def main() -> None:
    ascii_table = [ascii_bitmap(chr(code)) for code in range(0x20, 0x7F)]
    cn_table = [chinese_bitmap(ch) for ch in CN_CHARS]

    font_c = [
        '#include "OLED_Font.h"',
        "",
        "/* 5x7 ASCII glyphs stored in 6x8 cells, column-major, LSB = top row. */",
        "const uint8_t OLED_Font6x8[95][6] = {",
    ]
    for code, glyph in zip(range(0x20, 0x7F), ascii_table):
        label = repr(chr(code)) if code != 0x27 else "'\\''"
        font_c.append(f"    /* 0x{code:02X} {label} */")
        font_c.append("    {" + ", ".join(f"0x{v:02X}" for v in glyph) + "},")
    font_c.append("};")
    font_c.append("")
    font_c.append("/* 11x11 SimHei glyphs, row-major, two bytes per row, bit 0 = left. */")
    font_c.append("const uint8_t OLED_Font11x11[CN_COUNT][22] = {")
    for index, glyph in enumerate(cn_table):
        font_c.append(f"    /* Chinese glyph {index} */")
        font_c.append("    {")
        font_c.append(c_array(glyph, 12))
        font_c.append("    },")
    font_c.append("};")
    font_c.append("")

    font_h = [
        "#ifndef OLED_FONT_H",
        "#define OLED_FONT_H",
        "",
        "#include <stdint.h>",
        "",
        "/* Number of Chinese glyphs in OLED_Font12x12. */",
        "typedef enum {",
    ]
    for name in CN_ENUM:
        font_h.append(f"    {name},")
    font_h.extend(
        [
            "    CN_COUNT",
            "} OLED_ChineseId;",
            "",
            "#define CN_END 0xFF",
            "",
            "extern const uint8_t OLED_Font6x8[95][6];",
            "extern const uint8_t OLED_Font11x11[CN_COUNT][22];",
            "",
            "#endif",
            "",
        ]
    )

    (ROOT / "Hardware" / "OLED_Font.c").write_text("\n".join(font_c), encoding="utf-8")
    (ROOT / "Hardware" / "OLED_Font.h").write_text("\n".join(font_h), encoding="utf-8")

    payload = {
        "ascii": {chr(code): ascii_table[i] for i, code in enumerate(range(0x20, 0x7F))},
        "chinese": {ch: cn_table[i] for i, ch in enumerate(CN_CHARS)},
        "cn_order": list(CN_CHARS),
    }
    (ROOT / "tools" / "font_data.json").write_text(json.dumps(payload), encoding="utf-8")

    # Print the Chinese glyphs as ASCII art so mistakes are easy to spot.
    for ch, glyph in zip(CN_CHARS, cn_table):
        print(f"\n{ch}")
        for row in range(CN_SIZE):
            bits = glyph[row * 2] | (glyph[row * 2 + 1] << 8)
            print("".join("#" if bits & (1 << col) else "." for col in range(CN_SIZE)))

    print(f"\nGenerated {len(ascii_table)} ASCII and {len(cn_table)} Chinese glyphs.")


if __name__ == "__main__":
    main()
