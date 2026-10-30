#!/usr/bin/env python3
"""Drive the game in mupen64plus from a script: buttons, memory reads and writes, screenshots, savestates.

    python3 tools/emu/emu.py SCRIPT.py [--state FILE] [--gfx angrylion|rice] [--rom FILE] [--debug]

SCRIPT.py defines run(emu), a generator. Each `yield N` lets N frames (vertical interrupts) pass.
Between yields it can call:
    emu.hold('A', 'START', x=0, y=0)  set the buttons held from now on (names in BUTTON_BITS)
    emu.release()                     release everything
    emu.r8/r16/r32/rf(addr)           read memory (N64 address, such as 0x8011E653)
    emu.w8/w16/w32/wf(addr, value)    write memory
    emu.shot(name)                    save a PNG of the current frame to the output folder
    emu.save(name)                    save a mupen64plus savestate (.st) to the output folder
    emu.load(path)                    load a savestate; it takes effect at the next yield
    emu.cheat(name, [(code, value)])  enable a GameShark code through the core's cheat engine (needed
                                      for writes to code, which the recompiler would otherwise miss)
    emu.break_at(addr, fn)            call fn(emu, regs) each time the CPU is about to run the instruction at addr
                                      (regs: the 32 general registers, as unsigned 32-bit numbers). Needs --debug.
    emu.frame                         frames since the start of the run
--debug turns on the core's debugger, which runs the CPU in the interpreter (about 5 times slower) and allows
break_at. Needs setup.sh to have run, and a display (setup.sh starts Xvfb on :99).
"""
import argparse, ctypes, hashlib, os, runpy, shutil, struct, sys, zlib

HERE = os.path.dirname(os.path.abspath(__file__))
GW = os.path.abspath(os.path.join(HERE, '..', '..'))
CORE = os.environ.get('M64_CORE', '/usr/lib/x86_64-linux-gnu/libmupen64plus.so.2')
PLUGINS = os.environ.get('M64_PLUGINS', '/usr/lib/x86_64-linux-gnu/mupen64plus')
BUILD = os.path.join(HERE, 'build')

BUTTON_BITS = {'RIGHT': 0, 'LEFT': 1, 'DOWN': 2, 'UP': 3, 'START': 4, 'Z': 5, 'B': 6, 'A': 7,
               'CRIGHT': 8, 'CLEFT': 9, 'CDOWN': 10, 'CUP': 11, 'R': 12, 'L': 13}

# m64p_types.h
CMD_ROM_OPEN, CMD_ROM_CLOSE, CMD_EXECUTE, CMD_STOP = 1, 2, 5, 6
CMD_STATE_QUERY, CMD_STATE_LOAD, CMD_STATE_SAVE = 9, 10, 11
CMD_SET_FRAME_CALLBACK, CMD_STATE_SET, CMD_READ_SCREEN = 15, 17, 18
CORE_VIDEO_SIZE, CORE_SPEED_LIMITER = 6, 5
DATADIR = '/usr/share/games/mupen64plus'
RETAIL_MD5 = '87AA5740DFF79291EE97832DA1F86205'  # the retail ROM; mupen64plus's ini knows its save type
PLUGIN_RSP, PLUGIN_GFX, PLUGIN_AUDIO, PLUGIN_INPUT = 1, 2, 3, 4
TYPE_INT, TYPE_BOOL, TYPE_STRING = 1, 3, 4
DBG_PTR_RDRAM = 1
DBG_RUNSTATE_RUNNING, CPU_REG_REG = 2, 2
BKP_CMD_ADD_STRUCT, BKP_FLAGS_EXEC = 2, 0x01 | 0x08  # enabled + execute

DEBUGCB = ctypes.CFUNCTYPE(None, ctypes.c_void_p, ctypes.c_int, ctypes.c_char_p)
STATECB = ctypes.CFUNCTYPE(None, ctypes.c_void_p, ctypes.c_int, ctypes.c_int)
FRAMECB = ctypes.CFUNCTYPE(None, ctypes.c_uint)
DBGINITCB = ctypes.CFUNCTYPE(None)
DBGUPDATECB = ctypes.CFUNCTYPE(None, ctypes.c_uint)


class Breakpoint(ctypes.Structure):
    _fields_ = [('address', ctypes.c_uint), ('endaddr', ctypes.c_uint), ('flags', ctypes.c_uint)]


