# Smart Library - Complete Code Walkthrough

Plain-language guide to **every folder, every file and every function** of this project.
Technical words are explained where they first appear (also see the glossary at the end).

---

## 1. Big picture

**What the program does:** a console (text-menu) library. Students can search books, issue/return them,
see their history and get book suggestions. Admins can add/delete books, see who holds which book and create student accounts.

**Two layers working together (as in the PPT):**

| Layer | Language | Job | Files |
|---|---|---|---|
| Library Core Engine | C | Stores books, searches, sorts, issues, returns, saves to files | `list.c`, `search.c`, `library.h` |
| OOP + Recommendation layer | C++ | Users (Student/Admin), menus, login, recommendations | `oop.h`, `oop.cpp`, `main.cpp` |
| Data storage | text files | Books, users, issue history | `data/*.txt` |

**Flow of the whole program (one line each):**

1. `main.cpp` starts -> calls `loadBooks()` (C) -> books file becomes a **linked list**; a **hash table** and a **BST** are built from it.
2. User types username/password -> `loginUser()` (C++) creates a **Student** or **Admin** object.
3. `user->menu()` shows that role's menu (**polymorphism**).
4. Each menu choice calls a C function (search / issue / return / add / delete) or the C++ `Recommender`.
5. Every change is written to the text files immediately, so nothing is lost when the program closes.

```
 keyboard
    |
 main.cpp --> loginUser() --> Student / Admin object --> menu()
                                   |                        |
                                   |   (C++ calls C functions via library.h)
                                   v
        list.c  (linked list, files, issue/return)   search.c (hash, BST, sort)
                                   |
                                   v
                           data/*.txt (saved on disk)
```

---

## 2. Folder map (every folder and file)

```
Proj_3p/
|-- src/                 all source code
|   |-- library.h        shared header (Book struct + C function list)
|   |-- list.c           [Member 1] linked list, file handling, issue/return
|   |-- search.c         [Member 2] hashing, BST, sorting
|   |-- oop.h            [Member 3] class declarations
|   |-- oop.cpp          [Member 3] class code, login, recommendation
|   `-- main.cpp         [Member 2] program start + login loop
|-- data/                LIVE data the program reads and changes
|   |-- books.txt
|   |-- users.txt
|   `-- issues.txt
|-- data_sample/         untouched copy of the starting data (for reset and tests)
|-- docs/
|   |-- Project_Report.docx   full project report
|   |-- Viva_QnA.md           viva questions with simple answers
|   |-- test_output.txt       saved output of the last test run
|   |-- Code_Walkthrough.md   what every folder/file/function is
|   |-- Deep_Dive.md          how files connect + call chains
|   |-- Code_Explained.md     all code with explanations
|   |-- Language_and_Compiler.md   C vs C++ and GCC
|   `-- Debugging_Guide.md    step-by-step debugging
|-- Makefile             build instructions (one command: make)
|-- run_tests.sh         17 automatic tests (PASS/FAIL)
|-- README.md            quick start, demo logins, work split
`-- .gitignore           files Git must NOT upload (compiled files)
```

Files created when you build (not uploaded to GitHub): `src/*.o` (compiled pieces) and `library` (the program; `library.exe` on Windows).

---

## 3. Root files

### 3.1 `Makefile` - build instructions
- `make` compiles each file separately, then joins them:
  - `.c` files (`list.c`, `search.c`) -> compiled with **`gcc`** (C compiler)
  - `.cpp` files (`oop.cpp`, `main.cpp`) -> compiled with **`g++`** (C++ compiler)
  - final step uses `g++` to link all four `.o` files into one program called `library`.
- `-Wall` = show all common warnings.
- Rebuilds only the files whose source (or `library.h` / `oop.h`) changed.
- `make clean` deletes `src/*.o` and `library`.

### 3.2 `run_tests.sh` - automatic tester (Member 2)
- Feeds pre-written keystrokes to `./library` (like a robot typing), cleans the menus out of the output, and checks that expected sentences appear.
- Before **every** test it resets `data/` from `data_sample/`, so tests never affect each other.
- Prints `PASS`/`FAIL` for each test and a final `RESULT: 17 passed, 0 failed`.
- Run: `make && bash run_tests.sh`. (Needs bash: Mac/Linux/Git-Bash. Not for plain Windows cmd.)

