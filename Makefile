BASEROM ?= baserom.us.z64
TARGET  := NGVE
BUILD   := build
ROM     := $(BUILD)/$(TARGET).z64
ELF     := $(BUILD)/$(TARGET).elf
LD_SCRIPT := $(TARGET).ld

PYTHON  ?= python3
SPLAT   ?= $(PYTHON) tools/splat_run.py
CROSS   ?= mips-linux-gnu-

AS      := $(CROSS)as
LD      := $(CROSS)ld
OBJCOPY := $(CROSS)objcopy

GCC_DIR := tools/gcc-2.7.2
CC      := COMPILER_PATH=$(GCC_DIR) $(GCC_DIR)/gcc

IINC     := -Iinclude -Iinclude/PR -Isrc -I.
DEFINES  := -D_LANGUAGE_C -DF3DEX_GBI -D_FINALROM -DNDEBUG -DTARGET_N64
CFLAGS   := -G0 -mcpu=r4000 -mips2 -mmips-as -funsigned-char -Wa,-EB,-mips3,-mabi=32,-mgp32,-march=vr4300,-mfp32,-mno-shared,--no-pad-sections,-Iinclude
OPTFLAGS := -O2
ASFLAGS  := -EB -G0 -mips3 -mabi=32 -march=vr4300 -mno-shared --no-pad-sections -Iinclude
LDFLAGS  := -T $(LD_SCRIPT) -T undefined_syms_auto.txt -T undefined_funcs_auto.txt -T symbols.ld \
            -Map $(BUILD)/$(TARGET).map --no-check-sections

ALIGN_FIX := --set-section-alignment .text=4 --set-section-alignment .data=4 --set-section-alignment .bss=4 --set-section-alignment .rodata=4

OBJS = $(shell grep -oE 'build/[^ ()]+\.o' $(LD_SCRIPT) 2>/dev/null | sort -u)

all: check

check: $(ROM)
	@sha1sum -c $(TARGET).sha1

$(ROM): $(ELF)
	$(OBJCOPY) -O binary --pad-to=0x800000 --gap-fill=0x00 $< $@

$(ELF): $(OBJS) $(LD_SCRIPT) symbols.ld
	$(LD) $(LDFLAGS) -o $@

.SECONDEXPANSION:
$(BUILD)/src/%.o: src/%.c tools/asn64.py include/macro.inc $(wildcard include/*.h include/PR/*.h) $$(wildcard asm/nonmatchings/$$*/*.s)
	@mkdir -p $(dir $@)
	$(CC) -c $(CFLAGS) $(OPTFLAGS) $(IINC) $(DEFINES) $< -o $@
	$(OBJCOPY) $(ALIGN_FIX) $@

$(BUILD)/asm/%.o: asm/%.s include/macro.inc
	@mkdir -p $(dir $@)
	$(AS) $(ASFLAGS) $< -o $@
	$(OBJCOPY) $(ALIGN_FIX) $@

$(BUILD)/bin/%.o: bin/%.bin
	@mkdir -p $(dir $@)
	$(OBJCOPY) -I binary -O elf32-tradbigmips -B mips $< $@

setup:
	@$(PYTHON) -c "import splat, spimdisasm" 2>/dev/null || { echo "make setup: splat is not installed for $(PYTHON); activate the virtual environment first"; exit 1; }
	sed -e 's|^  target_path:.*|  target_path: $(abspath $(BASEROM))|' glover.yaml > .splat.yaml
	rm -rf asm bin
	$(SPLAT) split .splat.yaml
	rm -f .splat.yaml

toolchain:
	tools/setup_toolchain.sh

O0_FILES := decompress eeprom controller main jalloc text font banks sched
$(foreach f,$(O0_FILES),$(eval $(BUILD)/src/$(f).o: OPTFLAGS := -O0))

clean:
	rm -rf $(BUILD)

.PHONY: all check setup toolchain clean
.SUFFIXES:
MAKEFLAGS += --no-builtin-rules
