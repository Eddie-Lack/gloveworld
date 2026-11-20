#!/usr/bin/env python3
# Usage: tools/patches/mkpatchpage.py
"""Write the patch list from debugpad2.py into debugpatch.html. The page makes the same ROM.

Run it after any change to debugpad2.py or debugio_code.py.
"""
import json
import os
import re
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
import debugpad2  # noqa: E402

PAGE = os.path.join(HERE, "debugpatch.html")
rows = ",\n".join("  " + json.dumps(list(e)) for e in debugpad2.edits())
block = ("// EDITS BEGIN (written by mkpatchpage.py from debugpad2.py; do not edit by hand)\n"
         "// [ROM offset, original word or null, new word or null]\n"
         "const EDITS = [\n" + rows + "\n];\n// EDITS END")
with open(PAGE) as f:
    page = f.read()
page, n = re.subn(r"// EDITS BEGIN.*?// EDITS END", lambda m: block, page, flags=re.S)
if n != 1:
    sys.exit("debugpatch.html has no EDITS BEGIN ... EDITS END block")
with open(PAGE, "w") as f:
    f.write(page)
print(f"wrote {len(debugpad2.edits())} edits into {os.path.relpath(PAGE)}")
