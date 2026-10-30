#!/usr/bin/env python3
# Adapted from modern-asn64.py in RocketRet/modern-asn64 (https://github.com/RocketRet/modern-asn64, commit
# 20d21729ce6a2e7bc8a4a91ec9505cab0d6e50f9), which its author released into the public domain under the Unlicense
# (https://unlicense.org). The licence text is in THIRD_PARTY_NOTICES.md. The changes made for this project are
# described in docs/building.md, "Assembler".
# Preprocesses an input asm file to match asn64's reordering and macro expansion, then passes it to the provided assembler.

import sys
import re
import struct
import subprocess
from typing import *

assembler: str = sys.argv[1]
input_filename_index: int = -1

# Find the input file in the argument list
arg_index: int = 2
args_to_remove: List[int] = []
# Use a while loop. Indices can be skipped
while arg_index < len(sys.argv):
    cur_arg: str = sys.argv[arg_index]
    if cur_arg[0] != "-":
        if input_filename_index == -1:
            input_filename_index = arg_index
            args_to_remove.append(arg_index)
        else:
            print("Cannot process multiple asm files at once")
            sys.exit(1)
    elif len(cur_arg) == 2 and cur_arg[1] in ["o", "x"]:
        arg_index += 1 # Skip the next argument
    elif len(cur_arg) == 2 and cur_arg == "-V":
        sys.argv[arg_index] = "-v" # Replace with modern gas equivalent
    elif len(cur_arg) == 3 and cur_arg == "-Qy":
        args_to_remove.append(arg_index) # Remove this from the args since it's not accepted by modern gas
    arg_index += 1

if input_filename_index == -1:
    print("No input file in arguments")
    sys.exit(1)

input_filename: str = sys.argv[input_filename_index]

# Remove the input file from the argument list, as mips gas will be assembling stdin
for to_remove in reversed(args_to_remove):
    del sys.argv[to_remove]

flags: List[str] = sys.argv[2:]

branch_mnemonics: Set[str] = {
    "bc1f", "bc1fl", "bc1t", "bc1tl", # cop1
    "beq", "beql", # ==
    "bgez", "bgezal", "bgezall", "bgezl", # >= 0
    "bgtz", "bgtzl", # > 0
    "blez", "blezl", # <= 0
    "bltz", "bltzal", "bltzall", "bltzl", # < 0
    "bne", "bnel", # !=
    "j", "jal", "jalr", "jr" # unconditional absolute jumps
    "blt", "ble", "bgt", "bge" # pseudo instructions
}

cop1_branch_mnemonics: Set[str] = {
    "bc1f", "bc1fl", "bc1t", "bc1tl" # cop1
}

symbol_access_mnemonics: Set[str] = {
    "lb", "lbu", "lh", "lhu", "lw", "lwc1",
    "sb", "sh", "sw", "swc1"
}

# There are many more than these, but cc1 only generates these
compare_mnemonics: Set[str] = {
    "c.lt.s", "c.le.s", "c.eq.s",
    "c.lt.d", "c.le.d", "c.eq.d"
}

def get_line_tokens(line: str) -> List[str]:
    stripped: str = line.strip().split("#")[0].strip()
    return [s.strip() for s in stripped.split()]

