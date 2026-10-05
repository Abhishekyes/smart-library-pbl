# Language and Compiler used in this project

## 1. Language: BOTH C and C++ (one project, two languages)

| Part | Language | Files | Lines | Why this language |
|---|---|---|---|---|
| Library Core Engine | **C** | `list.c`, `search.c` | 498 | Linked list, hashing, BST, sorting, file handling are Data-Structures-in-C topics: direct memory control (`malloc`/`free`, pointers) and speed. |
| Shared header | **C** (readable by C++) | `library.h` | 69 | Book structure + function list; `extern "C"` makes it usable from C++. |
| OOP + Recommendation layer | **C++** | `oop.h`, `oop.cpp`, `main.cpp` | 392 | Classes, inheritance, polymorphism are OOPs-with-C++ topics. |
| Support (not program code) | Makefile, bash | `Makefile`, `run_tests.sh` | - | Build and test automation. |

So: **about half the code is C and half is C++**, exactly as the PPT says ("C for the core engine, C++ OOP for recommendation and user management").

### How the two languages talk to each other
- The C++ files call C functions (`issueBook`, `findById`, `showSorted` ...) through `library.h`.
- `library.h` wraps its function list in `extern "C" { ... }` so the C++ compiler does not rename them (*name mangling*).
- The C files **never** call C++ code. One-way link: C++ -> C.

### Language level used (keeps it compatible everywhere)
| Item | Detail |
|---|---|
| C code style | Plain C: `/* */` comments, variables declared at the top of functions, no special features. Works with any C compiler (even very old). |
| C++ code style | Basic C++ only: classes, inheritance, `virtual`, `std::string`; `new`/`delete` only for the Student/Admin object at login, `cin/cout`. **No STL containers (no vector/map), no lambdas, no templates**, nothing newer than the course topics. |
| Default standard | GCC picks it automatically (C: GNU17, C++: GNU++17 on GCC 11+). The code does not depend on it. |

## 2. Compiler: GCC (as in the PPT: "Compiler: GCC, IDE: Code::Blocks")

GCC = **GNU Compiler Collection**, a free compiler family. It gives two commands:

| Command | Compiles | Used for |
|---|---|---|
| `gcc` | C | `list.c`, `search.c` |
| `g++` | C++ | `oop.cpp`, `main.cpp` and the **final linking** |

### 2.1 What happens when you build (4 steps)
```
source (.c / .cpp)
   |  1. Preprocessing : #include / #define are expanded
   |  2. Compiling     : code -> machine instructions
   |  3. Assembling    : -> object file (.o)
   v
list.o  search.o  oop.o  main.o
   |  4. Linking       : g++ joins all .o files + standard libraries
   v
library   (library.exe on Windows)   <- the program
```

### 2.2 The exact commands (what `make` runs)
```
gcc -Wall -c src/list.c   -o src/list.o
gcc -Wall -c src/search.c -o src/search.o
g++ -Wall -c src/oop.cpp  -o src/oop.o
g++ -Wall -c src/main.cpp -o src/main.o
g++ -o library src/list.o src/search.o src/oop.o src/main.o
```
| Flag | Meaning |
|---|---|
| `-c` | Compile only, make a `.o`, do not link yet |
| `-o name` | Name of the output file |
| `-Wall` | Show all common warnings (our code builds with **0 warnings**, also with `-Wextra`) |

**Why link with `g++` and not `gcc`?** The final program contains C++ code, so it needs the C++ standard library; `g++` adds it automatically. Linking with plain `gcc` would give "undefined reference" errors.

### 2.3 Compiler on each computer
| System | What to install | Notes |
|---|---|---|
| **Windows** | Code::Blocks **"codeblocks-...mingw-setup.exe"** (includes MinGW-w64 = GCC for Windows) | Code::Blocks picks `gcc` for `.c` and `g++` for `.cpp` by itself. Make sure Settings > Compiler = "GNU GCC Compiler". |
| **Linux** | `sudo apt install build-essential` | Gives gcc, g++, make |
| **Mac** | `xcode-select --install` | The commands `gcc` / `g++` actually run Apple's **Clang** (same behaviour for this project). |

This project was built and tested with **GCC 11.4 (Ubuntu)**: 0 warnings, 17/17 tests passed, valgrind showed 0 memory errors.

### 2.4 Code::Blocks setup (IDE)
1. File > New > Project > **Empty project**.
2. Add all 6 files from `src/` (`.h` and `.c`/`.cpp`). Code::Blocks sends `.c` to gcc and `.cpp` to g++.
3. Project > Properties > Build targets > **Execution working dir** = project folder (so `data/books.txt` is found).
4. Build and Run (F9).
- Check: Settings > Compiler > Toolchain executables: C compiler `gcc`, C++ compiler `g++`, linker `g++`.

### 2.5 Check your compiler
```
gcc --version
g++ --version
```
Both must print a version. Then `make` (Linux/Mac) or the commands in 2.2 (Windows cmd).

### 2.6 Common errors
| Error message | Reason | Fix |
|---|---|---|
| `undefined reference to 'findById'` | A file was not added to the project / not linked | Add all `.c` and `.cpp` files |
| `undefined reference` for C functions from C++ | `extern "C"` missing in the header | Keep the wrapper in `library.h` |
| `gcc: command not found` | Compiler not installed or not in PATH | Install (see 2.3) |
| `make: command not found` (Windows) | `make` is called `mingw32-make` there | Use `mingw32-make` or the direct commands |
| Program starts but "No data/books.txt found" | Started from the wrong folder | Run from the project folder (or set Execution working dir) |
