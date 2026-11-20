#!/usr/bin/env python3
# Usage: moolog.py LOGFILE [OUTDIR]
"""Turn the recordings in an IS-Viewer log back into .moo files.

A ROM made by debugpad2.py sends each recording as text: a MOOFILE line with the game's file
name, MOODATA lines of hexadecimal, and a MOOEND line. Emulators may add text in front of each
line. The words are searched for anywhere in the line. Each file is written to OUTDIR (the
current folder by default) under the last part of the game's name, such as ATLANTIS_1.moo.
"""
import os
import re
import sys

LINE = re.compile(r"\b(MOOFILE|MOODATA|MOOEND)\b ?(.*)")


def main():
    if len(sys.argv) not in (2, 3):
        sys.exit(__doc__.strip().splitlines()[0])
    outdir = sys.argv[2] if len(sys.argv) == 3 else "."
    name = None
    data = bytearray()
    with open(sys.argv[1], errors="replace") as log:
        for text in log:
            m = LINE.search(text)
            if not m:
                continue
            kind, rest = m.group(1), m.group(2).strip()
            if kind == "MOOFILE":
                name = rest.replace("\\", "/").split("/")[-1] or "record.moo"
                data = bytearray()
            elif kind == "MOODATA" and name:
                data += bytes.fromhex(rest)
            elif kind == "MOOEND" and name:
                path = os.path.join(outdir, name)
                with open(path, "wb") as f:
                    f.write(data)
                print(f"wrote {path} ({len(data)} bytes)")
                name = None


if __name__ == "__main__":
    main()
