# Super-Neat Assembly - Syntax Rules

> This section defines the structure, formatting, and conventions used when writing Super-Neat Assembly (`.snasm` / `.as`) source files.

---

## File Structure
A Super-Neat Assembly source file is a plain text file containing:
- Instructions (e.g., `mov`, `cmp`, `jmp`).
- Labels (used for jumps, variables, or entry points).
- Directives (`.entry`, `.extern`, `.data` etc.).
- Comments (ignored by the assembler).
- Macros (define a reusable piece of code).

Each line may contain:
- An optional label
- An instruction or directive
- Optional operands
- An optional comment
---
## Labels

- Must begin a line (optionally with whitespace).
- Always end with a colon (`:`).
- Must begin with an alphabetic character, followed by letters/digits.
- Are case-sensitive.

### Examples:
```asm
    start1:  ; Valid
    LOOP:    ; Valid
    1fail:   ; Invalid (starts with digit)
```
---
## Comments

- Start with a semicolon (`;`).
- Everything after `;` is ignored by the assembler and is not included in the expanded files.
- Comments can be standalone or inlined.

### Example:
```asm
    mov  r1, r2   ; This is an inlined comment
                  ; This is a full-line comment
```
---
## Directives

- Directives tell the assembler how to handle data or labels.
- There are 4 directives:

### `.data`
- Declares a list of signed integers seperated by commas.
```asm
    NUMBERS: .data 4, -2, 15, 0
    ; Analogous to [4, -2, 15, 0]
```

### `.string`
- Declares a null-terminated list of characters (Only printable characters) surrounded by quotes (`"`).
```asm
    MESSAGE: .string "Hello!"
    ; Analogous to ['H', 'e', 'l', 'l', 'o', '!', 0,]
    ; 0 is a null terminator and represents the end of a string
```

### `.entry`
- Marks a label as an entry point to the program, exporting it.
- Exported labels can be accessed by other files (See `.extern`).
```asm
    .entry START
    START: add r1, r1
    ; Other files referencing START as external will be able to use it
```

### `.extern`
- Declares a label that is defined and exported in another file.
```asm
    .extern ExternalFunc
    ; ExternalFunc is defined in another source file and is referenced here
```
---
## Macros

- Macros let you define code that is reused often.
- Begin a macro with `mcro` + `<name>`.
- Write the macro body.
- End the macro with `mcroend`.
- Nested macros are NOT supported.

### Example:
```asm
    mcro LOADR1      ; Define the macro
        mov #5, r1   ; Macro body (Can be multi-line)
    endmcro          ; Enc macro

    LOADR1           ; Expands into: mov #5, r1

```
---
# Instructions And Operands
> This section documents the available instructions, their syntax, purpose, and allowed operands.

---
## Instruction Format

Each instruction line may consist of:
- A mnemonic (instruction name)
- Zero, one, or two operands
- Operands are separated by a comma `,` and may be (see `Addressing modes`):
  - **Immediate**: `#value`
  - **Direct**: `label`
  - **Relative**: `&label`
  - **Register**: `r0` to `r7` (legacy 24-bit) or `r0` to `r63` (modern 32-bit)

> Operands must match the expected types for each instruction!

### Addressing modes

The Super-Neat Assembly language defines **four addressing modes**, each used to reference data in a different way. These modes are automatically detected based on the operand syntax, and each instruction specifies which modes are allowed per operand.

| Mode | Name        | Syntax     | Example    | Description                                                                 |
|------|-------------|------------|------------|-----------------------------------------------------------------------------|
| 0    | Immediate   | `#value`   | `mov #42, r1`      | A constant value embedded directly in the instruction.                      |
| 1    | Direct      | `label`    | `inc NUM`  | Refers to a label in memory. The actual address will be resolved.          |
| 2    | Relative    | `&label`   | `jmp &LOOP`    | A label resolved as an offset relative to the current instruction address. |
| 3    | Register    | `r<N>`       | `clr r4`       | Refers to a register (`r0`–`r7` in legacy mode, `r0`–`r63` in modern mode). |

> ⚠️ Each instruction only supports specific modes for each operand. If an illegal mode is used, the assembler will reject the instruction with an error.

### Operand Count Rules

- Some instructions require **zero operands** (e.g., `stop`, `rts`, `int`).
- Others use **one operand**, typically for destination (`dst`) only.
- Most arithmetic and data movement instructions use **two operands**: `src`, `dst`.

> The first operand is always treated as **source**, the second as **destination**, unless the instruction's definition specifies otherwise (e.g., `cmp` is a compare, not a move).

#### Example Operand Usage

```asm
    mov #5, r1        ; Immediate to Register
    cmp r1, VALUE     ; Register to Direct
    add r2, r3        ; Register to Register
    lea LABEL, r4     ; Direct to Register
    bne &LOOP         ; Relative jump to label LOOP
```

## Supported Instructions

> Addressing modes will hereby be refeered to as:
> - IMM: *Immediate*
> - DIR: *Direct*
> - REL: *Relative*
> - REG: *Register*

### Instruction Reference Table