class CheatCode(ctypes.Structure):
    _fields_ = [('address', ctypes.c_uint), ('value', ctypes.c_int)]


def write_png(path, w, h, rgb):
    """RGB rows bottom-up, as READ_SCREEN returns them."""
    rows = b''.join(b'\0' + rgb[(h - 1 - y) * w * 3:(h - y) * w * 3] for y in range(h))
    chunk = lambda t, d: struct.pack('>I', len(d)) + t + d + struct.pack('>I', zlib.crc32(t + d) & 0xffffffff)
    with open(path, 'wb') as f:
        f.write(b'\x89PNG\r\n\x1a\n' + chunk(b'IHDR', struct.pack('>IIBBBBB', w, h, 8, 2, 0, 0, 0))
                + chunk(b'IDAT', zlib.compress(rows, 6)) + chunk(b'IEND', b''))


def data_dir_for(md5):
    """The core's data folder. A ROM that differs from the retail one (a patched ROM) has no entry in the core's ini
    and would get default settings, so a folder with an extra entry that refers to the retail ROM's is made for it."""
    if md5 == RETAIL_MD5:
        return DATADIR
    d = os.path.join(BUILD, 'data-' + md5)
    if not os.path.exists(os.path.join(d, 'mupen64plus.ini')):
        os.makedirs(d, exist_ok=True)
        for f in os.listdir(DATADIR):
            if f != 'mupen64plus.ini' and not os.path.exists(os.path.join(d, f)):
                os.symlink(os.path.join(DATADIR, f), os.path.join(d, f))
        ini = open(os.path.join(DATADIR, 'mupen64plus.ini'), encoding='latin-1').read()
        open(os.path.join(d, 'mupen64plus.ini'), 'w', encoding='latin-1').write(
            ini + f'\n[{md5}]\nGoodName=Glover (U) [patched]\nRefMD5={RETAIL_MD5}\n')
    return d


def state_for(path, md5):
    """A savestate records the ROM's MD5 and the core refuses it for another ROM. For a patched ROM this returns a
    copy of the state that holds the patched ROM's MD5 (the state is otherwise unchanged)."""
    if md5 == RETAIL_MD5:
        return path
    raw = zlib.decompress(open(path, 'rb').read(), 31)
    if raw[:8] != b'M64+SAVE':
        return path
    out = os.path.join(BUILD, 'patched-' + md5[:8] + '-' + os.path.basename(path))
    c = zlib.compressobj(6, zlib.DEFLATED, 31)
    open(out, 'wb').write(c.compress(raw[:12] + md5.encode() + raw[44:]) + c.flush())
    return out


