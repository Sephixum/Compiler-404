# Compiler-404

**Compiler-404** is a custom compiler built in C++23. It takes a source code file as input and outputs the compilation result to a file named `out.txt`.

---

## Features

- Lightweight and simple compiler framework.
- Outputs results to `out.txt` for easy inspection.
- Easy build and clean process using `make`.

---

## Build Instructions

To build **Compiler-404**, you need:

- **GCC** (C++23 compliant)
- **Make**

The project includes a Makefile with the following rules:

- `make clean` – Cleans previous build artifacts.  
- `make build` – Builds the compiler executable.  
- `make` – Performs `clean` first, then builds the compiler.

After building, the compiler executable is located in the `bin` directory.

---

## Usage

Run the compiler on a code file as follows:

```bash
./bin/app {code_file}
