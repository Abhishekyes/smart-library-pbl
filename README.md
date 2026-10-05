# Intelligent Digital Library and Book Recommendation System

Team ID: DSCPP-II-2026-T367 | Graphic Era (Deemed to be University) | PBL Semester 3

A console library program. **C** (linked list, hashing, BST, sorting, file handling) runs the library;
**C++** (classes, inheritance, polymorphism) handles users and book recommendations.
Nothing outside these concepts is used (no STL containers, no database, no external library).

## Folder map
| Path | What it is |
|---|---|
| `src/library.h` | Shared header (Book struct + C function list, `extern "C"` for C++) |
| `src/list.c` | Linked list, file handling, add / delete / issue / return, history |
| `src/search.c` | Hashing (by ID), BST (by title), insertion sort |
| `src/oop.h`, `src/oop.cpp` | User / Student / Admin classes, recommendation rules, login |
| `src/main.cpp` | Login screen + main loop |
| `data/` | `books.txt`, `users.txt`, `issues.txt` (live data) |
| `data_sample/` | Untouched copy of the sample data (copy back to reset) |
| `run_tests.sh` | Runs 14 scripted tests, prints PASS/FAIL (`bash run_tests.sh`) |
| `docs/test_output.txt` | Saved output of the last test run |
| `docs/Project_Report.docx` | Full project report |
| `docs/Viva_QnA.md` | Likely viva questions with simple answers |
| `docs/Code_Walkthrough.md` | Explains every folder, file, function and concept in plain language |
| `docs/Deep_Dive.md` | Who calls whom, who owns data, call chain per menu option, line-by-line code explanation |
| `Makefile` | Build file |

## Build and run
**Terminal (Linux / Mac / MinGW):**
```
make
./library          # Windows: library.exe
```
**Code::Blocks:** New empty project -> add every file in `src/` -> Project > Properties > Build targets >
set *Execution working dir* to the project folder -> Build and Run.

Always run from the project folder (data files are opened as `data/books.txt`).

## Demo logins
| Username | Password | Role |
|---|---|---|
| admin | admin123 | Admin |
| priya | priya123 | Student (Fantasy + Programming history -> good recommendation demo) |
| rahul | rahul123 | Student |

## Suggested demo order (5 minutes)
1. Login as `priya` -> **1** search `harry` (partial, A-Z via BST) -> **2** search ID `105` (hashing) -> **3** sort by popularity.
2. **7** Recommend -> shows 204 and 202 (score 7), then three Programming books (score 2). Explain: genre +2, author +3, from reading history.
3. **4** issue book 204 -> issue again (refused) -> **5** return it -> **6** history.
4. Logout, login as `admin` -> add a book, delete an issued book (refused), view issued books, create a student.
5. Show `data/books.txt` changing live.

## 3-member work division
| Member | Files | Concepts | Report sections | Test cases |
|---|---|---|---|---|
| **1 - Core Engine + Docs** | `library.h`, `list.c`, `Makefile`, `data/` | Linked list, file handling, issue/return | 1, 2, 3, 4, 6, 7, 10, 12 | T5 |
| **2 - Search + Main + Testing** | `search.c`, `main.cpp`, `run_tests.sh` | Hashing, BST, sorting, program flow | 5 (hash/BST/sort), 11 | T1-T4, T8, T9, T11, T12 |
| **3 - OOP + Recommendation** | `oop.h`, `oop.cpp` | Classes, inheritance, polymorphism, recommender, login | 5 (OOP), 8, 9 | T6, T6b, T7 |

Everyone: read all code once, and be able to demo + explain your own files.

## Known limits (also in report)
Plain-text passwords, unbalanced BST, insertion sort for big lists, one copy per book, no due dates/fines.

## Where to find each member's work in the code
Every source file starts with a comment block that says **OWNER**, **TOPICS** (which course concept it uses),
what is **NOT** in that file, and a list of **SECTIONS**. Search for `OWNER` to see the split quickly:
```
grep -n "OWNER" src/*
```