### 3.3 `README.md`
Quick start, folder map, demo logins, 5-minute demo order, 3-member split.

### 3.4 `.gitignore`
Tells Git to skip `*.o`, `library`, `library.exe`, `.DS_Store`, Code::Blocks project files. Compiled files are machine-specific and should be rebuilt, not uploaded.

---

## 4. `data/` - the text files (the "database")

All fields are separated by the **`|`** character (that is why `|` is not allowed inside a title).

### 4.1 `books.txt` - one line per book
```
201|Harry Potter and the Sorcerer's Stone|J K Rowling|Fantasy|1|3
 |          title                          author      genre  | |
 id                                                  available timesIssued
```
| Field | Meaning |
|---|---|
| id | Unique number of the book |
| title, author, genre | Text |
| available | `1` = on shelf, `0` = currently issued |
| timesIssued | How many times it has been issued (used for "sort by popularity") |

- Read by `loadBooks()`, written by `saveBooks()` (both in `list.c`).
- Sample has 25 books in **unsorted order on purpose** (so the BST is not a straight chain).

### 4.2 `users.txt` - one line per account
```
priya|priya123|student
admin|admin123|admin
```
`username | password | role`. Read by `loginUser()` and `userExists()`, appended by `Admin::addStudentAccount()` (all in `oop.cpp`).

### 4.3 `issues.txt` - one line per borrow = reading history
```
priya|201|RETURNED     <- priya borrowed book 201 and gave it back
priya|102|ISSUED       <- priya currently holds book 102
```
`username | bookId | ISSUED or RETURNED`.
- Written by `issueBook()` (adds a line) and `returnBook()` (changes ISSUED to RETURNED).
- **This same file is the "reading history"** used by the recommendation engine - no separate history file is needed.

### 4.4 `data_sample/`
Exact copy of the three starting files. Copy them into `data/` any time to reset the demo. `run_tests.sh` does this automatically.

---

## 5. `src/library.h` - the shared header

A **header file** lists what exists (structures and function names) so other files can use them. Both C and C++ files include it.

### 5.1 Constants
| Name | Value | Use |
|---|---|---|
| `TITLE_LEN`, `AUTHOR_LEN`, `GENRE_LEN`, `NAME_LEN` | 60, 40, 30, 30 | Maximum text sizes (fixed-size character arrays) |
| `MAX_HISTORY` | 200 | Max borrowed-book ids kept per user for recommendation |
| `MAX_BOOKS` | 500 | Max books in the library (fixed-size arrays are used instead of new/delete) |
| `BOOKS_FILE`, `ISSUES_FILE` | `"data/books.txt"`, `"data/issues.txt"` | File paths (relative - run the program from the project folder) |

### 5.2 `struct Book` - one node of the linked list
```c
typedef struct Book {
    int  id;  char title[60]; char author[40]; char genre[30];
    int  available;        // 1 / 0
    int  timesIssued;
    struct Book *next;     // pointer to the next book -> this makes it a LINKED LIST
} Book;
```
### 5.3 `extern "C" { ... }`
C and C++ store function names differently internally. This wrapper tells the C++ compiler "these functions were compiled as C", so C++ can call them. It is the only link between the two languages.

### 5.4 Function list (who owns it, what it returns)
| Function | File | Returns / does |
|---|---|---|
| `loadBooks()` / `saveBooks()` | list.c | Read / write books file |
| `splitLine(line, parts, max)` | list.c | Cut a text line at every pipe character; returns the number of pieces (used for books, issues and users files) |
| `getHead()` | list.c | Gives the first node of the list |
| `addBook(id,title,author,genre)` | list.c | `1` added, `0` duplicate id, `-1` library full |
| `deleteBook(id)` | list.c | `1` deleted, `0` not found, `-1` book is issued |
| `printBookHeader()`, `displayBook()` | list.c | Print table rows |
| `issueBook(user,id)` | list.c | `1` ok, `0` no such book, `-1` already issued |
| `returnBook(user,id)` | list.c | `1` ok, `0` this user did not issue it |
| `getUserHistory(user,ids[],max)` | list.c | Fills `ids[]`, returns how many |
| `showUserHistory(user)`, `displayIssuedBooks()` | list.c | Print history / admin report |
| `buildIndexes()` | search.c | Rebuild hash table + BST from the list |
| `findById(id)` | search.c | Book pointer or `NULL` (hashing) |
| `findByTitle(title)` | search.c | Book pointer or `NULL` (BST) |
| `searchTitleContains(word)` | search.c | Prints matches, returns how many |
| `showSorted(mode)` | search.c | `1` title, `2` author, `3` popularity |

