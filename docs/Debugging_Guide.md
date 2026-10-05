# Debugging Guide - step by step (with REAL error messages from this project)

*Debugging* = finding and fixing why a program does not build or does not behave as expected.
Every error text below was produced on purpose by putting a deliberate bug into this project's code, so you will see the **same** messages.

---

## 1. The three kinds of problems

| Kind | When you see it | Typical message | First thing to do |
|---|---|---|---|
| **Compile error** | While building, before the program exists | `error: expected ';' before 'b'` | Go to the file:line in the message |
| **Link error** | Compile passed, but joining files fails | `undefined reference to 'findById(int)'` | A file is missing or `extern "C"` is missing |
| **Run-time problem** | Program starts, then crashes or gives wrong output | `Segmentation fault`, or wrong list | Use prints, gdb, valgrind (section 4) |

A *warning* is not an error (the program still builds) but it usually points to a real bug. **Treat every warning as a bug** - this project builds with **0 warnings**.

---

## 2. Golden steps (use this order every time)

| Step | What to do | Command / action |
|---|---|---|
| 1 | Start clean, so old files do not confuse you | `make clean` then `make` |
| 2 | Read **only the FIRST error**. Later errors are often caused by the first one | Look at `file:line:column` in the first message |
| 3 | Open that file at that line. Check **that line and the line above it** (missing `;` or `)` is reported on the next line) | |
| 4 | Fix one thing, rebuild, repeat | `make` |
| 5 | Turn on all warnings and fix them too | `make` already uses `-Wall`; for extra: `gcc -Wall -Wextra -c src/library.c` |
| 6 | Run the automatic tests | `make test` -> must say `0 failed` |
| 7 | If still wrong, go to section 4 (run-time debugging) | |

---

## 3. Real examples: compile and link errors

### 3.1 Missing semicolon
Message:
```
b1/library.c: In function 'addBook':
b1/library.c:145:21: error: expected ';' before 'b'
  145 |     b->available = 1
      |                     ^
      |                     ;
```
- Meaning: the compiler expected `;` at the end of line 145.
- Fix: write `b->available = 1;`.

### 3.2 Typo in a function name
Message:
```
b2/library.c:137:9: warning: implicit declaration of function 'findByID'; did you mean 'findById'? [-Wimplicit-function-declaration]
```
- Meaning: C does not know a function `findByID` (capital D). The compiler even suggests the right name.
- Fix: use `findById`. C is **case-sensitive**.
- In C++ the same mistake is a hard error: `error: 'findById' was not declared in this scope` (it appears when the prototype is missing from `library.h`).

### 3.3 Link error: `undefined reference`
Message (happens if the `extern "C" { }` wrapper is removed from `library.h`):
```
/usr/bin/ld: main.o: in function `Recommender::recommend(...)':
main.cpp:(.text+0x181): undefined reference to `getUserHistory(char const*, int*)'
/usr/bin/ld: main.cpp:(.text+0x1d2): undefined reference to `getHead()'
/usr/bin/ld: main.cpp:(.text+0x23c): undefined reference to `findById(int)'
```
- Meaning: C++ looks for a **C++-style name** (`getHead()`), but the function was compiled as **C**, so it is not found.
- Fix: keep the `extern "C"` block in `library.h`.
- Same message also appears when you forgot to add a `.c` file to the Code::Blocks project or to the `Makefile` link line.

