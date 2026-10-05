# Smart Library - Every Code File, Code + Explanation

How to read: the program has 6 source files. They are shown in this order (shared header, C core, C++ layer, start-up).
Under every code block you get a plain-language explanation. The code is copied **directly from the real files**
(so it is always exact). Words in *italics* are explained where they first appear (full list: `Code_Walkthrough.md` glossary).

| # | File | Owner | One-line role |
|---|---|---|---|
| 1 | `src/library.h` | shared (Member 1) | Common header: `Book` structure + list of C functions |
| 2 | `src/list.c` | Member 1 | Linked list, file handling, add/delete/issue/return |
| 3 | `src/search.c` | Member 2 | Hashing, BST, sorting |
| 4 | `src/oop.h` | Member 3 | Class declarations (User, Student, Admin, Recommender) |
| 5 | `src/oop.cpp` | Member 3 | Class code: recommendation, menus, login |
| 6 | `src/main.cpp` | Member 2 | Program start + login loop |

---

## 1. `library.h` - shared header

**Used by: every file. Defines the Book node and lists every C function the other files may call.**

### `library.h` lines 1-17

```c
/* ====================================================================
 * FILE    : library.h
 * OWNER   : Member 1 (Core Engine) - but EVERY member uses this file
 * PURPOSE : The one shared header. It holds
 *             1) the Book structure (one node of the linked list), and
 *             2) the list of C functions that the C++ code is allowed to call.
 * TOPICS  : Linked list node (Data Structures in C)
 * NOTE    : The extern "C" block is only there so that the C++ files
 *           (oop.cpp, main.cpp) can call the C functions. Nothing else.
 *
 * WHO WRITES WHAT (3-member split)
 *   Member 1 -> list.c    : linked list, file handling, issue / return
 *   Member 2 -> search.c  : hashing, BST, sorting     (+ main.cpp, tests)
 *   Member 3 -> oop.h/.cpp: classes, inheritance, polymorphism, recommendation
 * ==================================================================== */
#ifndef LIBRARY_H
#define LIBRARY_H
```

- Top comment says who owns the file and what it is for (this style is used in every file).
- `#ifndef LIBRARY_H ... #endif` is an *include guard*: if two files include this header, the compiler reads it only once (otherwise "already defined" errors).

### `library.h` lines 19-21

```c
#ifdef __cplusplus
extern "C" {
#endif
```

- `extern "C" {` (only when a C++ compiler reads it) tells C++: "the functions below were compiled as **C**". Without this, linking C and C++ together fails because C++ renames functions internally (*name mangling*).
- C compilers skip this part (they do not define `__cplusplus`).

### `library.h` lines 23-31

```c
#define TITLE_LEN   60
#define AUTHOR_LEN  40
#define GENRE_LEN   30
#define NAME_LEN    30
#define MAX_HISTORY 200   /* max books remembered per user for recommendation */
#define MAX_BOOKS   500   /* max books in the library (fixed-size arrays) */

#define BOOKS_FILE  "data/books.txt"
#define ISSUES_FILE "data/issues.txt"
```

- `#define` = a named constant. These are the maximum text sizes and limits used everywhere. `MAX_BOOKS` (500) and `MAX_HISTORY` (200) are the sizes of the **fixed arrays** used in sorting and recommendation (no `malloc` needed there = fewer bugs).
- `BOOKS_FILE` / `ISSUES_FILE` are *relative paths*: the program must be started from the project folder so `data/` is found.

### `library.h` lines 33-42

```c
/* One book = one node of the linked list */
typedef struct Book {
    int  id;
    char title[TITLE_LEN];
    char author[AUTHOR_LEN];
    char genre[GENRE_LEN];
    int  available;     /* 1 = on shelf, 0 = issued */
    int  timesIssued;   /* how many times issued (used for "sort by popularity") */
    struct Book *next;
} Book;
```

- A `struct` groups related variables into one unit. One `Book` = one book's data.
- `available`: 1 = on shelf, 0 = issued. `timesIssued`: counter used for "sort by popularity".
- `struct Book *next` is a *pointer* (memory address) to the next book. This single field is what makes the books a **linked list**.
- `typedef ... Book;` lets us write `Book` instead of `struct Book`.

### `library.h` lines 44-57

```c
/* ---------- list.c  [Member 1] : linked list, file handling, issue/return ---------- */
void  loadBooks(void);
int   splitLine(char *line, char *parts[], int max);  /* cut a line at each '|' */
void  saveBooks(void);
Book *getHead(void);
int   addBook(int id, const char *title, const char *author, const char *genre); /* 1 ok, 0 duplicate id, -1 library full */
int   deleteBook(int id);              /* 1 ok, 0 not found, -1 book is issued */
void  printBookHeader(void);
void  displayBook(const Book *b);
int   issueBook(const char *user, int id);   /* 1 ok, 0 not found, -1 not available */
int   returnBook(const char *user, int id);  /* 1 ok, 0 no such issue record */
int   getUserHistory(const char *user, int ids[], int max);
void  showUserHistory(const char *user);
void  displayIssuedBooks(void);
```

