#!/bin/bash
# setup.sh: install mupen64plus (Ubuntu packages), build the angrylion software video plugin and the
# scriptinput plugin into tools/emu/build, and start a virtual display on :99 for emu.py.
set -e
here="$(cd "$(dirname "$0")" && pwd)"
build="$here/build"
mkdir -p "$build"
# Each package is checked on its own: a machine that already has the core may lack the others.
packages="libmupen64plus2 libmupen64plus-dev mupen64plus-rsp-z64 mupen64plus-rsp-hle mupen64plus-video-rice
    mupen64plus-audio-sdl libgl1-mesa-dri libgl-dev libglu1-mesa-dev xvfb cmake git"
missing=""
for p in $packages; do
    dpkg-query -W -f='${Status}' "$p" 2>/dev/null | grep -q "install ok installed" || missing="$missing $p"
done
if [ -n "$missing" ]; then
    sudo=""; [ "$(id -u)" = 0 ] || sudo=sudo
    $sudo apt-get update -qq
    DEBIAN_FRONTEND=noninteractive $sudo apt-get install -y -qq $missing
fi
# angrylion's RDP: a pixel-accurate software renderer (needs the low-level z64 RSP plugin)
if ! [ -e "$build/mupen64plus-video-angrylion-plus.so" ]; then
    [ -d "$build/angrylion-rdp-plus" ] || git clone -q --depth 1 https://github.com/ata4/angrylion-rdp-plus "$build/angrylion-rdp-plus"
    cmake -S "$build/angrylion-rdp-plus" -B "$build/angrylion-build" -DCMAKE_BUILD_TYPE=Release > /dev/null
    make -C "$build/angrylion-build" -j4 > /dev/null
    cp "$build/angrylion-build/mupen64plus-video-angrylion-plus.so" "$build/"
fi
cc -shared -fPIC -O2 -I/usr/include/mupen64plus -o "$build/scriptinput.so" "$here/scriptinput.c"
pgrep -x Xvfb > /dev/null || (Xvfb :99 -screen 0 1024x768x24 > /dev/null 2>&1 &)
echo "emu ready: python3 tools/emu/emu.py SCRIPT.py [--state FILE]"
