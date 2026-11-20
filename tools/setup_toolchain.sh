#!/bin/bash
# Builds the compilers into tools/.
set -e
cd "$(dirname "$0")"

clone() {
    if [ ! -d "$2" ]; then
        git clone https://github.com/$1 "$2"
        git -C "$2" checkout -q "$3"
    fi
}

clone decompals/mips-gcc-2.7.2 mips-gcc-2.7.2 43d1cdb67ed135879869b5266f01efaaada5e35a
clone decompals/SN64-gcc SN64-gcc 2c037f330a99b514e5b53c66f1c63f90d8a15302

# The old GCC Makefiles rely on make's built-in .c and .o suffixes. Some setups turn
# them off (for example MAKEFLAGS=-r), which skips every .c.o rule; this restores them.
SUFFIXES_MK=$(pwd)/gcc-suffixes.mk
printf '.SUFFIXES: .c .o\n' > "$SUFFIXES_MK"

backdate() {
    # Keep the shipped generated parsers newer than their grammar sources.
    find "$1" \( -name '*.y' -o -name '*.gperf' \) -exec touch -d '2000-01-01' {} +
}

# GCC 2.7.2: the compiler used by the build.
if [ ! -x gcc-2.7.2/cc1 ]; then
    backdate mips-gcc-2.7.2
    (cd mips-gcc-2.7.2 && ./configure --target=mips-mips-gnu --prefix=/opt/cross --with-gnu-as \
        --disable-gprof --disable-gdb --disable-werror --host=i386-pc-linux --build=i386-pc-linux)
    touch mips-gcc-2.7.2/c-parse.c
    make -C mips-gcc-2.7.2 -f Makefile -f "$SUFFIXES_MK" cpp cc1 xgcc CFLAGS="-static -std=gnu89 -m32"
    mkdir -p gcc-2.7.2
    cp mips-gcc-2.7.2/cpp mips-gcc-2.7.2/cc1 gcc-2.7.2/
    cp mips-gcc-2.7.2/xgcc gcc-2.7.2/gcc
fi

# SN64 GCC 2.7.2-970404: kept for comparison only (tools/cmpfunc.py --configs).
if [ ! -x sn64/cc1 ]; then
    backdate SN64-gcc/2.7.2-970404
    make -C SN64-gcc/2.7.2-970404 -f Makefile -f "$SUFFIXES_MK" cpp cc1 xgcc CFLAGS="-static -m32 -std=gnu89 -O2 -DHAVE_VPRINTF"
    mkdir -p sn64
    cp SN64-gcc/2.7.2-970404/cpp SN64-gcc/2.7.2-970404/cc1 sn64/
    cp SN64-gcc/2.7.2-970404/xgcc sn64/gcc
fi

for d in gcc-2.7.2 sn64; do
cat > $d/as <<'EOS'
#!/bin/bash
exec python3 "$(dirname -- "$0")/../asn64.py" mips-linux-gnu-as "$@"
EOS
chmod +x $d/as
done
