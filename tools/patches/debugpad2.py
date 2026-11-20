#!/usr/bin/env python3
# Usage: debugpad2.py BASEROM OUTROM [--ips PATCH.ips]
"""Turn on Glover's debug controls on controller 2, and send debug output to the IS-Viewer.

The game keeps five Pad entries but reads only the active controller (pad[0]). The debug
handler ProcessInputPersonal and the title screen's demo mode switch read pad[4], which the
game never fills. This script:

1. points every pad[4] access in ProcessInputPersonal and MainTitleWait at pad[1];
2. writes the code from debugio.c (see debugio_code.py) over ScreenShot, which the game never
   calls, and over PCcreat and PCclose, the functions it replaces;
3. makes ControllerProc call DebugReadController2 where it called EndTimer(2). Controller 2
   is read every frame, just after controller 1;
4. makes debugPrintf and PCwrite jump to their replacements. PRINTF messages and input
   recordings go to the IS-Viewer instead of the development PC;
5. recomputes the header checksum.

BASEROM must be the USA ROM (NGVE). It may be in .z64, .v64 or .n64 byte order. OUTROM is
written in .z64 order. --ips also writes an IPS patch made from the two ROMs.
"""
import argparse
import hashlib
import os
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from debugio_code import BLOCKS, SYMBOLS  # noqa: E402

ROM_SHA1 = "270be17b3c8da9b88a7b99c2a545b0bce16837f4"  # NGVE.sha1

PAD = 0x801FAA90
PAD_SIZE = 0x190
VRAM_TO_ROM = 0x800FF000  # vram minus ROM offset in the main code segment

END_TIMER = 0x80180534
SCREEN_SHOT = 0x80181B4C
DEBUG_PRINTF = 0x80139D70
PC_WRITE = 0x8010D728
CONTROLLER_PROC_END_TIMER_CALL = 0x8013E078


def jal(target):
    return 0x0C000000 | ((target >> 2) & 0x03FFFFFF)


def j(target):
    return 0x08000000 | ((target >> 2) & 0x03FFFFFF)


# ROM offset, original word, patched word: the low half of each pad[4] address becomes the
# low half of the same field in pad[1]. The high halves (0x8020) do not change.
PAD_SITES = [
    (0x81B7C, 0x8C42B110, 0x8C42AC60),  # ProcessInputPersonal
    (0x81C08, 0x9042B111, 0x9042AC61),
    (0x81CF0, 0x9042B110, 0x9042AC60),
    (0x8215C, 0x9042B10A, 0x9042AC5A),
    (0x8219C, 0x9042B108, 0x9042AC58),
    (0x821E0, 0x9042B0EC, 0x9042AC3C),
    (0x821F0, 0x8442B0D0, 0x8442AC20),
    (0x8222C, 0x8442B0D0, 0x8442AC20),
    (0x82268, 0x8442B0D2, 0x8442AC22),
    (0x822A4, 0x8442B0D2, 0x8442AC22),
    (0x822E0, 0x9042B0EB, 0x9042AC3B),
    (0x82318, 0x9042B0EA, 0x9042AC3A),
    (0x82338, 0x8442B0D0, 0x8442AC20),
    (0x82370, 0x8442B0D0, 0x8442AC20),
    (0x823A8, 0x8442B0D2, 0x8442AC22),
    (0x823E0, 0x8442B0D2, 0x8442AC22),
    (0x82418, 0x9042B0EB, 0x9042AC3B),
    (0x82450, 0x9042B0EA, 0x9042AC3A),
    (0x824E0, 0x9042B10C, 0x9042AC5C),
    (0x82534, 0x9042B0F1, 0x9042AC41),
    (0x82580, 0x9042B10D, 0x9042AC5D),
    (0x825D0, 0x9042B0F2, 0x9042AC42),
    (0x825F0, 0x9042B0F3, 0x9042AC43),
    (0x82774, 0x9042B10B, 0x9042AC5B),
    (0x82808, 0x9042B0EA, 0x9042AC3A),
    (0x82818, 0x9042B107, 0x9042AC57),
    (0x82998, 0x9042B0F6, 0x9042AC46),
    (0x829D8, 0x9042B0F7, 0x9042AC47),
    (0x829FC, 0x9042B109, 0x9042AC59),
    (0x1522C, 0x9042B108, 0x9042AC58),  # MainTitleWait
]

# Original words of ScreenShot that hold no relocated address. The SHA-1 check covers the rest.
SCREEN_SHOT_KNOWN = {0: 0x27BDFA90, 4: 0x27A70520, 16: 0x00E61025, 20: 0x30420003, 24: 0xAFB40568}

# Each entry jumps to its replacement; the delay slot is a nop. The original words of PCwrite are not
# known without the ROM's disassembly.
JUMPS = [
    (DEBUG_PRINTF, (0x27BDFFF8, 0xAFBE0000), SYMBOLS["IsvDebugPrintf"]),
    (PC_WRITE, (None, None), SYMBOLS["IsvPCwrite"]),
]