with open(input_filename, mode="r") as input_file:
    input_lines: List[str] = input_file.readlines()
    preprocessed: List[str] = [
        ".include \"macro.inc\"\n"
        "\n"
        ".macro move a, b\n"
        "    addu \\a, \\b, $0\n"
        ".endm\n"
        "\n"
        ".macro b target\n"
        "    bgez $0, \\target\n"
        ".endm\n"
        "\t.set noreorder\n"
    ]
    line: str
    is_reorder: bool = True
    generated_symbol_count: int = 0
    at_common: bool = False
    mfhilo_delay_count: int = 0
    mfhilo_delay_location: int = 0
    # Whether the previous instruction was a mul.s/mul.d; asn64 puts a nop between it
    # and a following multiply, but not between a mult and a following mul.s/mul.d
    prev_mul: bool = False
    # Whether the instruction before the current branch was a mul.s/mul.d
    branch_prev_mul: bool = False
    prev_fpcmp: bool = False
    comm_symbols: List[Tuple] = []
    lcomm_symbols: List[Tuple] = []
    # Holds the branch target string if the current instruction is a delay slot
    delay_slot: bool = False
    file_count: int = 0
    prev_instruction_index: int = 0
    last_file_directive: int = -1
    local_symbols: Set[str] = set()

    for line_index, line in enumerate(input_lines):
        tokens: List[str] = get_line_tokens(line)
        # SN64 inserts nops when it sees a commented out nop directly after a cop1 compare instruction
        if line[1:5] == "#nop":
            prev_instruction_tokens: List[str] = get_line_tokens(input_lines[prev_instruction_index])
            if prev_instruction_tokens[0] in compare_mnemonics:
                line = "\tnop\n"
                prev_fpcmp = False
        if len(tokens) == 0:
            preprocessed.append(line)
            continue
        identifier: str = tokens[0]
        operands: List[str]
        new_prev_mul: bool = False
        is_branch: bool = False
        is_instruction: bool = False

        if identifier[0] == ".": # Directives
            directive: str = identifier[1:]
            if directive == "set":
                setting: str = tokens[1]
                if setting == "noreorder":
                    is_reorder = False
                    continue # skip noreorder directives
                elif setting == "reorder":
                    is_reorder = True
                    continue # skip reorder directives
            elif directive == "local":
                local_symbols.add(tokens[1])
            elif directive == "comm":
                # Record .comm symbols and sizes to emit to bss later
                comm_symbol: str
                comm_size: str
                comm_symbol, comm_size = [s.strip() for s in tokens[1].split(",")][:2]
                if comm_symbol in local_symbols:
                    lcomm_symbols.append((comm_symbol, comm_size))
                else:
                    comm_symbols.append((comm_symbol, comm_size))
                line = ""
            elif directive == "lcomm":
                # Record .lcomm symbols and sizes to emit to bss later
                lcomm_symbol: str
                lcomm_size: str
                lcomm_symbol, lcomm_size = [s.strip() for s in tokens[1].split(",")]
                lcomm_symbols.append((lcomm_symbol, lcomm_size))
                line = ""
            elif directive == "file":
                # SN's cc1 has issues reusing file numbers in .file directives. This fixes them up
                file_count += 1
                line = f"\t.file\t{file_count + 1} {tokens[2]}\n"
                last_file_directive = len(preprocessed)
            elif directive == "rdata":
                line = "\t.section .rodata\n"
            elif directive == "ident":
                # asn64 stores #ident strings at the start of .rdata, unaligned;
                # the compiler's own trailing "GCC: ..." ident is not kept
                text = line.split(None, 1)[1].strip()
                if text.startswith('"GCC:'):
                    line = ""
                else:
                    line = f"\t.pushsection .rodata\n\t.asciz {text}\n\t.popsection\n"
            elif directive in ["def", "begin", "bend"]:
                # Modern gas doesn't understand these directives. They are removed
                line = ""
            elif directive == "word":
                # SN's cc1 uses $ for label references but . for the actual label names. This fixes them up
                if tokens[1][0] == "$":
                    line = f"\t.word\t.{tokens[1][1:]}\n"
        elif identifier[-1] == ":": # Labels
            # Nothing to do here, but it might be handy to have in the future
            label_name: str = identifier[:-1]
            pass
        else: # Macros or instructions
            is_instruction = True
            if prev_fpcmp and identifier in ["bc1t", "bc1f", "bc1tl", "bc1fl"]:
                # asn64 fills the FPU compare-to-branch hazard (MIPS II) with a nop,
                # also when the branch starts a noreorder block (behind a label)
                line = "\tnop\n" + line
            if identifier in branch_mnemonics:
                is_branch = True
                if is_reorder:
                    line += "\tnop\n" # insert a nop after the instruction
            elif identifier == "li.s":
                float_sym_name: str = f".LRODATA_SYM_{generated_symbol_count}"
                operands = [s.strip() for s in tokens[1].split(",")]
                if operands[0][1] == "f": # Only expand for float register targets
                    line = (
                        "\t.section .rodata\n"
                        "\t.align 2\n"
                        f"{float_sym_name}:\n"
                        f"\t.float {operands[1]}\n"
                        "\t.text\n"
                        "\t.set\tnoat\n"
                        f"\tlui $at, %hi({float_sym_name})\n"
                        f"\tlwc1 {operands[0]}, %lo({float_sym_name})($at)\n"
                        "\t.set\tat\n"
                    )
                    generated_symbol_count += 1
            elif identifier == "li.d":
                double_sym_name: str = f".LRODATA_SYM_{generated_symbol_count}"
                operands = [s.strip() for s in tokens[1].split(",")]
                if operands[0][1] == "f": # Only expand for float register targets
                    # asn64 only word-aligns double constants
                    hi_word, lo_word = struct.unpack(">II", struct.pack(">d", float(operands[1])))
                    line = (
                        "\t.section .rodata\n"
                        "\t.align 2\n"
                        f"{double_sym_name}:\n"
                        f"\t.word 0x{hi_word:08X}, 0x{lo_word:08X}\n"
                        "\t.text\n"
                        "\t.set\tnoat\n"
                        f"\tlui $at, %hi({double_sym_name})\n"
                        f"\tlwc1 $f{int(operands[0][2:]) + 1}, %lo({double_sym_name})($at)\n"
                        f"\tlwc1 {operands[0]}, %lo({double_sym_name}+4)($at)\n"
                        "\t.set\tat\n"
                    )
                    generated_symbol_count += 1
                # TODO expand li.d correctly in the case of a GPR target
                # addiu zero, 0 (word 2); lui (word 1) for a halfword double
                # addiu zero, 0 (word 2); lui + ori (word 1) for a full word double
                # lui (word 2); lui + ori (word 1) for a 1.5 word double,
                # lui + ori (word 2); lui + ori (word 1) for a 2 word double
            elif identifier in "div":
                # Insert nops if there was a mflo/mfhi recently right after it
                while mfhilo_delay_count > 0:
                    preprocessed.append("\tnop\n")
                    mfhilo_delay_count -= 1
                # Manually expand div operations, since modern gnu assembler expands them slightly differently
                operands = [s.strip() for s in tokens[1].split(",")]
                if operands[0] != "$0":
                    div_branch_label_1: str = f".LBRANCH_LABEL_{generated_symbol_count}"
                    div_branch_label_2: str = f".LBRANCH_LABEL_{generated_symbol_count + 1}"
                    generated_symbol_count += 2
                    line = (
                         "\t.set noat\n"
                        f"\tdiv $0,{operands[1]},{operands[2]}\n"
                        f"\tbnez {operands[2]},{div_branch_label_1}\n"
                         "\tnop\n"
                         "\tbreak 0x7\n"
                        f"{div_branch_label_1}:\n"
                         "\taddiu $1,$0,-1\n"
                        f"\tbne {operands[2]},$1,{div_branch_label_2}\n"
                         "\tlui $1,0x8000\n"
                        f"\tbne {operands[1]},$1,{div_branch_label_2}\n"
                         "\tnop\n"
                         "\tbreak 0x6\n"
                        f"{div_branch_label_2}:\n"
                        f"\tmflo {operands[0]}\n"
                         "\t.set at\n"
                    )
                    mfhilo_delay_count = 3 # Set to 3. It becomes 2 after the decrement
                    mfhilo_delay_location = len(preprocessed) + 1
            elif identifier in "divu":
                # Insert nops if there was a mflo/mfhi recently right after it
                while mfhilo_delay_count > 0:
                    preprocessed.append("\tnop\n")
                    mfhilo_delay_count -= 1
                # Manually expand divu operations, since modern gnu assembler expands them slightly differently
                operands = [s.strip() for s in tokens[1].split(",")]
                if operands[0] != "$0":
                    divu_branch_label: str = f".LBRANCH_LABEL_{generated_symbol_count}"
                    generated_symbol_count += 1
                    line = (
                         "\t.set noat\n"
                        f"\tdivu $0,{operands[1]},{operands[2]}\n"
                        f"\tbnez {operands[2]},{divu_branch_label}\n"
                         "\tnop\n"
                         "\tbreak 0x7\n"
                        f"{divu_branch_label}:\n"
                        f"\tmflo {operands[0]}\n"
                         "\t.set at\n"
                    )
                    mfhilo_delay_count = 3 # Set to 3. It becomes 2 after the decrement
                    mfhilo_delay_location = len(preprocessed) + 1
            elif identifier == "rem":
                # Insert nops if there was a mflo/mfhi recently right after it
                while mfhilo_delay_count > 0:
                    preprocessed.append("\tnop\n")
                    mfhilo_delay_count -= 1
                # Manually expand rem operations, since modern gnu assembler expands them slightly differently
                operands = [s.strip() for s in tokens[1].split(",")]
                if operands[0] != "$0":
                    rem_branch_label_1: str = f".LBRANCH_LABEL_{generated_symbol_count}"
                    rem_branch_label_2: str = f".LBRANCH_LABEL_{generated_symbol_count + 1}"
                    generated_symbol_count += 2
                    line = (
                         "\t.set noat\n"
                        f"\tdiv $0,{operands[1]},{operands[2]}\n"
                        f"\tbnez {operands[2]},{rem_branch_label_1}\n"
                         "\tnop\n"
                         "\tbreak 0x7\n"
                        f"{rem_branch_label_1}:\n"
                         "\taddiu $1,$0,-1\n"
                        f"\tbne {operands[2]},$1,{rem_branch_label_2}\n"
                         "\tlui $1,0x8000\n"
                        f"\tbne {operands[1]},$1,{rem_branch_label_2}\n"
                         "\tnop\n"
                         "\tbreak 0x6\n"
                        f"{rem_branch_label_2}:\n"
                        f"\tmfhi {operands[0]}\n"
                         "\t.set at\n"
                    )
                    mfhilo_delay_count = 3 # Set to 3. It becomes 2 after the decrement
                    mfhilo_delay_location = len(preprocessed) + 1
            elif identifier in ["l.d", "s.d"]:
                # asn64 splits doubleword FPU accesses into two word accesses, odd register first
                op = "lwc1" if identifier == "l.d" else "swc1"
                operands = [o.strip() for o in tokens[1].split(",", 1)]
                freg = int(operands[0][2:])
                hi_reg, lo_reg = f"$f{freg + 1}", f"$f{freg}"
                mem = operands[1]
                m = re.match(r"^(-?(?:0x)?[0-9A-Fa-f]*)\((\$\w+)\)$", mem)
                if m:
                    off = int(m.group(1), 0) if m.group(1) else 0
                    line = (
                        f"\t{op} {hi_reg},{off}({m.group(2)})\n"
                        f"\t{op} {lo_reg},{off + 4}({m.group(2)})\n"
                    )
                else:
                    line = (
                         "\t.set noat\n"
                        f"\tlui $at,%hi({mem})\n"
                        f"\t{op} {hi_reg},%lo({mem})($at)\n"
                        f"\t{op} {lo_reg},%lo({mem}+4)($at)\n"
                         "\t.set at\n"
                    )
            elif identifier in ["lb", "lbu", "lh", "lhu", "lw"] and re.match(r"^\s*\$\w+\s*,\s*[A-Za-z_.][\w.]*(?:[+-](?:0x)?[0-9A-Fa-f]+)?\(\$\w+\)\s*$", tokens[1]):
                # asn64 builds symbol(reg) load addresses in $at, not in the destination register
                operands = [o.strip() for o in tokens[1].split(",", 1)]
                m = re.match(r"^(.+)\((\$\w+)\)$", operands[1])
                line = (
                     "\t.set noat\n"
                    f"\tlui $at,%hi({m.group(1)})\n"
                    f"\taddu $at,$at,{m.group(2)}\n"
                    f"\t{identifier} {operands[0]},%lo({m.group(1)})($at)\n"
                     "\t.set at\n"
                )
            elif identifier == "remu":
                # Insert nops if there was a mflo/mfhi recently right after it
                while mfhilo_delay_count > 0:
                    preprocessed.append("\tnop\n")
                    mfhilo_delay_count -= 1
                operands = [s.strip() for s in tokens[1].split(",")]
                if operands[0] != "$0":
                    remu_branch_label: str = f".LBRANCH_LABEL_{generated_symbol_count}"
                    generated_symbol_count += 1
                    line = (
                         "\t.set noat\n"
                        f"\tdivu $0,{operands[1]},{operands[2]}\n"
                        f"\tbnez {operands[2]},{remu_branch_label}\n"
                         "\tnop\n"
                         "\tbreak 0x7\n"
                        f"{remu_branch_label}:\n"
                        f"\tmfhi {operands[0]}\n"
                         "\t.set at\n"
                    )
                    mfhilo_delay_count = 3 # Set to 3. It becomes 2 after the decrement
                    mfhilo_delay_location = len(preprocessed) + 1
            elif identifier in ["mflo", "mfhi"]:
                mfhilo_delay_count = 3 # Set to 3. It becomes 2 after the decrement
                mfhilo_delay_location = len(preprocessed) + 1
            elif identifier == "mult":
                # SN64 reorders delay slot mult instructions in noreorder to prevent mulmul bugs if the branch target starts with a multiply
                # However, it doesn't do it needs to insert nops after a mfhi/mflo
                # It checks the instruction before the branch for the mul-after-mul nop
                if delay_slot and not is_reorder and mfhilo_delay_count <= 0:
                    preprocessed.insert(len(preprocessed) - 1, ("\tnop\n" if branch_prev_mul else "") + line)
                    line = "\tnop\n"
                # Insert nops if there was a mflo/mfhi recently right after it
                while mfhilo_delay_count > 0:
                    if delay_slot:
                        preprocessed.insert(mfhilo_delay_location, "\tnop\n")
                    else:
                        preprocessed.append("\tnop\n")
                    mfhilo_delay_count -= 1
                if prev_mul:
                    line = "\tnop\n" + line
            elif identifier in ["mul.s", "mul.d"]:
                # SN64 reorders delay slot mul.s/mul.d instructions in noreorder,
                # checking the instruction before the branch for the mul-after-mul nop
                if delay_slot and not is_reorder and mfhilo_delay_count <= 0:
                    preprocessed.insert(len(preprocessed) - 1, ("\tnop\n" if branch_prev_mul else "") + line)
                    line = "\tnop\n"
                if prev_mul:
                    line = "\tnop\n" + line
                new_prev_mul = True

            # This isn't great as it doesn't account for macros correctly, but it should be fine for the subset of inputs actually used
            if mfhilo_delay_count > 0:
                mfhilo_delay_count -= 1
            # A load or store from a bare symbol expands to lui + the access, two instructions
            if mfhilo_delay_count > 0 and identifier in symbol_access_mnemonics and len(tokens) > 1 and "(" not in tokens[1]:
                mfhilo_delay_count -= 1
            
            if is_branch:
                branch_prev_mul = prev_mul
            prev_mul = new_prev_mul
            prev_fpcmp = identifier.startswith("c.")
            delay_slot = is_branch
            prev_instruction_index = line_index


        preprocessed.append(line)

    if len(comm_symbols) > 0 or len(lcomm_symbols) > 0:
        preprocessed.append("\t.section\t.bss\n")
    for (symbol, size) in lcomm_symbols:
        if int(size) > 4:
            preprocessed.append("\t.align 3\n")
        preprocessed.append(
            f"\t.globl {symbol}\n"
            f"{symbol}:\n"
            f"\t.space {size}\n"
        )
    for (symbol, size) in comm_symbols:
        if int(size) > 4:
            preprocessed.append("\t.align 3\n")
        preprocessed.append(
            f"\t.globl {symbol}\n"
            f"{symbol}:\n"
            f"\t.space {size}\n"
        )
    
    if last_file_directive != -1:
        file_directive_tokens: List[str] = get_line_tokens(preprocessed[last_file_directive])
        line_directive: str = f"\t.file\t1 {file_directive_tokens[2]}\n"
        preprocessed[last_file_directive] = ""
        preprocessed.insert(0, line_directive)

    assembler_input: str = "".join(preprocessed)
    # print(assembler_input)
    p = subprocess.run([assembler, *flags, "-"], stdout=subprocess.PIPE, input=assembler_input, encoding='ascii')
    sys.exit(p.returncode)