---

## 6. `src/list.c` - Member 1 (linked list + file handling)

### 6.1 Top of the file
- `head` - a global (file-private, `static`) pointer to the **first book**. The whole library hangs from it.
- `Record` + `records[2000]` - one line of `issues.txt` held in memory (`user`, `bookId`, `status`). Used only while reading/rewriting the history file.
- `getHead()` - lets other files (like `search.c`) walk the list without touching `head` directly.

### 6.2 Functions, one by one
| Function | What it does, step by step |
|---|---|
| `splitLine(line, parts, max)` | Removes the newline (`\r\n`) at the end of the line, then uses `strtok(line, "|")` repeatedly to cut it into pieces. Example: `"201|The Hobbit|Tolkien"` gives `"201"`, `"The Hobbit"`, `"Tolkien"`. Used by the books, issues **and** users files. |
| `countBooks()` | Walks the list and counts the nodes (used to stop at `MAX_BOOKS`). |
| `cleanText(dest, src, size)` | Copies text safely (never more than `size-1` characters) and turns any pipe or newline into a space, so a user cannot break the file format. |
| `freeList()` | Walks the list, `free()`s every node (gives memory back), sets `head = NULL`. |
| `appendNode(node)` | Puts a node at the **end**: if list empty -> it becomes `head`; otherwise walk to the last node and link it. |
| `loadBooks()` | Opens `books.txt`; reads line by line with `fgets`; `splitLine` cuts the line into 6 pieces (a line without exactly 6 pieces is skipped); `malloc` makes a node; `atoi` turns the number texts into numbers; `cleanText` copies the texts; `appendNode` adds it. Stops at `MAX_BOOKS`. Finally `buildIndexes()`. If the file is missing -> starts with an empty library. |
| `saveBooks()` | Opens `books.txt` for writing and prints every node as one line. |
| `addBook()` | Refuse if `findById(id)` already finds that id (`0`) or the library is full (`-1`); else make a node, clean the texts, set `available=1, timesIssued=0`, append, save, rebuild indexes. |
| `deleteBook()` | Walk with two pointers (`prev`, `cur`) to the node; refuse if missing or issued; else **re-link** `prev->next = cur->next` (or move `head`), `free(cur)`, save, rebuild indexes. |
| `printBookHeader()`, `displayBook()` | Formatted table printing. `%-40.40s` = left-aligned, exactly 40 wide, cut if longer. |
| `readRecords()` | Loads all lines of `issues.txt` into `records[]` (using `splitLine`, 3 pieces per line), returns the count. |
| `writeRecords(n)` | Writes `records[]` back to `issues.txt`. |
| `issueBook(user,id)` | Find book via hash; not found -> `0`; already issued -> `-1`; else `available=0`, `timesIssued++`, **append** `user|id|ISSUED` to the history file, save books, return `1`. |
| `returnBook(user,id)` | Load history; find the line with this user + id + `ISSUED`; change it to `RETURNED`; rewrite file; set book `available=1`; save books. If no such line -> `0` (so you cannot return someone else's book). |
| `getUserHistory(user, ids, max)` | Collects every book id this user ever borrowed (used by the recommender). |
| `showUserHistory(user)` | Prints that user's lines with book titles and status. |
| `displayIssuedBooks()` | Admin report: all lines with status `ISSUED` (who holds what). |

### 6.3 Concepts used here
- **Linked list** - see section 11.1.
- **File handling** - `fopen` (open), `fgets` (read a line), `strtok` (cut a line at `|`, inside `splitLine`), `fprintf` (write), `fclose` (close). Modes: `"r"` read, `"w"` overwrite, `"a"` append.
- **Dynamic memory** - `malloc` creates a node, `free` releases it.

---

## 7. `src/search.c` - Member 2 (hashing, BST, sorting)

### 7.1 Helpers
- `toLower(src, dest)` - copies a text in lower case (limited to `BUF` = 200 characters so a long search word can never overflow).
- `compareText(a, b)` - lower-cases both texts, then uses the normal `strcmp`. Result: negative (a first) / 0 (same) / positive. So `HARRY` and `harry` are equal.
- `containsText(text, word)` - lower-cases both, then `strstr` (find a piece of text inside another). 1 = found.

### 7.2 Hashing (`findById`)
- A **hash table** is an array of 101 slots (`table[101]`). The slot of a book is **`id % 101`** (remainder).
- Example: `105 % 101 = 4`, so book 105 lives in slot 4; `201 % 101 = 100`.
- **Collision** = two books land in the same slot: `101`, `202`, `303`, `404` all give `0`. They are **chained** - each slot holds a small linked list (`HNode` has `book` + `next`).
- `hashInsert(b)` adds at the front of that slot's chain; `hashClear()` frees everything; `findById(id)` jumps to slot `id % 101` and checks only that short chain.
- The table stores **pointers** to the same `Book` nodes - no copy of data.

### 7.3 Binary Search Tree (`findByTitle`, `searchTitleContains`)
- Each tree node (`TNode`) holds a `Book*`, a `left` child and a `right` child. Rule: **titles smaller (A-Z) go left, bigger go right.**
- `bstInsert()` - recursive: compare with the current node, go left or right until an empty place is found. Equal titles go right.
- `findByTitle(title)` - exact search: start at `root`, compare, go left or right. Each step throws away about half the tree.
- `searchTitleContains(word)` - **in-order walk** (left, then node, then right) via `walkContains()`; visiting a BST in-order gives **A-Z order**, so partial matches (e.g. `harry`) print alphabetically.
- `bstClear()` - frees all tree nodes.

### 7.4 `buildIndexes()`
Clears the hash table and tree, then walks the linked list once and inserts every book into both. Called after load, add and delete (issue/return only change fields inside the same nodes, so no rebuild is needed).

### 7.5 Sorting (`showSorted(mode)`)
1. Copy the **pointers** of the books into a fixed array `arr[MAX_BOOKS]` (the linked list order is not changed).
2. **Bubble sort**: compare neighbours `arr[j]` and `arr[j+1]`; if they are in the wrong order, swap them. After every round the "biggest" item has moved to the end, so the next round needs one comparison less.
3. `isAfter(a, b, mode)` decides the order: mode 1 title A-Z, mode 2 author A-Z, mode 3 most `timesIssued` first.
4. Print the array. (No `malloc`/`free` needed - a fixed array is used.)

Small example (sort 5, 2, 4): round 1: `[5,2,4]` -> swap -> `[2,5,4]` -> swap -> `[2,4,5]`. Round 2: no swap needed -> done.

## 8. `src/oop.h` and `src/oop.cpp` - Member 3 (C++ classes)

### 8.1 Class map
```
User  (abstract)               Recommender  (separate class, suggests books)
 |-- Student
 |-- Admin
```
- **Abstract class** = has at least one `= 0` function (a *pure virtual* function: "children MUST write this"). You cannot create an object of an abstract class.
- **`virtual`** = "decide at run time which child's version to run" = **polymorphism**.

### 8.2 Helpers in `oop.cpp`
- `readLine(prompt)` - prints prompt, reads a whole line; if input ends (Ctrl+D / test file ends) it exits cleanly instead of looping forever.
- `readInt(prompt)` - `readLine` + `atoi` (text to number; letters give `0`, which the menus treat as invalid/exit).

### 8.3 Recommendation code (`Recommender::recommend`)
| Step | What it does |
|---|---|
| 1 | `getUserHistory` (C) fills `history[]` with the ids the user borrowed. **No history -> prints a message and stops.** |
| 2 | Fixed arrays `cand[MAX_BOOKS]` (candidate books) and `score[MAX_BOOKS]` (their scores). No `new`/`delete` needed. |
| 3 | For every book in the library: loop over the history. If the user already read this book -> skip it. Otherwise, for each history book (found with `findById`): **same genre -> +2**, **same author -> +3**. |
| 4 | Books with score 0 are dropped. The rest go into `cand` / `score`. |
| 5 | **Bubble sort** on both arrays, highest score first (equal scores keep the books-file order). |
| 6 | Print the top 5 (`TOP_BOOKS`). |

**Worked example (priya).** History: 201 (Fantasy, J K Rowling), 203 (Fantasy, J R R Tolkien), 102 (Programming).

| Book | Genre | Author | Total |
|---|---|---|---|
| 202 Harry Potter 2 | 2+2 = 4 | +3 (Rowling) | **7** |
| 204 Lord of the Rings | 4 | +3 (Tolkien) | **7** |
| 105 / 101 / 103 (Programming) | +2 | 0 | **2** |

### 8.4 `User` and its children
- `User` holds `username` and three **common features** (written once, used by both roles): `searchByTitle()` (tries exact BST lookup first, then partial search), `searchById()` (hash), `viewSorted()` (asks 1/2/3, calls `showSorted`).
- `getRole()` and `menu()` are `virtual` - each child writes its own.
- **`Student::menu()`** - loop showing options 1-7 and `0 Logout`: search by title, search by ID, sorted list, issue, return, history, recommend. Uses `username` so every action is for *this* student.
- **`Admin::menu()`** - options: add book, delete book, sorted list, search title, search ID, issued report, create student account.
  - `addNewBook()` - validates (id > 0, no empty fields) then calls `addBook` (C).
  - `removeBook()` - calls `deleteBook` and explains the result code.
  - `addStudentAccount()` - checks no `|`, name shorter than 30, not already existing (`userExists()`), then appends `name|pass|student` to `users.txt`.
- **Role-based access** = a Student object simply has no admin functions; a student can never reach them.

### 8.5 `loginUser(name, password)`
Reads `users.txt` line by line; if name and password both match, returns `new Admin(name)` (role `admin`) or `new Student(name)`. Returns `NULL` if nothing matches. The return type is `User*`, so the caller works with either role without knowing which.

---

## 9. `src/main.cpp` - program start (Member 2)
1. `loadBooks()` - C core loads data and builds hash + BST.
2. Prints the title banner.
3. Loop: `1. Login / 0. Exit`.
4. On login: `loginUser(...)`; if `NULL` -> "Wrong username or password."; else greets, calls `u->menu()` (runs the Student or Admin menu until logout), then `delete u`.
5. On exit prints "Goodbye".

---

## 10. `docs/` files
| File | What it is | When to use |
|---|---|---|
| `Project_Report.docx` | Full report: problem, objectives, architecture, concepts, algorithm, tests, limitations, work split | Submission; fill names/roll numbers on page 1 |
| `Viva_QnA.md` | Likely viva questions with simple answers per member | Viva preparation |
| `test_output.txt` | Output of the last `run_tests.sh` run | Proof of testing |
| `Code_Walkthrough.md` | This guide: what every folder, file and function is | Understanding / revision |
| `Deep_Dive.md` | Who calls whom, who owns data, call chains | Understanding the structure |
| `Code_Explained.md` | Every source file with its code and an explanation under each part | Reading the code |
| `Language_and_Compiler.md` | C vs C++ and GCC build steps | Setup / viva |
| `Debugging_Guide.md` | Step-by-step debugging with real error messages | When something breaks |

---

## 11. Concepts - theory, where it works, where it doesn't

### 11.1 Linked list (list.c)
- **Theory:** books are chained; each node holds a pointer to the next.
- **Works:** frequent add/delete (no shifting). Example: admin deletes book 601 - only two pointers change.
- **Doesn't:** jumping to the n-th item; and our `appendNode` walks to the end each time (slow for 100,000 books).

### 11.2 Hashing with chaining (search.c)
- **Theory:** `id % 101` gives the slot; collisions share a slot as a short chain.
- **Works:** exact unique-ID lookup. Example: ID 105 found almost instantly.
- **Doesn't:** range or partial search (IDs 100-200; titles containing "harry").

### 11.3 Binary Search Tree (search.c)
- **Theory:** left smaller, right bigger; in-order walk = sorted.
- **Works:** title lookup and A-Z listing. Example: `the hobbit` found in a few comparisons.
- **Doesn't:** when titles are inserted already sorted, the tree becomes a chain (slow). Partial search still visits every node.

### 11.4 Bubble sort (search.c, recommender)
- **Theory:** repeatedly compare neighbours and swap them if they are in the wrong order; the biggest item "bubbles" to the end each round.
- **Works:** a few hundred books; very easy to read and explain; stable (equal items keep their order).
- **Doesn't:** very big data - about n x n steps (100,000 books would be far too slow).

### 11.5 File handling
- **Works:** small data, human-readable, easy backup.
- **Doesn't:** many users at once; the whole file is rewritten on each change.

### 11.6 Classes, inheritance, polymorphism
- **Classes:** bundle data + functions (`User`, `Student`, `Admin`, `Recommender`).
- **Inheritance:** child reuses parent (`Student`/`Admin` get `searchByTitle` etc. from `User`).
- **Polymorphism:** one call, different behaviour (`u->menu()` runs the Student or the Admin menu).
- **Doesn't help:** tiny programs where a plain function is enough, or deep hierarchies that confuse readers.

### 11.7 Recommendation by genre/author/history
- **Works:** reader with several borrowed books of the same genre/author.
- **Doesn't:** brand-new reader (no history -> message only); reader with 1-2 books (narrow suggestions); ignores story/theme.

---

## 12. Limitations (honest list)
| Limitation | Why | Possible fix |
|---|---|---|
| Passwords stored as plain text | Anyone can read `users.txt` | Store a hash |
| BST not balanced | Sorted inserts make it slow | AVL / balanced tree |
| Bubble sort | n x n steps | Quick/merge sort |
| One copy per book | Cannot hold duplicate copies | Add a "copies" field |
| No due dates/fines | Real libraries need them | Store dates |
| Recommended book may be currently issued | Recommendation ignores availability | Filter `available == 1` |
| Max 500 books, 2000 history lines, 200 per user | Fixed-size arrays (simple, but limited) | Dynamic arrays |
| Console only | Not user-friendly for all | GUI/web |

---

## 13. Testing (what `run_tests.sh` checks)
| Test | Checks | Result |
|---|---|---|
| T1 | Wrong password rejected | Pass |
| T2 | Title search: exact, partial (A-Z), not found (BST) | Pass |
| T3 | Search by ID: found / not found (hash) | Pass |
| T4a/b/c | Sort by title, author, popularity | Pass |
| T5 | Issue, issue again, wrong id, return, return again | Pass |
| T6 | Recommendation scores (204=7, 202=7, then 2s) | Pass |
| T6b | Already-read books never recommended | Pass |
| T7 | No-history student gets a clear message | Pass |
| T8 | Admin add / duplicate / delete issued / delete free | Pass |
| T9 | Admin issued report; duplicate username refused | Pass |
| T11 | Invalid menu input does not crash | Pass |
| T12 | Changes really saved in books.txt / issues.txt | Pass |
| T13 | Very long search word does not crash | Pass |
| T14 | Data files with Windows (CRLF) line endings work | Pass |
| T15 | Missing books.txt: starts with an empty library | Pass |

**Total: 17 passed, 0 failed.** Extra checks done: compiled with `-Wall -Wextra` (0 warnings) and valgrind memory check (0 errors, no leaks) and AddressSanitizer (0 errors).

---

## 14. Jargon glossary
| Word | Simple meaning |
|---|---|
| Compile | Turn source code into a program the computer can run |
| Link | Join the separately compiled pieces into one program |
| Header file (`.h`) | List of names (structures, functions) shared between files |
| Pointer | A variable holding the memory address of something else |
| `NULL` | "Points to nothing" |
| Node | One item of a linked list or tree |
| `malloc` / `free` | Ask for / give back memory |
| Static (file-private) | Visible only inside that one `.c` file |
| Recursive | A function that calls itself (used in BST insert/walk) |
| In-order walk | Visit left subtree, then node, then right subtree |
| Hash collision | Two keys map to the same slot |
| Abstract class | Class with a `= 0` function; only children can be created |
| Virtual function | Function whose version is chosen at run time by the real object |
| Flat file | Plain text file used as storage |
| `extern "C"` | Lets C++ call C functions |
