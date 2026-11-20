#!/usr/bin/env python3
"""Run splat with doubles written as two .word directives.

GNU as auto-aligns `.double` to 8 bytes relative to the section start. The
original assembler only word-aligned doubles, so the ROM has doubles at
addresses that are 4 mod 8. Writing them as words keeps the bytes and avoids
the extra alignment.
"""
import sys

from spimdisasm.mips.symbols import MipsSymbolBase


def as_words(self, i):
    localOffset = 4 * i
    currentVram = self.getVramOffset(localOffset)
    currentVrom = self.getVromOffset(localOffset)
    label = ""
    if i != 0:
        label = self.getExtraLabelFromSymbol(self.getSymbol(currentVram, vromAddress=currentVrom, tryPlusOffset=False))
    hi, lo = self.words[i], self.words[i + 1]
    comment = self.generateAsmLineComment(localOffset, (hi << 32) | lo, isDouble=True)
    out = f"{label}{comment} .word 0x{hi:08X}, 0x{lo:08X}"
    out += self.getEndOfLineComment(i)
    out += "\n"
    return out, 1


MipsSymbolBase.SymbolBase.getNthWordAsDouble = as_words

from splat.__main__ import splat_main  # noqa: E402

splat_main()