class Emu:
    def __init__(self, rom, gfx, out, verbose=False, debug=False):
        self.out = out
        self.debug = debug
        data = open(rom, 'rb').read()
        self.md5 = hashlib.md5(data).hexdigest().upper()
        self._breaks = {}
        os.makedirs(out, exist_ok=True)
        self.verbose = verbose
        self.core = ctypes.CDLL(CORE)
        self._debugcb = DEBUGCB(self._debug)
        self._statecb = STATECB(lambda ctx, p, v: None)
        self._framecb = FRAMECB(self._frame)
        cfg = os.path.join(HERE, 'build', 'cfg')
        os.makedirs(cfg, exist_ok=True)
        self._check(self.core.CoreStartup(0x020001, cfg.encode(), data_dir_for(self.md5).encode(), None,
                                          self._debugcb, None, self._statecb), 'CoreStartup')
        if debug:
            handle = ctypes.c_void_p()
            self._check(self.core.ConfigOpenSection(b'Core', ctypes.byref(handle)), 'config Core')
            on = ctypes.c_int(1)
            # breakpoints are only checked by the pure interpreter (R4300Emulator 0)
            self._check(self.core.ConfigSetParameter(handle, b'R4300Emulator', TYPE_INT, ctypes.byref(ctypes.c_int(0))), 'R4300Emulator')
            self._check(self.core.ConfigSetParameter(handle, b'EnableDebugger', TYPE_BOOL, ctypes.byref(on)), 'EnableDebugger')
            self._dbginit, self._dbgupdate, self._dbgvi = DBGINITCB(lambda: None), DBGUPDATECB(self._dbg_update), DBGINITCB(lambda: None)
            self.core.DebugGetCPUDataPtr.restype = ctypes.c_void_p
            self._check(self.core.DebugSetCallbacks(self._dbginit, self._dbgupdate, self._dbgvi), 'DebugSetCallbacks')
        self._rom = ctypes.create_string_buffer(data, len(data))
        self._check(self.core.CoreDoCommand(CMD_ROM_OPEN, len(data), self._rom), 'ROM open')
        if gfx == 'angrylion':
            plugins = [(PLUGIN_GFX, os.path.join(BUILD, 'mupen64plus-video-angrylion-plus.so')),
                       (PLUGIN_RSP, os.path.join(PLUGINS, 'mupen64plus-rsp-z64.so'))]
        else:
            plugins = [(PLUGIN_GFX, os.path.join(PLUGINS, 'mupen64plus-video-rice.so')),
                       (PLUGIN_RSP, os.path.join(PLUGINS, 'mupen64plus-rsp-hle.so'))]
        # The game's main loop waits for audio DMA completions. Without an audio plugin the core never
        # answers them, frameCount stays at 0 and no input is read. SDL's dummy driver gives silent output.
        plugins.insert(1, (PLUGIN_AUDIO, os.path.join(PLUGINS, 'mupen64plus-audio-sdl.so')))
        plugins.insert(2, (PLUGIN_INPUT, os.path.join(BUILD, 'scriptinput.so')))
        self.libs = []
        for kind, path in plugins:
            lib = ctypes.CDLL(path)
            lib.PluginStartup(ctypes.c_void_p(self.core._handle), None, self._debugcb)
            self._check(self.core.CoreAttachPlugin(kind, ctypes.c_void_p(lib._handle)), 'attach ' + path)
            self.libs.append(lib)
            if kind == PLUGIN_INPUT:
                self.input = lib
        self.input.scriptinput_set.argtypes = [ctypes.c_int, ctypes.c_uint]
        self.core.DebugMemGetPointer.restype = ctypes.c_void_p
        self.frame = 0
        self._ram = None
        self._buttons = 0
        self._wait = 0
        self._gen = None
        self.error = None

    def _check(self, rc, what):
        if rc:
            raise RuntimeError(f'{what} failed: error {rc}')

    def _debug(self, ctx, level, msg):
        if self.verbose or level <= 2:
            print('m64p:', msg.decode(errors='replace'), file=sys.stderr)

    # memory: RDRAM is stored as host-endian 32-bit words. Byte A of the N64's big-endian view is at A ^ 3
    def _addr(self, a):
        a &= 0x1FFFFFFF
        if a >= 0x800000:
            raise ValueError(hex(a))
        return a

    def _bytes(self, a, n):
        a = self._addr(a)
        return bytes(self._ram[(a + i) ^ 3] for i in range(n))

    def _put(self, a, data):
        a = self._addr(a)
        for i, b in enumerate(data):
            self._ram[(a + i) ^ 3] = b

    def r8(self, a): return self._bytes(a, 1)[0]
    def r16(self, a): return struct.unpack('>H', self._bytes(a, 2))[0]
    def r32(self, a): return struct.unpack('>I', self._bytes(a, 4))[0]
    def rf(self, a): return struct.unpack('>f', self._bytes(a, 4))[0]
    def read(self, a, n): return self._bytes(a, n)
    def w8(self, a, v): self._put(a, struct.pack('>B', v & 0xFF))
    def w16(self, a, v): self._put(a, struct.pack('>H', v & 0xFFFF))
    def w32(self, a, v): self._put(a, struct.pack('>I', v & 0xFFFFFFFF))
    def wf(self, a, v): self._put(a, struct.pack('>f', v))

    def hold(self, *names, x=0, y=0):
        v = 0
        for n in names:
            v |= 1 << BUTTON_BITS[n.upper()]
        v |= (x & 0xFF) << 16 | (y & 0xFF) << 24
        self.input.scriptinput_set(0, v)

    def release(self):
        self.input.scriptinput_set(0, 0)

    def shot(self, name):
        size = ctypes.c_int()
        self.core.CoreDoCommand(CMD_STATE_QUERY, CORE_VIDEO_SIZE, ctypes.byref(size))
        w, h = size.value >> 16, size.value & 0xFFFF
        buf = ctypes.create_string_buffer(w * h * 3)
        self.core.CoreDoCommand(CMD_READ_SCREEN, 0, buf)
        path = os.path.join(self.out, name + '.png')
        write_png(path, w, h, buf.raw)
        return path

    def save(self, name):
        path = os.path.join(self.out, name + '.st')
        self.core.CoreDoCommand(CMD_STATE_SAVE, 1, path.encode())
        return path

    def load(self, path):
        """Load a savestate during a run. The core applies it before the next frame; yield at least once after."""
        self._check(self.core.CoreDoCommand(CMD_STATE_LOAD, 0, os.path.abspath(state_for(path, self.md5)).encode()), 'state load')

    def cheat(self, name, codes):
        arr = (CheatCode * len(codes))(*[CheatCode(a, v) for a, v in codes])
        self._check(self.core.CoreAddCheat(name.encode(), arr, len(codes)), 'cheat ' + name)

    def break_at(self, addr, fn):
        """Call fn(emu, regs) each time the CPU is about to run the instruction at addr (needs debug=True)."""
        if not self.debug:
            raise RuntimeError('break_at needs --debug')
        self._breaks[addr] = fn
        bp = Breakpoint(addr, addr, BKP_FLAGS_EXEC)
        if self.core.DebugBreakpointCommand(BKP_CMD_ADD_STRUCT, 0, ctypes.byref(bp)) < 0:
            raise RuntimeError(f'breakpoint at {addr:#x} failed')

    def _dbg_update(self, pc):
        fn = self._breaks.get(pc)
        if fn:
            regs = ctypes.cast(self.core.DebugGetCPUDataPtr(CPU_REG_REG), ctypes.POINTER(ctypes.c_uint64))
            fn(self, [regs[i] & 0xFFFFFFFF for i in range(32)])
        self.core.DebugSetRunState(DBG_RUNSTATE_RUNNING)

    def _frame(self, index):
        try:
            if self._ram is None:
                p = self.core.DebugMemGetPointer(DBG_PTR_RDRAM)
                self._ram = (ctypes.c_ubyte * 0x800000).from_address(p)
            self.frame += 1
            if self._wait > 0:
                self._wait -= 1
                return
            n = next(self._gen)
            self._wait = max(int(n or 1), 1) - 1
        except StopIteration:
            self.core.CoreDoCommand(CMD_STOP, 0, None)
        except Exception as e:
            self.error = e
            self.core.CoreDoCommand(CMD_STOP, 0, None)

    def run(self, script, state=None):
        self._gen = script(self)
        self.core.CoreDoCommand(CMD_SET_FRAME_CALLBACK, 0, self._framecb)
        off = ctypes.c_int(0)
        self.core.CoreDoCommand(CMD_STATE_SET, CORE_SPEED_LIMITER, ctypes.byref(off))
        if state:
            self._check(self.core.CoreDoCommand(CMD_STATE_LOAD, 0, os.path.abspath(state_for(state, self.md5)).encode()), 'state load')
        self.core.CoreDoCommand(CMD_EXECUTE, 0, None)
        self.core.CoreDoCommand(CMD_ROM_CLOSE, 0, None)
        for kind in (PLUGIN_RSP, PLUGIN_INPUT, PLUGIN_AUDIO, PLUGIN_GFX):
            self.core.CoreDetachPlugin(kind)
        self.core.CoreShutdown()
        if self.error:
            raise self.error


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('script')
    ap.add_argument('--state')
    ap.add_argument('--gfx', default='angrylion', choices=['angrylion', 'rice'])
    ap.add_argument('--rom', default=os.environ.get('BASEROM', os.path.join(GW, 'baserom.us.z64')))
    ap.add_argument('--out', default=os.path.join(HERE, 'build', 'out'))
    ap.add_argument('--verbose', action='store_true')
    ap.add_argument('--debug', action='store_true', help='enable the core debugger (needed for break_at)')
    a = ap.parse_args()
    os.environ.setdefault('DISPLAY', ':99')
    os.environ.setdefault('SDL_AUDIODRIVER', 'dummy')
    emu = Emu(a.rom, a.gfx, a.out, a.verbose, a.debug)
    emu.run(runpy.run_path(a.script)['run'], a.state)


if __name__ == '__main__':
    main()