### 3.4 Warning: unused variable
Message:
```
b4/library.c:185:26: warning: unused variable 'unused' [-Wunused-variable]
```
- Meaning: a variable was declared but never used (often a leftover or a typo'd variable name).
- Fix: delete it, or use it. Other `-Wall` warnings to watch for: `may be used uninitialized`, `format ... expects argument of type`, `comparison between pointer and integer`.

---

## 4. Run-time debugging (program builds but behaves wrong)

### 4.1 Wrong output -> check the data first (80% of problems)
| Step | Check | Command |
|---|---|---|
| 1 | Reset the data to the clean sample | `cp data_sample/*.txt data/` |
| 2 | Look at the files - is every line `a|b|c|...` with the right number of `|`? | `cat data/books.txt` |
| 3 | `books.txt` needs **6** fields, `issues.txt` **3**, `users.txt` **3** | `awk -F'|' '{print NF}' data/books.txt` (should print 6 on every line) |
| 4 | A line with the wrong number of fields is **silently skipped** by `splitLine` - that is why a book may "disappear" | |
| 5 | Run from the project folder (data path is `data/books.txt`) | `pwd` |

### 4.2 Print debugging (simplest tool)
Add a temporary line, rebuild, run, remove it afterwards:
```c
printf("DEBUG issueBook: user=%s id=%d\n", user, id);
```
```cpp
cout << "DEBUG history count = " << n << endl;
```
- Print **what you think a variable contains** right before the line that misbehaves.
- Examples in this project: print `s` (score) inside `Recommender::recommend`; print `id % HASH_SIZE` in `hashInsert`; print `n` in `getUserHistory`.

### 4.3 Crash: "Segmentation fault" -> use gdb (the debugger)
*Segmentation fault* = the program touched memory it must not touch; in this project the usual reason is a **NULL pointer** (e.g. using a book that was not found).

Deliberate bug used here: the check `if (b == NULL) return 0;` was removed from `issueBook`, then "issue book 999" (a book that does not exist) was tried. Real result:
```
Program received signal SIGSEGV, Segmentation fault.
#0  issueBook (user="rahul", id=999) at library.c:218
#1  Student::menu (this=...)        at main.cpp:152
#2  main ()                          at main.cpp:34
```
Step by step:
| Step | Command |
|---|---|
| 1 | Build with debug info: `gcc -g ...` / `g++ -g ...` (in `Makefile` add `-g` to the flags) |
| 2 | Write the keystrokes in a file `in.txt` (one per line), or just run interactively |
| 3 | `gdb ./library` |
| 4 | `run < in.txt` (or `run`) |
| 5 | After the crash type `bt` (*backtrace*): shows **which function called which**, with file and line numbers |
| 6 | Read from the top: `#0` is where it crashed -> `library.c:218`. Open that line. |
| 7 | `print b` shows `0x0` -> `b` is NULL. The previous line should have checked for NULL. |
| 8 | Useful extra gdb commands: `break issueBook` (stop at that function), `next` (run one line), `print id`, `quit` |

### 4.4 Memory problems -> valgrind (or AddressSanitizer)
```
valgrind --leak-check=full ./library
```
- Run normally, use the program, then exit. At the end look at **`ERROR SUMMARY: 0 errors`** and **`definitely lost: 0 bytes`**.
- It reports: reading/writing outside an array, using freed memory, memory never freed.
- Alternative (faster, built into gcc): compile with `-fsanitize=address,undefined -g`.
- This project: **0 errors** with both tools.

### 4.5 Code::Blocks (Windows) debugging
1. Click on the grey bar left of a line number -> a red **breakpoint** dot appears.
2. Press **F8** (Debug) instead of F9 (Run); the program stops at the breakpoint.
3. **F7** = run next line, **Debug > Debugging windows > Watches** to see variable values (`b->available`, `id`).
4. **Shift+F8** = continue, **Debug > Stop** to finish.

---

## 5. Symptom -> cause -> fix (this project)

| Symptom | Likely cause | Fix |
|---|---|---|
| `undefined reference to 'findById'` | `library.c` not compiled/added | Add both source files (`library.c`, `main.cpp`) |
| "(No data/books.txt found - starting with an empty library)" | Program started from the wrong folder | `cd` into the project folder (Code::Blocks: set *Execution working dir*) |
| Login always says "Wrong username or password" | Typo, or `users.txt` line wrong (needs `name|password|role`) | `cat data/users.txt` |
| A book is missing from the list | Its line in `books.txt` has the wrong number of `|` (needs 6 fields) | Fix the line, or `cp data_sample/books.txt data/` |
| Search "harry" finds nothing | Book not in file, or spelling | Search is case-insensitive; check the title in `books.txt` |
| Recommendation says "No reading history yet" | The user has no lines in `issues.txt` | Issue a book first (`priya`/`rahul` have sample history) |
| Recommendation list is empty | User read books of every genre/author, or all similar books already read | Normal; add books or use another user |
| "already issued to someone else" | `available` is 0 in `books.txt` | Return it first, or edit the line to `...|1|n` |
| Program crashes right after typing text where a number is expected | Should not happen (`readInt` handles it); if it does, run gdb (4.3) | `bt` and read `#0` |
| Windows: `make` not found | It is called `mingw32-make` there | Use it, or the direct `gcc`/`g++` commands from `Language_and_Compiler.md` |
| Garbage characters / lines not splitting in files edited on Windows | CRLF line endings | Already handled: `splitLine` removes `\r\n` (test T14) |

---

## 6. Safe testing routine (before you change anything, and before you push)
1. Backup: `cp -r data data_backup`
2. Change one small thing.
3. `make clean && make` -> no errors, **no warnings**.
4. `make test` -> `RESULT: 17 passed, 0 failed`.
5. Reset data: `cp data_sample/*.txt data/` (the tests do this themselves).
6. Only then `git add -A && git commit -m "what you changed" && git push`.

If a test fails, the test output (in `docs/test_output.txt` style) prints `MISSING: <expected text>` - search that sentence in the code to find the function responsible.

## 7. How this project avoids bugs (design choices)
| Choice | Bug it prevents |
|---|---|
| Fixed-size arrays with limits (`MAX_BOOKS`, `MAX_HISTORY`, `MAX_RECORDS`) | No `new/delete` mistakes, no running out of memory unexpectedly |
| `cleanText` / `strncpy` with size | Text longer than the array cannot overflow it |
| `BUF`-limited `toLower` | A very long search word cannot overflow (test T13) |
| Every `findById` result is checked for `NULL` | Segmentation fault on a missing book |
| `buildIndexes()` after add/delete | Hash table / tree never point to a freed book |
| Each test starts from `data_sample/` | Tests never affect each other |
