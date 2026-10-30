"""structwatch: follow a struct's fields in memory through scripted phases, to see what each field does.

    from structwatch import Watch
    w = Watch(emu, 'include/hand.h', 'HandControl', 0x8028FBD0)
    w.phase('idle'); yield 40       # every frame in between is sampled by w.sample() (call it each frame)
    print(w.report())

Fields come from the `/* 0xOFF */ type name;` lines of the struct's definition. Scalars, arrays of scalars,
Vec3 and pointers are followed; other nested structs are skipped.
"""
import os, re, struct

GW = os.path.abspath(os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..'))
SCALARS = {'s8': 'b', 'u8': 'B', 's16': 'h', 'u16': 'H', 's32': 'i', 'u32': 'I', 'f32': 'f', 'char': 'b'}


def parse_struct(header, name):
    text = open(os.path.join(GW, header)).read()
    m = re.search(r'typedef struct \w*\s*\{(.*?)\}\s*' + name + r'\s*;', text, re.S)
    if not m:
        raise KeyError(name)
    fields = []
    for off, decl in re.findall(r'/\* (0x[0-9A-Fa-f]+) \*/\s*([^;]+);', m.group(1)):
        dm = re.match(r'(?:struct\s+)?(\w+)\s*(\*?)\s*(\w+)((?:\[[^\]]+\])*)$', decl.strip())
        if not dm:
            continue
        typ, ptr, fname, dims = dm.groups()
        count = 1
        for d in re.findall(r'\[([^\]]+)\]', dims):
            count *= int(d, 0)
        off = int(off, 16)
        if ptr:
            fields += [(off + 4 * i, 'I', f'{fname}[{i}]' if count > 1 else fname) for i in range(count)]
        elif typ in SCALARS:
            size = struct.calcsize(SCALARS[typ])
            fields += [(off + size * i, SCALARS[typ], f'{fname}[{i}]' if count > 1 else fname) for i in range(count)]
        elif typ == 'Vec3':
            for i in range(count):
                base = f'{fname}[{i}]' if count > 1 else fname
                fields += [(off + 12 * i + 4 * k, 'f', f'{base}.{c}') for k, c in enumerate('xyz')]
    return fields


class Watch:
    def __init__(self, emu, header, name, addr):
        self.emu, self.addr, self.fields = emu, addr, parse_struct(header, name)
        self.phases = []  # (name, [samples]) ; a sample is a tuple of field values

    def phase(self, name):
        self.phases.append((name, []))

    def sample(self):
        if self.phases:
            self.phases[-1][1].append(tuple(self._value(off, fmt) for off, fmt, _ in self.fields))

    def _value(self, off, fmt):
        v = struct.unpack('>' + fmt, self.emu.read(self.addr + off, struct.calcsize(fmt)))[0]
        return round(v, 3) if fmt == 'f' else v

    def report(self, limit=6):
        """For each field that changes anywhere: its values in each phase (first LIMIT distinct ones)."""
        lines = []
        for i, (off, fmt, name) in enumerate(self.fields):
            per = []
            for pname, samples in self.phases:
                seq = []
                for s in samples:
                    if not seq or seq[-1] != s[i]:
                        seq.append(s[i])
                per.append((pname, seq))
            if len({v for _, seq in per for v in seq}) <= 1:
                continue
            fmt_v = lambda v: (f'0x{v:08X}' if fmt == 'I' else str(v))
            parts = []
            for pname, seq in per:
                shown = ' > '.join(fmt_v(v) for v in seq[:limit]) + (' ...' if len(seq) > limit else '')
                parts.append(f'{pname}: {shown}')
            lines.append(f'0x{off:03X} {name}\n    ' + '\n    '.join(parts))
        return '\n'.join(lines)