| Mnemonic | Opcount | Source Modes        | Destination Modes     | Description |
|----------|---------|---------------------|------------------------|-------------|
| `mov`    | 2       | IMM, DIR, REG       | DIR, REG              | Copies a value from source to destination. |
| `cmp`    | 2       | IMM, DIR, REG       | IMM, DIR, REG         | Compares source and destination, affects condition codes. |
| `add`    | 2       | IMM, DIR, REG       | DIR, REG              | Adds source to destination. |
| `sub`    | 2       | IMM, DIR, REG       | DIR, REG              | Subtracts source from destination. |
| `lea`    | 2       | DIR                 | DIR, REG              | Loads effective address of label into destination. |
| `lod`      | 2       | IMM, DIR, REG     | DIR, REG        | Register, Immediate, Direct                      | Register, Direct                              | Load value from memory (address in source) into destination register or memory |
| `str`      | 2       | IMM, DIR, REG     | DIR, REG        | Register, Immediate, Direct                      | Register, Direct                              | Store value from source register or immediate to memory (address in destination) |
| `clr`    | 1       | —                   | DIR, REG              | Clears (sets to zero) the destination. |
| `not`    | 1       | —                   | DIR, REG              | Bitwise NOT on destination. |
| `inc`    | 1       | —                   | DIR, REG              | Increments destination by one. |
| `dec`    | 1       | —                   | DIR, REG              | Decrements destination by one. |
| `jmp`    | 1       | —                   | DIR, REL              | Unconditional jump to destination address. |
| `bne`    | 1       | —                   | DIR, REL              | Jumps to destination if the last comparison was not equal. |
| `jsr`    | 1       | —                   | DIR, REL              | Jumps to subroutine at destination. |
| `rts`    | 0       | —                   | —                     | Returns from subroutine. |
| `stop`   | 0       | —                   | —                     | Halts program execution. |
| `and`    | 2       | IMM, DIR, REG       | DIR, REG              | Bitwise AND between source and destination. |
| `or`     | 2       | IMM, DIR, REG       | DIR, REG              | Bitwise OR between source and destination. |
| `xor`    | 2       | IMM, DIR, REG       | DIR, REG              | Bitwise XOR between source and destination. |
| `mul`    | 2       | IMM, DIR, REG       | DIR, REG              | Multiplies source by destination. |
| `div`    | 2       | IMM, DIR, REG       | DIR, REG              | Divides destination by source. |
| `mod`    | 2       | IMM, DIR, REG       | DIR, REG              | Stores remainder of destination ÷ source. |
| `beq`    | 1       | —                   | DIR, REL              | Jumps to destination if the last comparison was equal. |
| `push`   | 1       | —                   | IMM, DIR, REG         | Pushes the value onto the stack. |
| `pop`    | 1       | —                   | DIR, REG              | Pops the top value from the stack into destination. |
| `nop`    | 0       | —                   | —                     | Does nothing; no-op instruction. |
| `int`    | 0       | —                   | —                     | Triggers an interrupt for system call. |

---

## Legacy 24-Bit Mode

The legacy 24-bit mode is the original architecture of the Super-Neat Assembly system. It defines each machine word as 24 bits wide, with limited operand width and register capacity. This mode supports:

- Registers `r0` through `r7`
- 21-bit addressing (leaving 3 bits for flags)
- Smaller instruction and data footprint

### Why it Exists

- When I first started this project, I believed that a 24-bit architecture would be sufficient for my needs.
- However, as development progressed, it became clear that expanding to a 32-bit format was necessary to support more complex instructions, larger registers, and better overall flexibility.
- Since I manage and maintain this project on my own, keeping the legacy 24-bit mode available allows me to preserve backward compatibility with earlier work while gradually transitioning to the more capable 32-bit architecture.

#### The 24-bit mode serves as:
- Backward compatibility for early SNASM programs
- A simplified learning platform before moving to the more capable 32-bit mode

Though replaced by the new default 32-bit mode, it remains supported through a compile-time flag (`-l` or `--legacy-24`) for compatibility and flexibility.

---

## Full Code Examples

#### Example 1: Basic Move and Add
```asm
    mov  #5, r1  ; Move immediate value 5 into register r1
    mov  r1, r2  ; Move the value from r1 into r2
    add  r2, r3  ; Add the value in r2 to r3, store result in r3
    stop         ; Stop execution
```

#### Example 2: Simple loop
```asm
START:  mov   #0, r1   ; Initialize r1 to 0
        mov   #3, r2   ; Set loop count to 3
LOOP:   cmp   r1, r2   ; Compare current count with 3
        beq   END      ; If equal, exit loop
        inc   r1       ; Increment counter
        jmp   LOOP     ; Jump back to LOOP
END:    stop           ; Stop execution
```

#### Example 3: Using multi-cell .data directives
```asm
    DATA: .data 5, 10, 15
        lea DATA, r1        ; Load address of DATA into r1
        inc r1              ; Incrementing r1
        lod r1, r2          ; Load value at address in r1 (DATA[1]) into r2
        add #2, r2          ; Add immediate 2 to r2
        str r2, r1          ; Store updated value back to address in r1 (DATA[1])
        stop                ; Stop execution
```

#### Example 4: Using macros
```asm
    mcro SquareR1    ; Define macro
        mul r1, r1   ; Square value in r1
    mcroend          ; End macro definition

    mov #4, r1       ; move immediate 4 into r1
    SquareR1         ; use macro: 4 * 4 = 16
    stop             ; Stop execution
```

#### Example 5: Using the stack
```asm
    mov #42, r3     ; Load 42 into r3
    push r3         ; Push r3 on stack
    mov #0, r3      ; Clear r3
    ; ... Run some operations using r3
    pop r3          ; Pop top of stack into r3 (restores 42)
    stop            ; Stop execution
```