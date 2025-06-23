# Super-Neat Assembly - Encoding Specification

> This document defines the binary encoding format of instructions and data in the Super-Neat Assembly (SNASM) system. It includes layouts for both the default 32-bit and legacy 24-bit architectures.

---

## Word Types

There are two main word types used in SNASM output files:
1. **Instruction words** – represent actual operations
2. **Value/address words** – represent constants, addresses, label values, etc.

Each word contains **MARE bits** at the end to determine how the word should be treated:
- `M` - More (This is not the last word for an instruction, not in 24 bit legacy mode)
- `A` - Absolute (no relocation required)
- `R` - Relocatable (label address to be resolved)
- `E` - External (defined in another file)

Only one of M/A/R/E will be set per word.

---

## 32-bit Mode (Default)

### Instruction Word (32-bit)

```
---------------------------------------------------------------------------------------
|31,30,29,28,27,26|25,24|23,22,21,20,19,18|17,16|15,14,13,12,11,10|9,8,7,6,5,4|3|2|1|0|
|     OPCODE      |SRC_A|    SRC_REG      |DST_A|    DST_REG      |   FUNCT   |M|A|R|E|
---------------------------------------------------------------------------------------
```

#### Field Breakdown:
- `OPCODE` (6 bits): Operation code (e.g., `mov`, `add`, `jmp`)
- `SRC_A` (2 bits): Source operand addressing mode
- `SRC_REG` (6 bits): Source register index (if used)
- `DST_A` (2 bits): Destination operand addressing mode
- `DST_REG` (6 bits): Destination register index (if used)
- `FUNCT` (6 bits): Instruction sub-type (used to distinguish similar ops like `add`/`sub`)
- `M` (1 bit): More flag (Explained above)
- `A/R/E` (3 bits): Address type (Absolute, Relocatable, External)

### Value/Operand Word (32-bit)

```
---------------------------------------------------------------------------------------
|31                                  ...                                     4|3|2|1|0|
|                             28-bit signed value                             |M|A|R|E|
---------------------------------------------------------------------------------------
```

- 28-bit value: Used for constants, label addresses, etc.
- `M` (1 bit): More flag (same as above)
- `A/R/E`: Address type (Absolute, Relocatable, External)

---

## 24-bit Mode (Legacy)

### Instruction Word (24-bit)

```
---------------------------------------------------------------
|23,22,21,20,19,18|17,16|15,14,13|12,11|10,9,8|7,6,5,4,3|2|1|0|
|     OPCODE      |SRC_A| SRC_RG |DST_A|DST_RG|  FUNCT  |A|R|E|
---------------------------------------------------------------
```

#### Field Breakdown:
- `OPCODE` (6 bits): Operation code
- `SRC_A` (2 bits): Source addressing mode (M0–M3)
- `SRC_REG` (3 bits): Source register index (0–7)
- `DST_A` (2 bits): Destination addressing mode (M0–M3)
- `DST_REG` (3 bits): Destination register index (0–7)
- `FUNCT` (5 bits): Instruction sub-type
- `A/R/E`: Address type (Absolute, Relocatable, External)

> ⚠️ Note: In legacy mode, only 3-bit register indices are allowed (`r0`–`r7`), and values are limited to 21 bits.

### Value/Operand Word (24-bit)

```
---------------------------------------------------------------
|23                            ...                     3|2|1|0|
|                   21-bit signed value                 |A|R|E|
---------------------------------------------------------------
```

- 21-bit value: Represents constants, label offsets, etc.
- `A/R/E`: Address type (Absolute, Relocatable, External)

---

## Addressing Mode Values

Each operand may use one of four addressing modes:
- `00` → Immediate
- `01` → Direct
- `10` → Relative
- `11` → Register

These are encoded into `SRC_A` and `DST_A` fields in both instruction formats.

---

## Notes

- The `FUNCT` field is often used to distinguish variants of the same opcode class (e.g., `add` vs `sub`).
- For simplicity, most instructions will require one or two additional operand words after the instruction word, depending on the addressing modes.

---

## Summary

| Format       | Instruction Word Width | Value Word Width | Registers Supported |
|--------------|------------------------|------------------|---------------------|
| **32-bit**   | 32 bits                | 32 bits          | `r0`–`r63`          |
| **24-bit**   | 24 bits                | 24 bits          | `r0`–`r7`           |

> Default output is in 32-bit mode unless the `-l` or `--legacy-24` flag is used at assembly time.