- The **function list for list.c** (Member 1's work). Only names and parameter types here; the real code is in `list.c`. This is called a *declaration* (prototype).
- `splitLine` is the small helper that cuts a data-file line at every `|`; `oop.cpp` uses it too (for `users.txt`).
- Return codes are documented in the comments (e.g. `deleteBook`: 1 ok, 0 not found, -1 issued; `addBook`: 1 ok, 0 duplicate id, -1 library full).
- `const char *` = "text I will only read, not change".

### `library.h` lines 59-64

```c
/* ---------- search.c [Member 2] : hashing, BST, sorting ---------- */
void  buildIndexes(void);                  /* rebuild hash table + BST from list */
Book *findById(int id);                    /* hashing */
Book *findByTitle(const char *title);      /* BST, exact title */
int   searchTitleContains(const char *word); /* BST in-order walk, prints matches */
void  showSorted(int mode);                /* 1 title, 2 author, 3 popularity */
```

- The **function list for search.c** (Member 2's work): `findById` (hashing), `findByTitle` and `searchTitleContains` (BST), `showSorted` (sorting), and `buildIndexes` (fills the hash table and the BST).

### `library.h` lines 66-69

```c
#ifdef __cplusplus
}
#endif
#endif
```

- Closes the `extern "C" {` block and the include guard. Nothing else is in the file.


---

## 2. `list.c` - Member 1: linked list + file handling

**Concepts: linked list, file handling, issue/return. Uses `findById` and `buildIndexes` from search.c.**

### `list.c` lines 1-22

```c
/* ====================================================================
 * FILE    : list.c
 * OWNER   : Member 1 (Core Engine)
 * PPT     : "Library Core Engine - built in C using DSA"
 * TOPICS  : 1) LINKED LIST  - every book is a node, all books are chained
 *           2) FILE HANDLING - books / issue history saved in text files
 *           3) Book operations: add, delete, display, issue, return
 * NOT HERE: searching & sorting -> search.c (Member 2)
 *           users, menus, recommendation -> oop.cpp (Member 3)
 *
 * SECTIONS IN THIS FILE
 *   A. Linked list helpers        (freeList, appendNode)
 *   B. Books file                 (loadBooks, saveBooks)
 *   C. Add / delete a book        (addBook, deleteBook)
 *   D. Display functions
 *   E. Issue history file         (readRecords, writeRecords)
 *   F. Issue / return / history   (issueBook, returnBook, getUserHistory ...)
 * ==================================================================== */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "library.h"
```

- Header comment: owner Member 1; topics = **linked list** + **file handling**; and what is *not* here (search/sort -> `search.c`, users/recommendation -> `oop.cpp`).
- `#include <stdio.h>` = printing and files (`printf`, `fopen`); `<stdlib.h>` = `malloc`/`free`; `<string.h>` = text functions (`strcpy`, `strcmp`); `"library.h"` = our own header.

### `list.c` lines 24-46

```c
#define MAX_RECORDS 2000

static Book *head = NULL;

/* one line of data/issues.txt:  user|bookId|ISSUED  (or RETURNED) */
typedef struct {
    char user[NAME_LEN];
    int  bookId;
    char status[10];
} Record;

static Record records[MAX_RECORDS];

Book *getHead(void) { return head; }

/* remove '|' and newline because '|' separates fields in our files */
static void cleanText(char *dest, const char *src, int size) {
    int i;
    strncpy(dest, src, size - 1);
    dest[size - 1] = '\0';
    for (i = 0; dest[i] != '\0'; i++)
        if (dest[i] == '|' || dest[i] == '\n') dest[i] = ' ';
}
```

- `head` = pointer to the **first book**. The whole library hangs from this one variable. `static` = private to this file; other files must use `getHead()`.
- `Record` + `records[2000]` = one line of `issues.txt` (`user | bookId | status`) held in memory while we read or rewrite the history file.
- `cleanText` copies text safely: `strncpy` never copies more than `size-1` characters, then we force the ending `\0` (end-of-text marker). Any `|` or newline becomes a space, so a user cannot break the `|`-separated file format.

### `list.c` lines 48-62

```c
/* Cut one line of a data file at every '|' character.
   Example: "201|The Hobbit|Tolkien"  ->  parts[0]="201", parts[1]="The Hobbit", parts[2]="Tolkien"
   Returns how many pieces were found (at most max). Used for books, issues and users files. */
int splitLine(char *line, char *parts[], int max) {
    int n = 0;
    char *p;
    line[strcspn(line, "\r\n")] = '\0';      /* remove the newline at the end of the line */
    p = strtok(line, "|");
    while (p != NULL && n < max) {
        parts[n] = p;
        n++;
        p = strtok(NULL, "|");
    }
    return n;
}
```

- `strcspn(line, "\r\n")` finds where the newline starts, and we put `\0` (end of text) there, so Windows (`\r\n`) and Linux (`\n`) files both work.
- `strtok(line, "|")` returns the first piece up to the next `|`; calling `strtok(NULL, "|")` returns the following pieces. We store each piece's address in `parts[]` and count them.
- The caller checks the count: a book line must give **6** pieces, an issue line **3**, a user line **3**. A damaged line gives a different number and is skipped.
- Note: `strtok` changes the line (puts `\0` where `|` was), so `parts[]` points *inside* `line` - the line must stay alive while `parts` is used.

### `list.c` lines 64-70

```c
/* how many books are in the linked list right now */
static int countBooks(void) {
    int n = 0;
    Book *cur;
    for (cur = head; cur != NULL; cur = cur->next) n++;
    return n;
}
```

- Walks the list and counts the nodes. Used by `loadBooks` so the library can never grow beyond `MAX_BOOKS` (`addBook` checks it too).

### `list.c` lines 72-81

```c
/* ---------------- A + B. linked list helpers and books file ---------------- */
static void freeList(void) {
    Book *cur = head, *nxt;
    while (cur != NULL) {
        nxt = cur->next;
        free(cur);
        cur = nxt;
    }
    head = NULL;
}
```

- Walks the list; for each node it first remembers the next one (`nxt`), then `free(cur)` gives that memory back. (If we freed first we would lose the link.)
- Sets `head = NULL` (= empty list). Used by `loadBooks` before reloading.

### `list.c` lines 83-90

```c
static void appendNode(Book *node) {
    Book *cur;
    node->next = NULL;
    if (head == NULL) { head = node; return; }
    cur = head;
    while (cur->next != NULL) cur = cur->next;
    cur->next = node;
}
```

- Adds a node at the **end**. Empty list -> the node becomes `head`. Otherwise walk to the last node (`cur->next == NULL`) and link the new node there.
- Picture: `head -> [A] -> [B]` becomes `head -> [A] -> [B] -> [NEW]`.
- Limitation: it walks the whole list every time (slow for huge libraries).

### `list.c` lines 92-119

```c
/* read data/books.txt, create one linked-list node per line */
void loadBooks(void) {
    FILE *f = fopen(BOOKS_FILE, "r");
    char line[300];
    char *parts[6];
    Book *b;
    freeList();
    if (f == NULL) {
        printf("(No %s found - starting with an empty library)\n", BOOKS_FILE);
        buildIndexes();
        return;
    }
    while (fgets(line, sizeof(line), f) != NULL) {
        if (splitLine(line, parts, 6) != 6) continue;    /* skip blank / bad line */
        if (countBooks() >= MAX_BOOKS) break;            /* library is full */
        b = (Book *)malloc(sizeof(Book));                /* one new node */
        if (b == NULL) break;
        b->id = atoi(parts[0]);                          /* atoi: text -> number */
        cleanText(b->title,  parts[1], TITLE_LEN);
        cleanText(b->author, parts[2], AUTHOR_LEN);
        cleanText(b->genre,  parts[3], GENRE_LEN);
        b->available   = atoi(parts[4]);
        b->timesIssued = atoi(parts[5]);
        appendNode(b);
    }
    fclose(f);
    buildIndexes();
}
```

- `fopen(..., "r")` opens `books.txt` for reading; `NULL` means the file is missing -> start with an empty library.
- `fgets` reads one line at a time into `line[]`. `splitLine` cuts it into `parts[]`; **exactly 6 parts** means a complete line, otherwise the line is skipped.
- For a good line: `malloc` makes one `Book` node, `atoi` turns the number texts into ints (`id`, `available`, `timesIssued`), `cleanText` copies the texts safely, then `appendNode` adds it to the list. Loading stops at `MAX_BOOKS`.
- At the end `buildIndexes()` (from `search.c`) fills the hash table and BST from the finished list.

### `list.c` lines 121-130

```c
/* write the whole linked list back to data/books.txt */
void saveBooks(void) {
    FILE *f = fopen(BOOKS_FILE, "w");
    Book *cur;
    if (f == NULL) { printf("Error: cannot write %s\n", BOOKS_FILE); return; }
    for (cur = head; cur != NULL; cur = cur->next)
        fprintf(f, "%d|%s|%s|%s|%d|%d\n", cur->id, cur->title, cur->author,
                cur->genre, cur->available, cur->timesIssued);
    fclose(f);
}
```

- Opens `books.txt` with `"w"` (overwrite) and writes every node as one `|`-separated line using `fprintf` (like `printf` but into a file).
- Called after every change (add, delete, issue, return), so data is never lost.

### `list.c` lines 132-151

```c
/* ---------------- C. add / delete a book (linked list operations) ---------------- */
/* add a new book at the END of the linked list, then save the file.
   returns 1 = added, 0 = a book with this id already exists, -1 = library is full */
int addBook(int id, const char *title, const char *author, const char *genre) {
    Book *b;
    if (findById(id) != NULL) return 0;          /* duplicate id */
    if (countBooks() >= MAX_BOOKS) return -1;    /* library is full */
    b = (Book *)malloc(sizeof(Book));
    if (b == NULL) return 0;
    b->id = id;
    cleanText(b->title, title, TITLE_LEN);
    cleanText(b->author, author, AUTHOR_LEN);
    cleanText(b->genre, genre, GENRE_LEN);
    b->available = 1;
    b->timesIssued = 0;
    appendNode(b);
    saveBooks();
    buildIndexes();
    return 1;
}
```

- First `findById(id)` (hashing): if that id already exists -> return 0 (duplicate).
- `malloc` a new node, copy the texts through `cleanText`, set `available = 1` and `timesIssued = 0`, `appendNode`, save the file, rebuild the indexes (so hash/BST know the new book). Return 1.

### `list.c` lines 153-169

```c
/* delete a node: link the previous node to the next one, then free() it.
   returns 1 = deleted, 0 = id not found, -1 = book is issued (not allowed) */
int deleteBook(int id) {
    Book *cur = head, *prev = NULL;
    while (cur != NULL && cur->id != id) {
        prev = cur;
        cur = cur->next;
    }
    if (cur == NULL) return 0;
    if (cur->available == 0) return -1;          /* cannot delete an issued book */
    if (prev == NULL) head = cur->next;
    else prev->next = cur->next;
    free(cur);
    saveBooks();
    buildIndexes();
    return 1;
}
```

- Two pointers: `cur` (the node being checked) and `prev` (the one before it). The loop stops at the matching id.
- Not found -> 0. Issued (`available == 0`) -> -1 (a book someone holds cannot be deleted).
- Deleting the first node: `head` moves forward. Otherwise `prev->next = cur->next` makes the list **skip** the node: `[A] -> [B] -> [C]` becomes `[A] -> [C]`.
- `free(cur)` releases memory; then save and `buildIndexes()` - essential, otherwise the hash table/BST would still point to freed memory.

### `list.c` lines 171-182

```c
/* ---------------- D. display ---------------- */
void printBookHeader(void) {
    printf("%-5s %-40s %-20s %-12s %-9s %s\n",
           "ID", "Title", "Author", "Genre", "Status", "Issued");
    printf("-----------------------------------------------------------------------------------------------\n");
}

void displayBook(const Book *b) {
    printf("%-5d %-40.40s %-20.20s %-12.12s %-9s %d\n",
           b->id, b->title, b->author, b->genre,
           b->available ? "Available" : "Issued", b->timesIssued);
}
```

- Table printing. `%-40.40s` = text, left-aligned, exactly 40 characters wide, cut if longer. `%-9s` pads to 9.
- `displayBook` prints one row; `b->available ? "Available" : "Issued"` is the *ternary* shortcut for if/else.
- Also used by `search.c` and `oop.cpp`, so every table looks the same.

### `list.c` lines 184-202

```c
/* ---------------- E + F. issue history file and issue / return ----------------
   data/issues.txt keeps one line per borrow:  user|bookId|ISSUED or RETURNED.
   This same file is the "reading history" used by the recommendation engine. */
static int readRecords(void) {
    FILE *f = fopen(ISSUES_FILE, "r");
    char line[200];
    char *parts[3];
    int n = 0;
    if (f == NULL) return 0;
    while (n < MAX_RECORDS && fgets(line, sizeof(line), f) != NULL) {
        if (splitLine(line, parts, 3) != 3) continue;    /* skip blank / bad line */
        cleanText(records[n].user, parts[0], NAME_LEN);
        records[n].bookId = atoi(parts[1]);
        cleanText(records[n].status, parts[2], 10);
        n++;
    }
    fclose(f);
    return n;
}
```

- Loads every line of `issues.txt` into the `records[]` array and returns how many lines were read. Same `fgets` + `splitLine` pattern as `loadBooks`; **3 parts** = `user | bookId | status`.
- This history file doubles as the **reading history** for recommendations.

### `list.c` lines 204-211

```c
static void writeRecords(int n) {
    FILE *f = fopen(ISSUES_FILE, "w");
    int i;
    if (f == NULL) { printf("Error: cannot write %s\n", ISSUES_FILE); return; }
    for (i = 0; i < n; i++)
        fprintf(f, "%s|%d|%s\n", records[i].user, records[i].bookId, records[i].status);
    fclose(f);
}
```

- The opposite of `readRecords`: writes the whole `records[]` array back to `issues.txt` (mode `"w"` overwrites). Needed when a line changes from ISSUED to RETURNED.

### `list.c` lines 213-229

```c
/* give a book to a user: mark it not available and add an ISSUED line.
   returns 1 = ok, 0 = no such book, -1 = already issued to someone */
int issueBook(const char *user, int id) {
    Book *b = findById(id);
    FILE *f;
    if (b == NULL) return 0;
    if (b->available == 0) return -1;
    b->available = 0;
    b->timesIssued++;
    f = fopen(ISSUES_FILE, "a");
    if (f != NULL) {
        fprintf(f, "%s|%d|ISSUED\n", user, id);
        fclose(f);
    }
    saveBooks();
    return 1;
}
```

- `findById` (hashing) gets the book. Not found -> 0. Already issued -> -1.
- Otherwise: `available = 0`, `timesIssued++`, then `fopen(..., "a")` (**append**) adds one new line `user|id|ISSUED` to the history, and `saveBooks()` updates `books.txt`. Returns 1.

### `list.c` lines 231-248

```c
/* take a book back: change that user's ISSUED line to RETURNED and mark the
   book available again. returns 1 = ok, 0 = this user has not issued it */
int returnBook(const char *user, int id) {
    int n = readRecords(), i;
    Book *b;
    for (i = 0; i < n; i++) {
        if (strcmp(records[i].user, user) == 0 && records[i].bookId == id &&
            strcmp(records[i].status, "ISSUED") == 0) {
            strcpy(records[i].status, "RETURNED");
            writeRecords(n);
            b = findById(id);
            if (b != NULL) b->available = 1;
            saveBooks();
            return 1;
        }
    }
    return 0;
}
```

- Loads the history; looks for a line with **this user + this book id + status ISSUED** (so nobody can return someone else's book).
- Found: change the status to `RETURNED` (`strcpy`), `writeRecords` saves the file, set the book `available = 1`, `saveBooks`. Return 1. Not found -> 0.

### `list.c` lines 250-256

```c
/* ids of all books this user has ever borrowed (used by the recommender) */
int getUserHistory(const char *user, int ids[], int max) {
    int n = readRecords(), i, count = 0;
    for (i = 0; i < n && count < max; i++)
        if (strcmp(records[i].user, user) == 0) ids[count++] = records[i].bookId;
    return count;
}
```

- Collects the ids of **every** book this user has ever borrowed (issued or returned) into the array `ids[]`, and returns the count. `max` stops it from overflowing the array.
- Used only by the recommender (`oop.cpp`).

### `list.c` lines 258-273

```c
void showUserHistory(const char *user) {
    int n = readRecords(), i, found = 0;
    Book *b;
    for (i = 0; i < n; i++) {
        if (strcmp(records[i].user, user) != 0) continue;
        if (!found) {
            printf("%-5s %-40s %-10s\n", "ID", "Title", "Status");
            printf("----------------------------------------------------\n");
            found = 1;
        }
        b = findById(records[i].bookId);
        printf("%-5d %-40.40s %-10s\n", records[i].bookId,
               b ? b->title : "(book removed)", records[i].status);
    }
    if (!found) printf("No reading history yet.\n");
}
```

- Prints the user's history as a table (id, title, status). `found` makes the header print only once, before the first row.
- `b ? b->title : "(book removed)"` handles a book the admin deleted later.

### `list.c` lines 275-291

```c
/* admin report: who currently holds which book */
void displayIssuedBooks(void) {
    int n = readRecords(), i, found = 0;
    Book *b;
    for (i = 0; i < n; i++) {
        if (strcmp(records[i].status, "ISSUED") != 0) continue;
        if (!found) {
            printf("%-12s %-5s %-40s\n", "Issued to", "ID", "Title");
            printf("-----------------------------------------------------\n");
            found = 1;
        }
        b = findById(records[i].bookId);
        printf("%-12s %-5d %-40.40s\n", records[i].user, records[i].bookId,
               b ? b->title : "(book removed)");
    }
    if (!found) printf("No books are currently issued.\n");
}
```

- Admin report: every history line whose status is `ISSUED`, shown with the username and book title = "who holds which book right now".


---

## 3. `search.c` - Member 2: hashing, BST, sorting

**Concepts: hashing with chaining, binary search tree, bubble sort. Uses `getHead`, `printBookHeader`, `displayBook` from list.c.**

### `search.c` lines 1-31

```c
/* ====================================================================
 * FILE    : search.c
 * OWNER   : Member 2 (Search & Algorithms)
 * PPT     : "Fast title search via Binary Search Tree",
 *           "Instant lookup by unique ID via hashing",
 *           "Sorting by title, author, or popularity"
 * TOPICS  : 1) HASHING (chaining)          -> find a book by its ID
 *           2) BINARY SEARCH TREE (BST)    -> find a book by its title
 *           3) SORTING (bubble sort)       -> by title / author / popularity
 * NOT HERE: linked list & files -> list.c (Member 1)
 *           menus, recommendation -> oop.cpp (Member 3)
 *
 * HOW IT FITS: the hash table and the BST only store POINTERS to the
 * linked-list nodes created in list.c, so no data is copied. They are
 * rebuilt (buildIndexes) whenever a book is added, deleted or loaded.
 *
 * SECTIONS IN THIS FILE
 *   A. small helpers (compareText, containsText)
 *   B. Hashing        (hashInsert, findById)
 *   C. BST            (bstInsert, findByTitle, searchTitleContains)
 *   D. buildIndexes   (fills both structures)
 *   E. Sorting        (showSorted)
 * ==================================================================== */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "library.h"

/* ---------------- A. helpers ---------------- */
#define BUF 200      /* big enough for any title or search word we compare */
```

- Header: owner Member 2; topics = **hashing**, **BST**, **sorting**. Important line: the hash table and BST store only **pointers** to the nodes made in `list.c` - no data is copied.
- `<ctype.h>` provides `tolower` (to ignore upper/lower case).

### `search.c` lines 33-39

```c
/* copy src into dest in lower case (so "HARRY" and "harry" look the same) */
static void toLower(const char *src, char *dest) {
    int i;
    for (i = 0; src[i] != '\0' && i < BUF - 1; i++)
        dest[i] = (char)tolower((unsigned char)src[i]);
    dest[i] = '\0';
}
```

- Copies `src` into `dest` with every letter converted to lower case (`tolower`). The loop stops at `BUF - 1` characters, so even a very long search word cannot overflow the array (test T13).
- Everything that must ignore upper/lower case first goes through this function.

### `search.c` lines 41-48

```c
/* compare two texts ignoring upper/lower case. Result like strcmp:
   negative = a comes first, 0 = same, positive = b comes first */
static int compareText(const char *a, const char *b) {
    char x[BUF], y[BUF];
    toLower(a, x);
    toLower(b, y);
    return strcmp(x, y);
}
```

- Like `strcmp` but **ignores case**, so `HARRY` equals `harry`: lower-case both texts with `toLower`, then plain `strcmp`. Returns negative (a before b), 0 (same), or positive (a after b).

### `search.c` lines 50-56

```c
/* does "text" contain "word" anywhere (ignoring case)? 1 = yes, 0 = no */
static int containsText(const char *text, const char *word) {
    char x[BUF], y[BUF];
    toLower(text, x);
    toLower(word, y);
    return strstr(x, y) != NULL;
}
```

- "Is `word` somewhere inside `text`?" (case ignored). Lower-case both, then `strstr` (a standard function that finds a text inside another text) returns non-NULL if found.
- Used for the partial title search (`harry` inside `Harry Potter and ...`).

### `search.c` lines 58-70

```c
/* ================= B. HASHING (by book ID) =================
   hash function: id % 101 gives the slot number; books that land in the
   same slot are chained in a small linked list (chaining). */
#define HASH_SIZE 101

typedef struct HNode {
    Book *book;
    struct HNode *next;      /* chaining: books with the same hash value */
} HNode;

static HNode *table[HASH_SIZE];

static int hashId(int id) { return id % HASH_SIZE; }
```

- **Hash table** = an array of 101 slots (`table[101]`). The slot of a book is `id % 101` (remainder).
- Example: `105 % 101 = 4` -> slot 4. `201 % 101 = 100` -> slot 100.
- *Collision*: ids 101, 202, 303, 404 all give slot 0. They are *chained*: each slot holds a small linked list of `HNode`s (`book` pointer + `next`).

### `search.c` lines 72-79

```c
static void hashInsert(Book *b) {
    int h = hashId(b->id);
    HNode *n = (HNode *)malloc(sizeof(HNode));
    if (n == NULL) return;
    n->book = b;
    n->next = table[h];
    table[h] = n;
}
```

- Computes the slot, `malloc`s a hash node holding the **pointer** to the book, and puts it at the **front** of that slot's chain (`n->next = table[h]; table[h] = n;`).

### `search.c` lines 81-89

```c
static void hashClear(void) {
    int i;
    HNode *cur, *nxt;
    for (i = 0; i < HASH_SIZE; i++) {
        cur = table[i];
        while (cur != NULL) { nxt = cur->next; free(cur); cur = nxt; }
        table[i] = NULL;
    }
}
```

- Empties all 101 slots, freeing every chain node (not the books themselves - those belong to `list.c`). Called before rebuilding.

### `search.c` lines 91-98

```c
Book *findById(int id) {
    HNode *cur = table[hashId(id)];
    while (cur != NULL) {
        if (cur->book->id == id) return cur->book;
        cur = cur->next;
    }
    return NULL;
}
```

- Jump straight to slot `id % 101` and check only that slot's short chain. Match -> return the book pointer, else `NULL`.
- This is why ID search is almost instant, even with thousands of books.

### `search.c` lines 100-107

```c
/* ================= C. BINARY SEARCH TREE (by title) =================
   smaller titles go to the left, bigger ones to the right (A-Z order). */
typedef struct TNode {
    Book *book;
    struct TNode *left, *right;
} TNode;

static TNode *root = NULL;
```

- **Binary Search Tree** node: a book pointer plus a `left` and a `right` child. Rule: **smaller title (A-Z) goes left, bigger goes right**.
- `root` = the top node of the tree.

### `search.c` lines 109-122

```c
static TNode *bstInsert(TNode *node, Book *b) {
    if (node == NULL) {
        node = (TNode *)malloc(sizeof(TNode));
        if (node == NULL) return NULL;
        node->book = b;
        node->left = node->right = NULL;
        return node;
    }
    if (compareText(b->title, node->book->title) < 0)
        node->left = bstInsert(node->left, b);
    else
        node->right = bstInsert(node->right, b);
    return node;
}
```

- *Recursive* (calls itself). Empty spot found (`node == NULL`) -> create the node there.
- Otherwise compare titles: smaller -> insert into the left subtree, else (bigger or equal) -> right subtree. The returned pointer re-attaches the (possibly new) subtree.

### `search.c` lines 124-129

```c
static void bstClear(TNode *node) {
    if (node == NULL) return;
    bstClear(node->left);
    bstClear(node->right);
    free(node);
}
```

- Frees the whole tree: clear left, clear right, then free this node (children first, otherwise we would lose them).

### `search.c` lines 131-140

```c
Book *findByTitle(const char *title) {
    TNode *cur = root;
    int c;
    while (cur != NULL) {
        c = compareText(title, cur->book->title);
        if (c == 0) return cur->book;
        cur = (c < 0) ? cur->left : cur->right;
    }
    return NULL;
}
```

- **Exact** title search: start at `root`, compare; equal -> found, smaller -> go left, bigger -> go right. Each step throws away about half of the remaining titles, so it is fast. Not found -> `NULL`.

### `search.c` lines 142-158

```c
/* in-order walk = titles come out in A-Z order */
static void walkContains(TNode *node, const char *word, int *count) {
    if (node == NULL) return;
    walkContains(node->left, word, count);
    if (containsText(node->book->title, word)) {
        if (*count == 0) printBookHeader();
        displayBook(node->book);
        (*count)++;
    }
    walkContains(node->right, word, count);
}

int searchTitleContains(const char *word) {
    int count = 0;
    walkContains(root, word, &count);
    return count;
}
```

- **In-order walk**: (1) left subtree, (2) this node, (3) right subtree. On a BST this visits titles in **A-Z order**.
- At each node: if the title contains the word, print it (header only before the first hit) and add 1 to `count`.
- `int *count` is a pointer so every recursive call updates the **same** counter.
- `searchTitleContains` just starts the walk at `root` and returns the count (0 = nothing found).

### `search.c` lines 160-173

```c
/* ---- D. build / rebuild both structures from the linked list ---- */
void buildIndexes(void) {
    Book *cur;
    hashClear();
    bstClear(root);
    root = NULL;
    for (cur = getHead(); cur != NULL; cur = cur->next) {
        hashInsert(cur);
        root = bstInsert(root, cur);
    }
}

/* ================= E. SORTING (bubble sort) =================
   mode 1 = title A-Z, mode 2 = author A-Z, mode 3 = most issued first */
```

- Clears the old hash table and tree, then walks the linked list **once** and inserts every book into both.
- Called after load, add and delete. (Issue/return only change fields inside the same nodes, so no rebuild is needed.)

### `search.c` lines 175-180

```c
/* returns 1 if book a must come AFTER book b for the chosen mode */
static int isAfter(const Book *a, const Book *b, int mode) {
    if (mode == 1) return compareText(a->title, b->title) > 0;
    if (mode == 2) return compareText(a->author, b->author) > 0;
    return a->timesIssued < b->timesIssued;       /* mode 3: most popular first */
}
```

- The only part that changes between the three sort modes. Returns true if book `a` must come **after** `b`:
  mode 1 = title A-Z, mode 2 = author A-Z, mode 3 = fewer issues goes later (so **most issued first**).

### `search.c` lines 182-207

```c
void showSorted(int mode) {
    Book *arr[MAX_BOOKS];       /* pointers to the books (the linked list itself is not changed) */
    Book *cur, *temp;
    int n = 0, i, j;

    for (cur = getHead(); cur != NULL && n < MAX_BOOKS; cur = cur->next) {
        arr[n] = cur;
        n++;
    }
    if (n == 0) { printf("No books in the library.\n"); return; }

    /* bubble sort: compare neighbours, swap them if they are in the wrong order.
       After each round the "biggest" item has moved to the end. */
    for (i = 0; i < n - 1; i++) {
        for (j = 0; j < n - 1 - i; j++) {
            if (isAfter(arr[j], arr[j + 1], mode)) {
                temp = arr[j];
                arr[j] = arr[j + 1];
                arr[j + 1] = temp;
            }
        }
    }

    printBookHeader();
    for (i = 0; i < n; i++) displayBook(arr[i]);
}
```

- Copies pointers to all books into a **fixed array** `arr[MAX_BOOKS]` (the linked list order is untouched) and counts them in `n`.
- **Bubble sort**: compare neighbours `arr[j]` and `arr[j+1]`; if `isAfter(...)` says they are in the wrong order, swap them. After each round of the inner loop the "biggest" item has moved to the end, so the next round can stop one place earlier (`n - 1 - i`).
- Print the sorted array. Nothing to free (fixed array).
- Mini example sorting `5, 2, 4`: round 1: `[5,2,4]` -> `[2,5,4]` -> `[2,4,5]`; round 2: no swap needed -> sorted.


---

## 4. `oop.h` - Member 3: class declarations

**Concepts: classes, inheritance, polymorphism (virtual functions).**

### `oop.h` lines 1-23

```cpp
// ====================================================================
// FILE    : oop.h
// OWNER   : Member 3 (OOP & Recommendation)
// PPT     : "OOP model (Book, User, Library) with inheritance & polymorphism",
//           "Role-based access for Student and Admin accounts",
//           "Personalized recommendations by genre, author & reading history"
// TOPICS  : 1) CLASSES       - User, Student, Admin, Recommender
//           2) INHERITANCE   - Student and Admin inherit User
//           3) POLYMORPHISM  - virtual menu(): same call, different menu per role
// NOT HERE: linked list / files -> list.c, hashing / BST / sorting -> search.c
//
// CLASS MAP
//   User (abstract)        Recommender (separate class: suggests books)
//    |-- Student
//    |-- Admin
// ====================================================================
#ifndef OOP_H
#define OOP_H

#include <string>
#include "library.h"

#define USERS_FILE "data/users.txt"
```

- Header comment with the **class map** (who inherits from whom) and the PPT topics covered: classes, inheritance, polymorphism, role-based access.
- `#ifndef OOP_H ... #endif` = include guard (read only once).
- Then the includes: `<string>` (C++ text type `std::string`), our `library.h` (so C++ can use `Book`, `findById` ...), and the users-file path `USERS_FILE`.

### `oop.h` lines 25-27

```cpp
// ---------- small input helpers ----------
std::string readLine(const char *prompt);
int readInt(const char *prompt);
```

- Declarations of two input helpers (written in `oop.cpp`): `readLine` reads a full line, `readInt` reads a number.

### `oop.h` lines 29-33

```cpp
// ---------- recommendation (genre + author + reading history) ----------
// Gives every book the user has NOT read a score and prints the best ones:
//   +2 points for every book in the history with the same genre
//   +3 points for every book in the history with the same author
#define TOP_BOOKS 5            // how many suggestions to show
```

- The comment explains the scoring in two lines. `TOP_BOOKS` = how many suggestions are shown (5).

### `oop.h` lines 35-38

```cpp
class Recommender {
public:
    void recommend(const std::string &user);
};
```

- A class with **one** function, `recommend(user)`. It is a normal class (no inheritance needed): it reads the history, scores every unread book and prints the best `TOP_BOOKS`. Written in `oop.cpp`.

### `oop.h` lines 40-55

```cpp
// ---------- users (INHERITANCE + role-based access) ----------
class User {
protected:
    std::string username;
public:
    User(const std::string &name) : username(name) {}
    virtual ~User() {}
    std::string getName() const { return username; }
    virtual std::string getRole() = 0;     // "Student" or "Admin"
    virtual void menu() = 0;               // each role has its OWN menu (polymorphism)

    // features common to every user (written once here, used by both children)
    void searchByTitle();      // calls the BST in search.c
    void searchById();         // calls the hash table in search.c
    void viewSorted();         // calls the sorting in search.c
};
```

- Abstract base class for every logged-in person. `username` is `protected`.
- `getRole()` and `menu()` are **pure virtual** -> Student and Admin must each write their own. This is what makes `u->menu()` behave differently per role (*polymorphism*).
- The three normal functions at the bottom (`searchByTitle`, `searchById`, `viewSorted`) are written **once** in `User` and inherited by both children.

### `oop.h` lines 57-62

```cpp
class Student : public User {
public:
    Student(const std::string &name) : User(name) {}
    std::string getRole() { return "Student"; }
    void menu();
};
```

- Child of `User` (`: public User`). Provides `getRole()` (returns `"Student"`) and declares `menu()` (code in `oop.cpp`). The constructor passes the name up to `User`.

### `oop.h` lines 64-73

```cpp
class Admin : public User {
public:
    Admin(const std::string &name) : User(name) {}
    std::string getRole() { return "Admin"; }
    void menu();
private:
    void addNewBook();
    void removeBook();
    void addStudentAccount();
};
```

- Child of `User`. Besides `menu()` it has three `private:` helper functions that only the Admin class can call: add a book, remove a book, add a student account. A Student object has none of these = **role-based access**.

### `oop.h` lines 75-78

```cpp
// login: returns a new Student or Admin object, or NULL if name/password is wrong
User *loginUser(const std::string &name, const std::string &password);

#endif
```

- `loginUser` returns a `User*` (pointer to a Student **or** an Admin object, or `NULL`). `#endif` closes the include guard.


---

## 5. `oop.cpp` - Member 3: recommendation, menus, login

**Concepts: classes, inheritance, polymorphism. Calls the C core through library.h.**

### `oop.cpp` lines 1-22

```cpp
// ====================================================================
// FILE    : oop.cpp
// OWNER   : Member 3 (OOP & Recommendation)
// TOPICS  : classes, inheritance, polymorphism (see oop.h for the class map)
// NOT HERE: linked list & files (list.c), hashing / BST / sorting (search.c)
//           - this file only CALLS those C functions.
//
// SECTIONS IN THIS FILE
//   A. input helpers
//   B. Recommender                          (genre, author, reading history)
//   C. features common to all users         (User class)
//   D. Student menu
//   E. Admin menu
//   F. login (role-based: creates a Student or an Admin object)
// ====================================================================
#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include "oop.h"

using namespace std;
```

- Header lists the sections A-F of the file. `using namespace std;` lets us write `cout`/`string` instead of `std::cout`/`std::string`.
- `<cstdio>`, `<cstdlib>`, `<cstring>` are the C++ names of the C headers (for `printf`, `exit`, `atoi`, `strcmp`).

### `oop.cpp` lines 24-33

```cpp
// ================= A. input helpers =================
string readLine(const char *prompt) {
    string s;
    cout << prompt;
    if (!getline(cin, s)) {            // input closed (Ctrl+D / end of test file)
        cout << "\nInput ended. Exiting.\n";
        exit(0);
    }
    return s;
}
```

- Prints the prompt, then `getline(cin, s)` reads the **whole line** (spaces allowed, e.g. a book title).
- If input ends (Ctrl+D, or the end of a test script) `getline` fails; we print a message and `exit(0)` - otherwise menus would loop forever.

### `oop.cpp` lines 35-38

```cpp
int readInt(const char *prompt) {
    string s = readLine(prompt);
    return atoi(s.c_str());
}
```

- Reads a line, then `atoi` converts text to a number. Letters give `0`, which the menus treat as "Logout/Exit" or "Invalid" - the program never crashes on bad input.

### `oop.cpp` lines 40-99

```cpp
// ================= B. Recommender =================
// Recommend books for one user:
//   1. read the user's reading history (data/issues.txt, through list.c)
//   2. every book the user has NOT read gets a score:
//        +2 for each history book with the same genre
//        +3 for each history book with the same author
//   3. sort by score (highest first) and print the top books
void Recommender::recommend(const string &user) {
    int history[MAX_HISTORY];
    int n = getUserHistory(user.c_str(), history, MAX_HISTORY);

    if (n == 0) {                       // no history = nothing to learn from
        cout << "No reading history yet. Issue a few books first,\n"
             << "then ask again - suggestions are based on what you have read.\n";
        return;
    }

    Book *cand[MAX_BOOKS];              // books that can be recommended
    int score[MAX_BOOKS];               // score of each of those books
    int m = 0;                          // how many candidates we have

    // ---- step 2: score every book ----
    for (Book *b = getHead(); b != NULL && m < MAX_BOOKS; b = b->next) {
        int s = 0;
        bool alreadyRead = false;
        for (int i = 0; i < n; i++) {
            if (history[i] == b->id) { alreadyRead = true; break; }
            Book *old = findById(history[i]);          // a book the user read earlier
            if (old == NULL) continue;                 // that book was deleted later
            if (strcmp(old->genre, b->genre) == 0) s += 2;
            if (strcmp(old->author, b->author) == 0) s += 3;
        }
        if (alreadyRead || s == 0) continue;           // skip read books and no-match books
        cand[m] = b;
        score[m] = s;
        m++;
    }

    // ---- step 3: bubble sort, highest score first ----
    for (int i = 0; i < m - 1; i++) {
        for (int j = 0; j < m - 1 - i; j++) {
            if (score[j] < score[j + 1]) {             // wrong order -> swap both arrays
                Book *tb = cand[j];  cand[j] = cand[j + 1];   cand[j + 1] = tb;
                int ts = score[j];   score[j] = score[j + 1]; score[j + 1] = ts;
            }
        }
    }

    // ---- print ----
    if (m == 0) {
        cout << "No similar unread books found right now.\n";
        return;
    }
    cout << "Based on your reading history (genre and author):\n";
    printf("%-5s %-40s %-20s %-12s %s\n", "ID", "Title", "Author", "Genre", "Score");
    printf("---------------------------------------------------------------------------------------\n");
    for (int i = 0; i < m && i < TOP_BOOKS; i++)
        printf("%-5d %-40.40s %-20.20s %-12.12s %d\n", cand[i]->id, cand[i]->title,
               cand[i]->author, cand[i]->genre, score[i]);
}
```

Step by step:
1. `getUserHistory` (C) fills `history[]` with the ids this user borrowed. **No history -> print a message and return** (nothing to learn from).
2. Two fixed arrays: `cand[]` (candidate books) and `score[]` (their scores); `m` counts the candidates.
3. For each book in the library: if its id is in the history, mark it `alreadyRead`. For every history book, `findById` gets the old book; same **genre** -> `s += 2`, same **author** -> `s += 3`.
4. Skip books already read or with `s == 0`; keep the rest.
5. **Bubble sort** on `cand` and `score` together (swap both arrays at the same time), highest score first (equal scores keep file order).
6. Print the top `TOP_BOOKS` with `printf`. No `new`/`delete` is used here.

### `oop.cpp` lines 101-114

```cpp
// ================= C. common user features (User class) =================
void User::searchByTitle() {
    string word = readLine("Enter title (or part of it): ");
    if (word.empty()) { cout << "Nothing entered.\n"; return; }

    Book *exact = findByTitle(word.c_str());      // BST exact lookup
    if (exact != NULL) {
        printBookHeader();
        displayBook(exact);
        return;
    }
    if (searchTitleContains(word.c_str()) == 0)   // BST walk for partial matches
        cout << "No book found with \"" << word << "\".\n";
}
```

- Reads the text. **First** tries `findByTitle` (BST exact match) - found: print and stop. **Otherwise** `searchTitleContains` (BST in-order walk) prints all partial matches in A-Z order; if it returns 0, say "No book found".

### `oop.cpp` lines 116-122

```cpp
void User::searchById() {
    int id = readInt("Enter book ID: ");
    Book *b = findById(id);                       // hashing
    if (b == NULL) { cout << "No book with ID " << id << ".\n"; return; }
    printBookHeader();
    displayBook(b);
}
```

- Reads a number, `findById` (hash table). `NULL` -> "No book with ID". Else print one row.

### `oop.cpp` lines 124-129

```cpp
void User::viewSorted() {
    cout << "Sort by: 1) Title  2) Author  3) Popularity\n";
    int mode = readInt("Choice: ");
    if (mode < 1 || mode > 3) { cout << "Invalid choice.\n"; return; }
    showSorted(mode);
}
```

- Asks 1 = title, 2 = author, 3 = popularity; checks the range; calls `showSorted(mode)` from `search.c`.

### `oop.cpp` lines 131-166

```cpp
// ================= D. Student (child of User) =================
void Student::menu() {
    Recommender rec;
    int choice;
    do {
        cout << "\n===== STUDENT MENU (" << username << ") =====\n"
             << "1. Search book by title\n"
             << "2. Search book by ID\n"
             << "3. View all books (sorted)\n"
             << "4. Issue a book\n"
             << "5. Return a book\n"
             << "6. My reading history\n"
             << "7. Recommend books for me\n"
             << "0. Logout\n";
        choice = readInt("Choice: ");
        cout << "\n";
        if (choice == 1) searchByTitle();
        else if (choice == 2) searchById();
        else if (choice == 3) viewSorted();
        else if (choice == 4) {
            int id = readInt("Enter ID of the book to issue: ");
            int r = issueBook(username.c_str(), id);
            if (r == 1) cout << "Book issued successfully.\n";
            else if (r == 0) cout << "No book with ID " << id << ".\n";
            else cout << "Sorry, this book is already issued to someone else.\n";
        }
        else if (choice == 5) {
            int id = readInt("Enter ID of the book to return: ");
            if (returnBook(username.c_str(), id)) cout << "Book returned successfully.\n";
            else cout << "You have not issued a book with this ID.\n";
        }
        else if (choice == 6) showUserHistory(username.c_str());
        else if (choice == 7) rec.recommend(username);
        else if (choice != 0) cout << "Invalid choice.\n";
    } while (choice != 0);
}
```

- A `do { ... } while (choice != 0)` loop: show options, read a number, run the matching feature, repeat until `0` (Logout).
- Options 1-3 are the inherited common features. 4 and 5 call `issueBook` / `returnBook` with `username` (so every action is for **this** student) and explain the return code (1 ok, 0 no such book, -1 already issued). 6 = `showUserHistory`. 7 = `rec.recommend(username)`.
- `username.c_str()` converts the C++ `string` into the plain C text the C functions expect.

### `oop.cpp` lines 168-183

```cpp
// ================= E. Admin (child of User) =================
void Admin::addNewBook() {
    int id = readInt("Book ID (number): ");
    if (id <= 0) { cout << "ID must be a positive number.\n"; return; }
    string title  = readLine("Title: ");
    string author = readLine("Author: ");
    string genre  = readLine("Genre: ");
    if (title.empty() || author.empty() || genre.empty()) {
        cout << "Title, author and genre cannot be empty.\n";
        return;
    }
    int r = addBook(id, title.c_str(), author.c_str(), genre.c_str());
    if (r == 1) cout << "Book added.\n";
    else if (r == 0) cout << "A book with this ID already exists.\n";
    else cout << "The library is full (maximum " << MAX_BOOKS << " books).\n";
}
```

- Reads id, title, author, genre. Checks: id must be positive, no empty fields. Then `addBook` (C): returns 1 -> "Book added", 0 -> "already exists", -1 -> "library is full".

### `oop.cpp` lines 185-191

```cpp
void Admin::removeBook() {
    int id = readInt("Enter ID of the book to delete: ");
    int r = deleteBook(id);
    if (r == 1) cout << "Book deleted.\n";
    else if (r == 0) cout << "No book with ID " << id << ".\n";
    else cout << "This book is currently issued, so it cannot be deleted.\n";
}
```

- Calls `deleteBook` and turns its code into a message: 1 deleted, 0 no such id, -1 "currently issued, cannot be deleted".

### `oop.cpp` lines 193-208

```cpp
// users.txt line:  name|password|role
// returns true if a user with this name is already in the file
static bool userExists(const string &name) {
    FILE *f = fopen(USERS_FILE, "r");
    char line[200];
    char *parts[3];
    if (f == NULL) return false;
    while (fgets(line, sizeof(line), f) != NULL) {
        if (splitLine(line, parts, 3) == 3 && name == parts[0]) {   // splitLine is in list.c
            fclose(f);
            return true;
        }
    }
    fclose(f);
    return false;
}
```

- Reads `users.txt` line by line; `splitLine` cuts it into name | password | role. Returns true if `parts[0]` equals `name`. Used so two accounts cannot share a username.

### `oop.cpp` lines 210-225

```cpp
void Admin::addStudentAccount() {
    string name = readLine("New student username: ");
    string pass = readLine("Password: ");
    if (name.empty() || pass.empty() ||
        name.find('|') != string::npos || pass.find('|') != string::npos ||
        name.length() >= NAME_LEN) {
        cout << "Invalid username/password (no '|' allowed, username under 30 characters).\n";
        return;
    }
    if (userExists(name)) { cout << "This username already exists.\n"; return; }
    FILE *f = fopen(USERS_FILE, "a");
    if (f == NULL) { cout << "Error: cannot write users file.\n"; return; }
    fprintf(f, "%s|%s|student\n", name.c_str(), pass.c_str());
    fclose(f);
    cout << "Student account created.\n";
}
```

- Reads username and password. Rejects empty values, a `|` character (it would break the file format) or a name longer than 29 characters. Checks `userExists`. Then appends `name|password|student` to `users.txt` (mode `"a"`).

### `oop.cpp` lines 227-250

```cpp
void Admin::menu() {
    int choice;
    do {
        cout << "\n===== ADMIN MENU (" << username << ") =====\n"
             << "1. Add a book\n"
             << "2. Delete a book\n"
             << "3. View all books (sorted)\n"
             << "4. Search book by title\n"
             << "5. Search book by ID\n"
             << "6. View currently issued books\n"
             << "7. Create a student account\n"
             << "0. Logout\n";
        choice = readInt("Choice: ");
        cout << "\n";
        if (choice == 1) addNewBook();
        else if (choice == 2) removeBook();
        else if (choice == 3) viewSorted();
        else if (choice == 4) searchByTitle();
        else if (choice == 5) searchById();
        else if (choice == 6) displayIssuedBooks();
        else if (choice == 7) addStudentAccount();
        else if (choice != 0) cout << "Invalid choice.\n";
    } while (choice != 0);
}
```

- Same loop pattern as the student menu, with admin options: add/delete book, sorted list, searches (inherited), issued-books report (`displayIssuedBooks`), create student account. **A student object has no way to reach these** (role-based access).

### `oop.cpp` lines 252-270

```cpp
// ================= F. login (role-based access) =================
// users.txt decides the role; a Student or an Admin object is created and
// returned as a User* - the caller does not need to know which one it is.
User *loginUser(const string &name, const string &password) {
    FILE *f = fopen(USERS_FILE, "r");
    char line[200];
    char *parts[3];                     // parts[0]=name, parts[1]=password, parts[2]=role
    if (f == NULL) return NULL;
    while (fgets(line, sizeof(line), f) != NULL) {
        if (splitLine(line, parts, 3) != 3) continue;      // skip bad / blank line
        if (name == parts[0] && password == parts[1]) {    // both must match
            fclose(f);
            if (strcmp(parts[2], "admin") == 0) return new Admin(name);
            return new Student(name);
        }
    }
    fclose(f);
    return NULL;
}
```

- Reads `users.txt` line by line and cuts each line with `splitLine` (`parts[0]` name, `parts[1]` password, `parts[2]` role). A line without 3 parts is skipped. If **both** name and password match: role `admin` -> `new Admin(name)`, otherwise `new Student(name)`. No match -> `NULL`.
- Return type is `User*` - the caller (main) never needs to know which child it got. `main.cpp` does `delete u` after logout.


---

## 6. `main.cpp` - Member 2: program start

**Calls `loadBooks` (C), `loginUser` and `menu()` (C++).**

### `main.cpp` lines 1-13

```cpp
// ====================================================================
// FILE    : main.cpp
// OWNER   : Member 2 (program flow) - small file, uses everyone's code
// PURPOSE : starting point. Flow of the whole program:
//             1. loadBooks()  [list.c]  -> books file into the linked list,
//                                          hash table + BST are built
//             2. login screen  [oop.cpp] -> Student or Admin object
//             3. user->menu()            -> the menu of that role (polymorphism)
// ====================================================================
#include <iostream>
#include "oop.h"

using namespace std;
```

- Header explains the flow of the whole program in 3 steps. Includes `oop.h` (which brings `library.h` too).

### `main.cpp` lines 15-20

```cpp
int main() {
    loadBooks();     // C core: read books file into the linked list + build hash/BST

    cout << "=====================================================\n"
         << "  Intelligent Digital Library & Book Recommendation\n"
         << "=====================================================\n";
```

- `main()` is where every C/C++ program starts - and the **only** `main` in the project.
- `loadBooks()` (C core) reads `books.txt` into the linked list and builds the hash table + BST. Then the banner is printed.

### `main.cpp` lines 22-44

```cpp
    int choice;
    do {
        cout << "\n1. Login\n0. Exit\n";
        choice = readInt("Choice: ");
        if (choice == 1) {
            string name = readLine("Username: ");
            string pass = readLine("Password: ");
            User *u = loginUser(name, pass);
            if (u == NULL) {
                cout << "Wrong username or password.\n";
            } else {
                cout << "\nWelcome, " << u->getName() << " (" << u->getRole() << ")\n";
                u->menu();          // polymorphism: Student or Admin menu runs
                delete u;
            }
        } else if (choice != 0) {
            cout << "Invalid choice.\n";
        }
    } while (choice != 0);

    cout << "Thank you. Goodbye!\n";
    return 0;
}
```

- Menu loop: `1. Login / 0. Exit`. Option 1 reads username and password and calls `loginUser`.
  - `NULL` -> "Wrong username or password."
  - otherwise greets the user with `getName()` and `getRole()` and calls **`u->menu()`** - the *virtual* call that runs the Student menu or the Admin menu. When the user logs out, `menu()` returns and `delete u` frees the object created by `loginUser`.
- Any other number -> "Invalid choice."; `0` ends the loop and prints "Goodbye".


---

## How the six files work together (one-minute summary)
1. `main.cpp` starts -> `loadBooks()` fills the **linked list** (`list.c`) and builds the **hash table + BST** (`search.c`).
2. `loginUser()` (`oop.cpp`) returns a **Student** or **Admin** object; `menu()` is chosen by **polymorphism**.
3. Menu actions call the C functions declared in `library.h`; every change is saved to `data/*.txt`.
4. Recommendation (`Recommender`) reads the history from `issues.txt`, scores unread books (genre +2, author +3), and prints the best five.

For who-calls-whom tables, memory ownership and full traces see `Deep_Dive.md`.
