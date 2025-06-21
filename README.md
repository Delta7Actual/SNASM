# SNASM - Super-Neat Assembler

SNASM is a simple assembler written in C for the Super-Neat Assembly language. It supports macro expansion, symbol table generation, two-pass assembly, and outputs object, entry, and external files in a custom format.

## Features

- **Macro Expansion:** Supports user-defined macros with `mcro`/`mcroend` blocks.
- **Two-Pass Assembly:** First pass builds the symbol table, second pass encodes instructions and data.
- **Symbol Table:** Handles labels, entries, externals, and validates symbol usage.
- **Custom Output:** Generates `.sno` (object), `.sne` (entries), and `.snr` (externals) files.
- **Verbose Logging:** Multiple log levels for debugging and verbose output.
- **Cross-Platform:** Build scripts for both Linux (Makefile) and Windows (build.bat, Makefile.windows).

## Build Instructions From Source Code

*(Pre-compiled binaries are available in the realeases section)

### Requirements
- GCC or compatible C compiler
- Make (Linux/macOS) or build.bat (Windows with MinGW)
- (Optional) Windows users: MinGW-w64 for compiling and running

### Linux

1. **Install GCC** (if not already installed):

    ```sh
    sudo apt-get install build-essential
    ```

2. **Build the assembler:**

    ```sh
    make
    ```

    This will produce the `SNASM` executable in the project root.

### Windows

You can build using either the batch script or the Windows Makefile.

#### Using `build.bat`

1. Open a Command Prompt in the project directory.
2. Run:

    ```bat
    build.bat
    ```

    This will produce `SNASM.exe` in the project root.

## Usage

```sh
./SNASM [options] file1.as [file2.as ...]
```

### Options

- `-v`, `--verbose`        Enable verbose logging
- `-d`, `--debug`          Enable debug-level logging
- `-q`, `--quiet`          Suppress all logging
- `-s`, `--symbols`        Output symbol table
- `-x`, `--externals`      Output external references
- `-e`, `--entries`        Output entries table
- `-o`, `--output <file>`  Specify output file prefix
- `--version`              Show assembler version
- `--help`                 Show help message

### Example

```sh
./SNASM -v example.snasm
```

This will assemble `example.snasm`, expand macros, generate symbol tables, and produce output files in the root directory.

## Output Files

- `.sno` — Object file (machine code)
- `.sne` — Entries file (entry points)
- `.snr` — Externals file (external references)

## The Super-Neat Assembly Language (Full documentation coming soon)

- **Labels:** Defined with `LABEL:`
- **Directives:** `.data`, `.string`, `.entry`, `.extern`
- **Macros:** Defined with `mcro name ... mcroend`
- **Instructions:** See [include/command.h](include/command.h) for the full instruction set. (Full documentation coming soon)

## Example

```assembly
.entry MAIN
.extern EXT_LABEL
MAIN:   mov r3, EXT_LABEL
        add r1, r2
        stop
```

---

Developed and maintained by Dror Sheffer