CIC_SEEDS = {"6102": 0xF8CA4DDC, "6103": 0xA3886759, "6105": 0xDF26F436, "6106": 0x1FEA617A}
M = 0xFFFFFFFF


def to_z64(rom):
    magic = rom[:4]
    if magic == b"\x80\x37\x12\x40":
        return bytearray(rom)
    out = bytearray(len(rom))
    if magic == b"\x37\x80\x40\x12":  # .v64: bytes swapped in pairs
        out[0::2], out[1::2] = rom[1::2], rom[0::2]
        return out
    if magic == b"\x40\x12\x37\x80":  # .n64: little-endian words
        for i in range(4):
            out[i::4] = rom[3 - i::4]
        return out
    sys.exit("not an N64 ROM: unknown byte order")


def word(rom, off):
    return int.from_bytes(rom[off:off + 4], "big")


def put_word(rom, off, value):
    rom[off:off + 4] = value.to_bytes(4, "big")


def checksum(rom, cic):
    t1 = t2 = t3 = t4 = t5 = t6 = CIC_SEEDS[cic]
    for i in range(0x1000, 0x101000, 4):
        d = word(rom, i)
        if (t6 + d) & M < t6:
            t4 = (t4 + 1) & M
        t6 = (t6 + d) & M
        t3 ^= d
        s = d & 0x1F
        r = ((d << s) | (d >> (32 - s))) & M if s else d
        t5 = (t5 + r) & M
        t2 ^= r if t2 > d else t6 ^ d
        if cic == "6105":
            t1 = (t1 + (word(rom, 0x750 + (i & 0xFF)) ^ d)) & M
        else:
            t1 = (t1 + (t5 ^ d)) & M
    if cic == "6103":
        return ((t6 ^ t4) + t3) & M, ((t5 ^ t2) + t1) & M
    if cic == "6106":
        return (t6 * t4 + t3) & M, (t5 * t2 + t1) & M
    return t6 ^ t4 ^ t3, t5 ^ t2 ^ t1


def find_cic(rom):
    header = (word(rom, 0x10), word(rom, 0x14))
    for cic in CIC_SEEDS:
        if checksum(rom, cic) == header:
            return cic
    sys.exit("the header checksum does not match any known boot chip")


def write_ips(path, old, new):
    out = bytearray(b"PATCH")
    i = 0
    while i < len(new):
        if old[i] == new[i]:
            i += 1
            continue
        start = i
        while i < len(new) and old[i] != new[i] and i - start < 0xFFFF:
            i += 1
        if start == 0x454F46:  # an offset that reads "EOF" would end the patch early
            start -= 1
        out += start.to_bytes(3, "big") + (i - start).to_bytes(2, "big") + new[start:i]
    out += b"EOF"
    with open(path, "wb") as f:
        f.write(out)


def edits():
    """(ROM offset, original word or None, new word or None) for every word the patch checks or writes."""
    out = list(PAD_SITES)
    out.append((CONTROLLER_PROC_END_TIMER_CALL - VRAM_TO_ROM, jal(END_TIMER), jal(SYMBOLS["DebugReadController2"])))
    for addr, old, target in JUMPS:
        out.append((addr - VRAM_TO_ROM, old[0], j(target)))
        out.append((addr + 4 - VRAM_TO_ROM, old[1], 0))
    for off, old in SCREEN_SHOT_KNOWN.items():
        out.append((SCREEN_SHOT + off - VRAM_TO_ROM, old, None))
    for vram, words in BLOCKS:
        out += [(vram + 4 * i - VRAM_TO_ROM, None, w) for i, w in enumerate(words)]
    return out


def main():
    ap = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    ap.add_argument("baserom")
    ap.add_argument("outrom")
    ap.add_argument("--ips", help="also write an IPS patch")
    args = ap.parse_args()

    with open(args.baserom, "rb") as f:
        rom = to_z64(f.read())
    if hashlib.sha1(rom).hexdigest() != ROM_SHA1:
        sys.exit("this is not the expected ROM: Glover (USA) must have SHA-1 " + ROM_SHA1)
    original = bytes(rom)
    cic = find_cic(rom)

    for off, old, new in edits():
        if old is not None and word(original, off) != old:
            sys.exit(f"unexpected word 0x{word(original, off):08X} at 0x{off:X} (expected 0x{old:08X})")
        if new is not None:
            put_word(rom, off, new)

    crc1, crc2 = checksum(rom, cic)
    put_word(rom, 0x10, crc1)
    put_word(rom, 0x14, crc2)

    with open(args.outrom, "wb") as f:
        f.write(rom)
    if args.ips:
        write_ips(args.ips, original, rom)
    changed = sum(word(rom, i) != word(original, i) for i in range(0, len(rom), 4)) - 2
    print(f"wrote {args.outrom} ({changed} words changed, CIC-{cic} checksum updated)")


if __name__ == "__main__":
    main()
