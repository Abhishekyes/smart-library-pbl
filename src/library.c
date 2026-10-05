/* ################ PART 1 : Member 1 - linked list, files, issue/return ################ */
/* ====================================================================
 * FILE    : library.c  (C part of the project)
 * OWNER   : PART 1 = Member 1 (list, files)   |   PART 2 = Member 2 (search, sort)
 * PPT     : "Library Core Engine - built in C using DSA"
 * TOPICS  : 1) LINKED LIST  - every book is a node, all books are chained
 *           2) FILE HANDLING - books / issue history saved in text files
 *           3) Book operations: add, delete, display, issue, return
 * (Members 1 and 2 share this file: PART 1 = Member 1, PART 2 = Member 2.
 *  Users, menus, recommendation are in main.cpp = Member 3.)
 *
 * SECTIONS IN THIS FILE
 *   A. Linked list helpers        (freeList, appendNode)
 *   B. Books file                 (loadBooks, saveBooks)
 *   C. Add / delete a book        (addBook, deleteBook)
 *   D. Display functions
 *   E. Issue history file         (readRecords, writeRecords)
 *   F. Issue / return / history   (issueBook, returnBook, getUserHistory ...)
 * ==================================================================== */
/*
 * ===================== BEGINNER GLOSSARY (read this first) =====================
 * Words that appear again and again in this file. Each is explained ONCE here.
 *
 * 1) POINTER  (the * sign)
 *    A pointer is a variable that stores the ADDRESS (house number) of another
 *    variable in memory, not the value itself.
 *        Book *p;      p can hold the address of one Book
 *        p->title      "go to the Book that p points to and take its title"
 *        (*p).title    exactly the same thing, written the long way
 *    Picture:   p [ address 5000 ] ------> [ Book: id=201, title=... ]
 *
 * 2) NULL
 *    NULL is the special pointer value that means "points to NOTHING".
 *    Always check  if (p == NULL)  before using p. Using a NULL pointer crashes
 *    the program ("segmentation fault").
 *
 * 3) STRUCT
 *    A struct puts several variables under ONE name. Our Book struct (see
 *    library.h) holds id, title, author, genre, available, timesIssued and next.
 *    One Book variable = the whole record of one book.
 *
 * 4) malloc / free  (memory that is asked for while the program is running)
 *    malloc(n)  asks the computer for n bytes. It gives back a pointer to that
 *               memory, or NULL if no memory is left (so always check for NULL).
 *    free(p)    gives the memory back. Every malloc needs exactly ONE free.
 *    sizeof(Book) = how many bytes one Book needs, so
 *    malloc(sizeof(Book)) means "give me room for exactly one Book".
 *    Forget free()          -> "memory leak" (memory is lost until the program ends)
 *    Use p after free(p)    -> "dangling pointer" (garbage or crash)
 *
 * 5) static
 *    A global variable or function marked static is PRIVATE to this file; other
 *    files cannot see it (head, records, cleanText ... are static).
 *    Functions WITHOUT static (loadBooks, addBook ...) are public: they are
 *    listed in library.h so main.cpp can call them.
 *
 * 6) strtok  (string tokenizer = "cut a text into tokens/pieces")
 *    strtok(line, "|") returns the first piece before a '|'.
 *    strtok(NULL, "|") returns the next piece, and so on, until it gives NULL.
 *    It WRITES an end marker '\0' over every '|', so it changes the line itself.
 *
 * 7) fopen modes     FILE *f = fopen(name, mode);
 *    +------+-------------------------------------------------------------+
 *    | "r"  | read only. Returns NULL if the file does not exist.         |
 *    | "w"  | write. Creates the file, or ERASES the old content!         |
 *    | "a"  | append. Adds at the END, old content stays.                 |
 *    +------+-------------------------------------------------------------+
 *    Always fclose(f) when finished, so the data is really written to disk.
 *
 * 8) C STRING = an array of char that ends with '\0' (the end-of-text marker).
 *    "Hobbit" is stored as   H o b b i t \0   (7 bytes, not 6).
 *    strcmp, strcpy, strstr, printf("%s") all look for that '\0' to know where
 *    the text ends. strcmp(a,b) == 0 means "the two texts are equal".
 *    (Never compare texts with ==, that compares addresses, not letters.)
 */
#include <stdio.h>                                    /* standard input/output: printf, fopen, fgets */
#include <stdlib.h>                                   /* memory and number helpers: malloc, free, atoi */
#include <string.h>                                   /* text helpers: strcmp, strcpy, strtok */
#include <ctype.h>                                    /* letter helpers: tolower */
#include "library.h"                                  /* our own header: Book structure and function list */

/*
 * #define MAX_RECORDS 2000 : #define is a find-and-replace done BEFORE compiling.
 * Everywhere we write MAX_RECORDS the compiler sees 2000. Change it in one place
 * and the whole file follows. (Same for NAME_LEN, MAX_BOOKS ... in library.h.)
 */
#define MAX_RECORDS 2000                              /* most issue-history lines we can keep in memory */

/*
 * ------------------------- THE LINKED LIST (core of PART 1) -------------------------
 * Book is a struct that contains a pointer called "next". So every book knows
 * where the next book is. 'head' remembers only the FIRST book.
 *
 *    head --> [301|Wings of Fire] --> [105|OOP with C++] --> [201|Harry..] --> NULL
 *              (node 1)                (node 2)               (node 3)
 *    NULL in the last "next" means "the chain ends here".
 *
 * Why a linked list and not an array?
 *   - an array has a fixed size; a list grows one node at a time with malloc
 *   - deleting a book = re-link ONE pointer (no shifting of other books)
 * Weak point: to find a book you must walk from head, one by one = O(n).
 * (n = number of books). That is why PART 2 adds a hash table and a BST.
 *
 *   Where it works:        small / changing collections, many add + delete.
 *                          Example: 500 library books that are added and removed.
 *   Where it does not:     fast search by position or by key.
 *                          Example: "give me the 400th book" needs 400 steps.
 *
 * "static Book *head = NULL;"
 *    static  -> private to this file     Book *  -> pointer to a Book
 *    = NULL  -> the library starts EMPTY (static variables are zero-filled
 *               anyway, but writing NULL makes the meaning clear)
 */
static Book *head = NULL;   /* head = first book; the whole list hangs from it (NULL = empty) */

/*
 * struct + typedef : "typedef struct { ... } Record;" creates a NEW TYPE NAME
 * Record, so later we can write  Record r;  instead of  struct Record r;
 * A Record is one line of data/issues.txt.
 *    Line   : priya|201|RETURNED
 *    Record : user="priya"   bookId=201   status="RETURNED"
 * status[10] is big enough: "RETURNED" has 8 letters + 1 for '\0' = 9 bytes.
 */
/* one line of data/issues.txt:  user|bookId|ISSUED  (or RETURNED) */
typedef struct {                                      /* a Record = one line of issues.txt */
    char user[NAME_LEN];                              /* name of the user who borrowed the book */
    int  bookId;                                      /* id of the borrowed book */
    char status[10];                                  /* text ISSUED or RETURNED */
} Record;                                             /* end of Record type */

/*
 * records[] is an ARRAY of 2000 Record variables = a copy of issues.txt in memory.
 * Every function that needs it calls readRecords() first, so it is always fresh.
 * If the file has more than 2000 lines the extra lines are ignored (no overflow).
 */
static Record records[MAX_RECORDS];   /* issues.txt lines kept in memory */

/*
 * getHead
 *   WHAT   : returns the pointer to the first book.
 *   WHY    : 'head' is static (private). Other code (the hash table builder, the
 *            sorter, the recommender in main.cpp) must ask through this function.
 *            Such a function is called a "getter".
 *   INPUT  : nothing (void).
 *   OUTPUT : Book * = first book, or NULL when the library is empty.
 *   EXAMPLE: for (Book *b = getHead(); b != NULL; b = b->next) ... visits all books.
 */
Book *getHead(void) { return head; }   /* other files read the list through this */

/*
 * cleanText
 *   WHAT   : copies a text into dest and turns every '|' and newline into a space.
 *   WHY    : '|' separates the fields in our text files. A title like "A|B" would
 *            create an extra piece, the line would have 7 pieces instead of 6,
 *            and splitLine/loadBooks would throw that book away on the next start.
 *   INPUT  : dest = where to copy to (must have room for size characters)
 *            src  = the original text (it is NOT changed: const)
 *            size = size of dest, e.g. TITLE_LEN = 60
 *   OUTPUT : nothing returned (void). The result is written into dest.
 *   EXAMPLE: cleanText(b->title, "A|B\n", 60)   ->   b->title = "A B "
 *   PICTURE: src  = A | B \n \0
 *            dest = A _ B _ \0        (_ = a space)
 *   WHAT BREAKS if a line is removed:
 *     - dest[size-1] = '\0'  : strncpy does NOT add '\0' when src is too long.
 *       Without this line a long title has no end marker and printf prints junk.
 *     - size - 1 (not size)  : keeps one byte free for that '\0'.
 */
/* remove '|' and newline because '|' separates fields in our files */
/* cleanText: copies text safely and removes bad characters */
static void cleanText(char *dest, const char *src, int size) {
    int i;                                            /* i = position (index) inside the text */
    strncpy(dest, src, size - 1);            /* copy at most size-1 letters (no overflow) */
    dest[size - 1] = '\0';                       /* '\0' = end-of-text marker */
    for (i = 0; dest[i] != '\0'; i++)                 /* go through every letter until the end of the text */
        /* if the letter is '|' or newline, change it to a space */
        if (dest[i] == '|' || dest[i] == '\n') dest[i] = ' ';
}                                                     /* end of cleanText */

/*
 * splitLine
 *   WHAT   : cuts one line of a data file into pieces at every '|' character.
 *   WHY    : all our files (books, issues, users) store one record per line with
 *            '|' between the fields, so every loader needs this one helper.
 *   INPUT  : line  = the text to cut (it WILL be changed, see below)
 *            parts = an array of pointers; this function fills it
 *            max   = how many pieces we have room for (stops there)
 *   OUTPUT : the number of pieces found.  The caller compares it with what it
 *            expects (6 for a book, 3 for an issue or a user); a different
 *            number means a blank or damaged line, which is skipped.
 *   EXAMPLE: line = "201|The Hobbit|J R R Tolkien|Fantasy|1|3"   max = 6
 *            returns 6 and
 *              parts[0]="201"  parts[1]="The Hobbit"  parts[2]="J R R Tolkien"
 *              parts[3]="Fantasy"  parts[4]="1"  parts[5]="3"
 *            line = "hello" returns 1, so the caller says "not 6, skip it".
 *   PICTURE (strtok overwrites each '|' with an end marker '\0'):
 *      before : 2 0 1 | T h e   H o b b i t | J R R ...
 *      after  : 2 0 1 \0 T h e   H o b b i t \0 J R R ...
 *                ^                ^
 *           parts[0]         parts[1]       (they point INSIDE line, no copy)
 *   NOTES / COMMON MISTAKES:
 *     - strcspn(line, "\r\n") = position of the first newline (or the end).
 *       We put '\0' there to remove the newline that fgets keeps at the line end.
 *     - parts[] point INTO line. If line is reused or goes away, parts become
 *       wrong. Use the pieces before reading the next line.
 *     - strtok skips empty fields: "5||x" gives only 2 pieces ("5" and "x").
 *     - "n < max" in the loop protects parts[] from overflow. Remove it and a
 *       line with 20 '|' writes outside the array.
 */
/* Cut one line of a data file at every '|' character.
   Example: "201|The Hobbit|Tolkien"  ->  parts[0]="201", parts[1]="The Hobbit", parts[2]="Tolkien"
   Returns how many pieces were found (at most max). Used for books, issues and users files. */
int splitLine(char *line, char *parts[], int max) {   /* splitLine: cuts a line into pieces at each '|' */
    int n = 0;                                        /* n = how many pieces we found so far */
    char *p;                                          /* p = points to the current piece */
    line[strcspn(line, "\r\n")] = '\0';      /* remove the newline at the end of the line */
    /* strtok gives the first piece (text before the first '|') */
    p = strtok(line, "|");
    while (p != NULL && n < max) {                    /* repeat while a piece exists and there is room */
        parts[n] = p;                                 /* save this piece in the result array */
        n++;                                          /* one more piece found */
        p = strtok(NULL, "|");                        /* strtok(NULL,..) gives the next piece */
    }
    return n;                                         /* tell the caller how many pieces were found */
}                                                     /* end of splitLine */

/*
 * countBooks
 *   WHAT   : counts how many nodes the linked list has.
 *   WHY    : we must not go over MAX_BOOKS (500) because other code uses fixed
 *            arrays of that size (see showSorted and the recommender).
 *   INPUT  : nothing.
 *   OUTPUT : number of books (0 when the list is empty).
 *   EXAMPLE: head --> [301] --> [105] --> [201] --> NULL   returns 3
 *   TIME   : O(n) - it walks the whole list every time it is called.
 */
/* how many books are in the linked list right now */
static int countBooks(void) {                         /* countBooks: counts the nodes in the list */
    int n = 0;                                        /* n = running total of books */
    Book *cur;                                        /* cur = the book we are looking at now */
    /* start at the first book and step to the next one; count each */
    for (cur = head; cur != NULL; cur = cur->next) n++;
    return n;                                         /* give back the total */
}                                                     /* end of countBooks */

/*
 * freeList
 *   WHAT   : frees (gives back) the memory of EVERY book node and empties the list.
 *   WHY    : loadBooks() starts by forgetting the old list. If we only set
 *            head = NULL, the old nodes would stay in memory for ever (leak).
 *   INPUT  : nothing.   OUTPUT : nothing (head becomes NULL).
 *   EXAMPLE: head --> [A] --> [B] --> NULL
 *      round 1: cur=[A]  nxt=[B]   free(A)   cur=[B]
 *      round 2: cur=[B]  nxt=NULL  free(B)   cur=NULL  -> loop ends
 *      head = NULL
 *   COMMON MISTAKES:
 *     - writing free(cur) BEFORE nxt = cur->next : then cur->next is read from
 *       memory that was already given back (undefined behaviour / crash).
 *     - forgetting head = NULL : head would keep pointing at freed memory
 *       (dangling pointer).
 */
/* ---------------- A + B. linked list helpers and books file ---------------- */
static void freeList(void) {                          /* freeList: gives back the memory of every book */
    Book *cur = head, *nxt;                    /* cur = node to free, nxt = remembered next node */
    while (cur != NULL) {                             /* repeat until we pass the last book */
        nxt = cur->next;                         /* remember next BEFORE freeing */
        free(cur);                               /* give this node's memory back */
        cur = nxt;                                    /* move on to the remembered next node */
    }
    head = NULL;                                 /* list is empty now */
}                                                     /* end of freeList */

/*
 * appendNode
 *   WHAT   : adds one ready-made node at the END of the linked list.
 *   WHY    : books are kept in the order they were added (same as the file).
 *   INPUT  : node = pointer to a Book that was already created with malloc.
 *   OUTPUT : nothing (the list is changed).
 *   EXAMPLE: list is  head --> [301] --> [105] --> NULL, add [999]
 *      step 1: node->next = NULL                    [999] --> NULL
 *      step 2: walk until cur->next == NULL         cur = [105]
 *      step 3: cur->next = node
 *      result: head --> [301] --> [105] --> [999] --> NULL
 *   TIME   : O(n) because we must walk to the last node.
 *   WHAT BREAKS if a line is removed:
 *     - node->next = NULL missing : malloc leaves garbage in "next", the walk
 *       later follows a garbage address and crashes.
 *     - the "if (head == NULL)" line missing : on an empty list cur is NULL and
 *       cur->next crashes.
 */
static void appendNode(Book *node) {                  /* appendNode: adds one book to the end of the list */
    Book *cur;                                        /* cur = helper pointer used to walk the list */
    node->next = NULL;                         /* new node will be the last one */
    if (head == NULL) { head = node; return; }   /* empty list: node becomes first */
    cur = head;                                       /* start walking from the first book */
    while (cur->next != NULL) cur = cur->next;   /* walk to the last node */
    cur->next = node;                          /* hook the new node at the end */
}                                                     /* end of appendNode */

/*
 * loadBooks
 *   WHAT   : reads data/books.txt and builds the linked list, one node per line.
 *   WHY    : the program must remember the books between runs; the file is the
 *            permanent storage, the linked list is the working copy in memory.
 *   INPUT  : nothing (the file name is BOOKS_FILE from library.h).
 *   OUTPUT : nothing returned. Result: the list is filled, and buildIndexes()
 *            has filled the hash table and the BST too.
 *   EXAMPLE: file line  "203|The Hobbit|J R R Tolkien|Fantasy|1|2"
 *            -> splitLine gives 6 pieces
 *            -> node id=203, title="The Hobbit", author="J R R Tolkien",
 *               genre="Fantasy", available=1, timesIssued=2
 *            -> appendNode puts it at the end of the list.
 *   FLOW   : freeList -> fopen("r") -> [fgets -> splitLine -> malloc -> fill
 *            fields -> appendNode] repeated -> fclose -> buildIndexes
 *   NOTES  :
 *     - fgets(line, size, f) reads ONE line (at most size-1 characters) and
 *       returns NULL at the end of the file.
 *     - atoi("203") = 203 : converts text to a number (atoi = "ASCII to integer").
 *     - (Book *)malloc(...) : malloc returns a plain address (void *); the cast
 *       says "treat it as a Book pointer". The cast is optional in C.
 *     - "continue" skips the rest of the loop body; "break" leaves the loop.
 *   COMMON MISTAKES:
 *     - not checking fopen() for NULL : a missing file would crash on fgets.
 *     - not checking malloc() for NULL : out of memory would crash on b->id.
 *     - forgetting fclose : the file stays open (resource leak).
 *     - forgetting buildIndexes : the list is full but findById / findByTitle
 *       find nothing, because the hash table and the BST are still empty.
 */
/* read data/books.txt, create one linked-list node per line */
void loadBooks(void) {                                /* loadBooks: reads books.txt into the linked list */
    FILE *f = fopen(BOOKS_FILE, "r");          /* "r" = open for reading; NULL = file missing */
    char line[300];                              /* one line of the file */
    char *parts[6];                              /* the 6 pieces of a book line */
    Book *b;                                          /* b = pointer to the new book node */
    freeList();                                    /* forget old list before loading */
    if (f == NULL) {                                  /* if the file could not be opened */
        /* tell the user that the file is missing */
        printf("(No %s found - starting with an empty library)\n", BOOKS_FILE);
        buildIndexes();                               /* still create (empty) search structures */
        return;                                       /* leave the function - nothing to read */
    }
    while (fgets(line, sizeof(line), f) != NULL) {    /* read one line at a time until the file ends */
        if (splitLine(line, parts, 6) != 6) continue;    /* skip blank / bad line */
        if (countBooks() >= MAX_BOOKS) break;            /* library is full */
        b = (Book *)malloc(sizeof(Book));                /* one new node */
        if (b == NULL) break;                         /* no memory left: stop reading */
        b->id = atoi(parts[0]);                          /* atoi: text -> number */
        cleanText(b->title,  parts[1], TITLE_LEN);    /* copy the title (and remove '|') */
        cleanText(b->author, parts[2], AUTHOR_LEN);   /* copy the author */
        cleanText(b->genre,  parts[3], GENRE_LEN);    /* copy the genre */
        b->available   = atoi(parts[4]);              /* 1 = on the shelf, 0 = issued */
        b->timesIssued = atoi(parts[5]);              /* how many times it was issued before */
        appendNode(b);                                /* add this node at the end of the list */
    }
    fclose(f);                                         /* always close a file */
    buildIndexes();
}                                                     /* end of loadBooks */

/*
 * saveBooks
 *   WHAT   : writes the whole linked list into data/books.txt, one line per book.
 *   WHY    : after add / delete / issue / return the file must match memory,
 *            otherwise the change is lost when the program closes.
 *   INPUT  : nothing.   OUTPUT : nothing (the file is rewritten).
 *   EXAMPLE: node id=203 gives the line
 *            203|The Hobbit|J R R Tolkien|Fantasy|1|2
 *   NOTES  : fprintf is printf for a FILE. %d prints a number, %s prints a text,
 *            \n ends the line. "w" erases the old file first, so we write
 *            EVERYTHING again (simple and always correct for 500 books).
 *   COMMON MISTAKE: not calling fclose : buffered text may never reach the disk.
 */
/* write the whole linked list back to data/books.txt */
void saveBooks(void) {                                /* saveBooks: writes the whole list into books.txt */
    FILE *f = fopen(BOOKS_FILE, "w");          /* "w" = overwrite the file */
    Book *cur;                                        /* cur = the book being written */
    /* if the file cannot be opened, show an error and stop */
    if (f == NULL) { printf("Error: cannot write %s\n", BOOKS_FILE); return; }
    for (cur = head; cur != NULL; cur = cur->next)   /* visit every book */
        /* write one line per book, fields separated by '|' */
        fprintf(f, "%d|%s|%s|%s|%d|%d\n", cur->id, cur->title, cur->author,
                cur->genre, cur->available, cur->timesIssued);  /* (continued) the rest of the fields */
    fclose(f);                                        /* close the file so everything is saved */
}                                                     /* end of saveBooks */

/* ---------------- C. add / delete a book (linked list operations) ---------------- */
/*
 * addBook
 *   WHAT   : creates a new book, puts it at the end of the list, saves the file.
 *   WHY    : this is the admin feature "Add a book".
 *   INPUT  : id, title, author, genre of the new book.
 *   OUTPUT : 1 = added,  0 = this id already exists (or no memory),
 *            -1 = library is full (MAX_BOOKS).
 *   EXAMPLE: addBook(999, "Clean Code", "Robert Martin", "Programming")  -> 1
 *            the same call again -> 0   (findById(999) already finds it)
 *   ORDER OF WORK: check duplicate -> check full -> malloc -> fill fields ->
 *                  appendNode -> saveBooks -> buildIndexes
 *   WHAT BREAKS if a line is removed:
 *     - buildIndexes() missing : the book is in the list and in the file, but
 *       searchById(999) says "not found" because the hash table and BST do not
 *       know the new node yet.
 *     - saveBooks() missing : the book disappears after the program is closed.
 *     - the findById check missing : two books with the same id; the hash table
 *       would return only one of them.
 */
/* add a new book at the END of the linked list, then save the file.
   returns 1 = added, 0 = a book with this id already exists, -1 = library is full */
/* addBook: adds a new book to the library */
int addBook(int id, const char *title, const char *author, const char *genre) {
    Book *b;                                          /* b = pointer to the new book node */
    if (findById(id) != NULL) return 0;          /* duplicate id */
    if (countBooks() >= MAX_BOOKS) return -1;    /* library is full */
    b = (Book *)malloc(sizeof(Book));                 /* ask the system for memory for one Book */
    if (b == NULL) return 0;                          /* no memory available: cannot add */
    b->id = id;                                       /* store the id */
    cleanText(b->title, title, TITLE_LEN);            /* store the title */
    cleanText(b->author, author, AUTHOR_LEN);         /* store the author */
    cleanText(b->genre, genre, GENRE_LEN);            /* store the genre */
    b->available = 1;                                 /* a new book starts as available */
    b->timesIssued = 0;                               /* nobody has issued it yet */
    appendNode(b);                                    /* put it at the end of the list */
    saveBooks();                                      /* save the list to the file */
    /* rebuild hash table and tree so they know the new book */
    buildIndexes();
    return 1;                                         /* 1 = book added */
}                                                     /* end of addBook */

/*
 * deleteBook
 *   WHAT   : removes one book from the linked list and frees its memory.
 *   WHY    : admin feature "Delete a book".
 *   INPUT  : id of the book to delete.
 *   OUTPUT : 1 = deleted,  0 = id not found,
 *            -1 = the book is issued right now (not allowed to delete).
 *   IDEA   : keep TWO pointers while walking: cur (the book to delete) and prev
 *            (the book just before it). Then prev can jump over cur.
 *   PICTURE A - delete a book in the MIDDLE (id 105):
 *      before : head --> [301] --> [105] --> [201] --> NULL
 *                         prev      cur
 *      step   : prev->next = cur->next
 *                         [301] ----------> [201]      [105] is cut out
 *      free(cur) : the memory of [105] goes back to the system
 *      after  : head --> [301] --> [201] --> NULL
 *   PICTURE B - delete the FIRST book (id 301): prev is still NULL
 *      before : head --> [301] --> [105] --> NULL
 *      step   : head = cur->next
 *      after  : head --> [105] --> NULL
 *   EXAMPLE: deleteBook(105) when 105 is on loan (available == 0) returns -1
 *            and nothing is changed.
 *   WHAT BREAKS if a line is removed (the most important place in this file):
 *     - buildIndexes() missing : the hash table and the BST still hold a pointer
 *       to the freed node (dangling pointer). The next search touches freed
 *       memory = wrong data or a crash.
 *     - free(cur) missing : memory leak.
 *     - using cur after free(cur) : crash. That is why free comes AFTER re-linking.
 *     - the "prev == NULL" branch missing : deleting the first book would use
 *       prev->next with prev = NULL = crash.
 */
/* delete a node: link the previous node to the next one, then free() it.
   returns 1 = deleted, 0 = id not found, -1 = book is issued (not allowed) */
int deleteBook(int id) {                              /* deleteBook: removes a book from the library */
    /* cur = book being checked, prev = the book just before it */
    Book *cur = head, *prev = NULL;
    while (cur != NULL && cur->id != id) {            /* walk on until the id is found or the list ends */
        prev = cur;                                   /* remember the current book as the previous one */
        cur = cur->next;                              /* move to the next book */
    }
    if (cur == NULL) return 0;                        /* id not found: return 0 */
    if (cur->available == 0) return -1;          /* cannot delete an issued book */
    if (prev == NULL) head = cur->next;             /* deleting the first book */
    else prev->next = cur->next;               /* previous node skips the deleted one */
    free(cur);                                   /* release memory */
    saveBooks();                                      /* save the shorter list to the file */
    buildIndexes();                                   /* rebuild hash table and tree without this book */
    return 1;                                         /* 1 = book deleted */
}                                                     /* end of deleteBook */

/* ---------------- D. display ---------------- */
/*
 * printBookHeader
 *   WHAT   : prints the title row of the book table and a line below it.
 *   WHY    : every list of books (search, sorted view, recommendations) looks the
 *            same, so the heading is written once here.
 *   INPUT  : nothing.  OUTPUT : nothing (text on the screen).
 *   NOTES  : %-5s = print a text, LEFT aligned, in a column 5 characters wide.
 *            (without the minus sign it would be right aligned)
 *   EXAMPLE OUTPUT:
 *      ID    Title                                    Author               ...
 *      -------------------------------------------------------------------
 */
/* printBookHeader: prints the column titles of the table */
void printBookHeader(void) {
    printf("%-5s %-40s %-20s %-12s %-9s %s\n",        /* print the titles in columns of fixed width */
           "ID", "Title", "Author", "Genre", "Status", "Issued");  /* (continued) the title words */
    printf("-----------------------------------------------------------------------------------------------\n");
}                                                     /* end of printBookHeader */

/*
 * displayBook
 *   WHAT   : prints ONE book as one row of the table.
 *   WHY    : used by search, sorted view and the walk of the BST.
 *   INPUT  : b = pointer to the book. "const Book *" is a promise that this
 *            function will only READ the book, never change it.
 *   OUTPUT : nothing (text on the screen).
 *   NOTES  : %-40.40s = column 40 wide AND cut the text after 40 letters, so a
 *            very long title cannot push the other columns away.
 *            (b->available ? "Available" : "Issued") is a short if/else:
 *            if available is not 0 use the first text, otherwise the second.
 *   EXAMPLE OUTPUT:
 *      203   The Hobbit        J R R Tolkien    Fantasy      Available 2
 */
void displayBook(const Book *b) {                     /* displayBook: prints one book as a table row */
    printf("%-5d %-40.40s %-20.20s %-12.12s %-9s %d\n",  /* print id, title, author and genre in neat columns */
           b->id, b->title, b->author, b->genre,      /* (continued) the book's id, title, author, genre */
           /* (continued) Available/Issued word and times issued */
           b->available ? "Available" : "Issued", b->timesIssued);
}                                                     /* end of displayBook */

/* ---------------- E + F. issue history file and issue / return ----------------
   data/issues.txt keeps one line per borrow:  user|bookId|ISSUED or RETURNED.
   This same file is the "reading history" used by the recommendation engine. */
/*
 * readRecords
 *   WHAT   : loads data/issues.txt into the records[] array.
 *   WHY    : returnBook, getUserHistory, showUserHistory and displayIssuedBooks all
 *            need to see the borrow history; they call this first.
 *   INPUT  : nothing.
 *   OUTPUT : how many records were read (0 if the file does not exist yet).
 *   EXAMPLE: file lines "priya|201|RETURNED" and "priya|102|ISSUED"
 *            records[0] = {"priya", 201, "RETURNED"}
 *            records[1] = {"priya", 102, "ISSUED"}      returns 2
 *   NOTES  : "n < MAX_RECORDS" is tested FIRST so we never write past the end
 *            of the array. A line that does not have 3 pieces is skipped.
 */
static int readRecords(void) {                        /* readRecords: loads issues.txt into the records array */
    FILE *f = fopen(ISSUES_FILE, "r");                /* open the issues file for reading */
    char line[200];                                   /* line = one line of the file */
    char *parts[3];                                   /* parts = the 3 pieces of a line */
    int n = 0;                                        /* n = number of records read so far */
    if (f == NULL) return 0;                          /* no file yet: zero records */
    /* read lines until the array is full or the file ends */
    while (n < MAX_RECORDS && fgets(line, sizeof(line), f) != NULL) {
        if (splitLine(line, parts, 3) != 3) continue;    /* skip blank / bad line */
        cleanText(records[n].user, parts[0], NAME_LEN);  /* copy the user name */
        records[n].bookId = atoi(parts[1]);           /* book id: text -> number */
        cleanText(records[n].status, parts[2], 10);   /* copy the status word */
        n++;                                          /* one more record stored */
    }
    fclose(f);                                        /* close the file */
    return n;                                         /* tell the caller how many records were read */
}                                                     /* end of readRecords */

/*
 * writeRecords
 *   WHAT   : writes the first n records back into data/issues.txt.
 *   WHY    : a text file cannot be edited in the middle, so to change one line
 *            (ISSUED -> RETURNED) we change it in memory and write ALL lines again.
 *   INPUT  : n = how many records to write (the value readRecords returned).
 *   OUTPUT : nothing.
 *   WARNING: mode "w" erases the old file first. If n were 0 by mistake, the whole
 *            history would be lost. That is why returnBook passes the same n that
 *            readRecords gave.
 */
/* writeRecords: saves the first n records into issues.txt */
static void writeRecords(int n) {
    /* open the issues file for writing (old content is erased) */
    FILE *f = fopen(ISSUES_FILE, "w");
    int i;                                            /* i = loop counter */
    /* if the file cannot be opened, show an error and stop */
    if (f == NULL) { printf("Error: cannot write %s\n", ISSUES_FILE); return; }
    for (i = 0; i < n; i++)                           /* go through each record */
        /* write it as  user|bookId|status */
        fprintf(f, "%s|%d|%s\n", records[i].user, records[i].bookId, records[i].status);
    fclose(f);                                        /* close the file */
}                                                     /* end of writeRecords */

/*
 * issueBook
 *   WHAT   : lends a book to a user.
 *   WHY    : student feature "Issue a book".
 *   INPUT  : user = name of the borrower, id = book id.
 *   OUTPUT : 1 = success,  0 = no book with this id,
 *            -1 = the book is already on loan.
 *   EXAMPLE: issueBook("priya", 201) with book 201 available=1, timesIssued=3
 *            -> book becomes available=0, timesIssued=4
 *            -> a line  priya|201|ISSUED  is added to issues.txt
 *            -> returns 1.   Calling it again returns -1.
 *   NOTES  : b points to the REAL node of the linked list (found through the hash
 *            table), so b->available = 0 changes the list itself, not a copy.
 *            File mode "a" (append) adds one line at the end.
 *   WHAT BREAKS if a line is removed:
 *     - "a" changed to "w" : every old history line is erased (and the
 *       recommender loses all its data).
 *     - saveBooks() missing : the new available/issued state is lost at exit.
 */
/* give a book to a user: mark it not available and add an ISSUED line.
   returns 1 = ok, 0 = no such book, -1 = already issued to someone */
int issueBook(const char *user, int id) {             /* issueBook: lends a book to a user */
    Book *b = findById(id);                           /* b = the book with this id (NULL if none) */
    FILE *f;                                          /* f = the issues file */
    if (b == NULL) return 0;                          /* no such book: return 0 */
    if (b->available == 0) return -1;                 /* book is already on loan: return -1 */
    b->available = 0;                            /* now on loan */
    b->timesIssued++;                         /* popularity counter */
    f = fopen(ISSUES_FILE, "a");                  /* "a" = append: add a line at the end */
    if (f != NULL) {                                  /* if the file opened correctly */
        fprintf(f, "%s|%d|ISSUED\n", user, id);       /* write the line  user|id|ISSUED */
        fclose(f);                                    /* close the file */
    }
    saveBooks();                                      /* save the new available/issued state */
    return 1;                                         /* 1 = success */
}                                                     /* end of issueBook */

/*
 * returnBook
 *   WHAT   : takes a book back from a user.
 *   WHY    : student feature "Return a book".
 *   INPUT  : user = name of the borrower, id = book id.
 *   OUTPUT : 1 = success,  0 = this user has no ISSUED record for this id.
 *   EXAMPLE: returnBook("priya", 102)
 *      records before : priya|102|ISSUED
 *      records after  : priya|102|RETURNED   (writeRecords saves it)
 *      book 102 : available = 1 again.     Returns 1.
 *   WHY THREE CHECKS: user AND bookId AND status "ISSUED". So rahul cannot return
 *            priya's book, and a book already RETURNED cannot be returned twice.
 *   NOTES  : strcmp(a,b) == 0 means "equal". strcpy(dest, src) copies a text;
 *            "RETURNED" (8 letters + '\0') fits in status[10].
 */
/* take a book back: change that user's ISSUED line to RETURNED and mark the
   book available again. returns 1 = ok, 0 = this user has not issued it */
int returnBook(const char *user, int id) {            /* returnBook: takes a book back from a user */
    int n = readRecords(), i;                         /* n = number of records, i = loop counter */
    Book *b;                                          /* b = the book being returned */
    for (i = 0; i < n; i++) {                         /* look at every record */
        /* does the record belong to this user and this book... */
        if (strcmp(records[i].user, user) == 0 && records[i].bookId == id &&
            strcmp(records[i].status, "ISSUED") == 0) {  /* ...and is it still ISSUED? */
            strcpy(records[i].status, "RETURNED");   /* ISSUED -> RETURNED */
            writeRecords(n);                          /* save all records back to the file */
            b = findById(id);                         /* find the book in the library */
            if (b != NULL) b->available = 1;          /* back on the shelf */
            saveBooks();                              /* save the books file */
            return 1;                                 /* 1 = success */
        }
    }
    return 0;                                         /* no matching ISSUED record: return 0 */
}                                                     /* end of returnBook */

/*
 * getUserHistory
 *   WHAT   : collects the ids of all books a user has ever borrowed.
 *   WHY    : the Recommender in main.cpp learns the user's taste from this list.
 *   INPUT  : user = name, ids[] = an array that THIS function fills,
 *            max = size of ids[] (so it cannot overflow).
 *   OUTPUT : how many ids were stored.
 *   EXAMPLE: issues.txt has priya|201|RETURNED, priya|203|RETURNED, priya|102|ISSUED
 *            getUserHistory("priya", ids, 200) -> ids = {201, 203, 102}, returns 3
 *   NOTES  : an array parameter is passed as a pointer, so the function writes
 *            into the CALLER'S array (this is how a C function returns many values).
 *            ids[count++] = x   means: store x, THEN add 1 to count.
 *            RETURNED and ISSUED records both count as "has read".
 */
/* ids of all books this user has ever borrowed (used by the recommender) */
/* getUserHistory: collects the book ids a user has borrowed */
int getUserHistory(const char *user, int ids[], int max) {
    int n = readRecords(), i, count = 0;              /* n = records, i = loop counter, count = ids stored */
    /* go on while records remain and there is room in ids[] */
    for (i = 0; i < n && count < max; i++)
        /* if the record is this user's, store its book id */
        if (strcmp(records[i].user, user) == 0) ids[count++] = records[i].bookId;
    return count;                                     /* tell the caller how many ids were stored */
}                                                     /* end of getUserHistory */

/*
 * showUserHistory
 *   WHAT   : prints a table of everything one user has borrowed.
 *   WHY    : student menu option 6 "My reading history".
 *   INPUT  : user = name.   OUTPUT : nothing (text on the screen).
 *   EXAMPLE OUTPUT for priya:
 *      ID    Title                                    Status
 *      201   Harry Potter and the Sorcerer's Stone    RETURNED
 *      102   Data Structures Using C                  ISSUED
 *   NOTES  : "found" is a flag: the heading is printed only once, before the
 *            first row. If it stays 0 we print "No reading history yet."
 *            findById may return NULL for a deleted book; the "b ? ... : ..."
 *            test then prints "(book removed)" instead of crashing.
 */
/* showUserHistory: prints everything a user has borrowed */
void showUserHistory(const char *user) {
    /* n = records, i = loop counter, found = did we print a row yet */
    int n = readRecords(), i, found = 0;
    Book *b;                                          /* b = the book of a record */
    for (i = 0; i < n; i++) {                         /* go through each record */
        if (strcmp(records[i].user, user) != 0) continue;  /* skip records of other users */
        if (!found) {                                 /* first matching record: print the table heading first */
            printf("%-5s %-40s %-10s\n", "ID", "Title", "Status");  /* column titles */
            printf("----------------------------------------------------\n");
            found = 1;                                /* remember that we have printed something */
        }
        b = findById(records[i].bookId);              /* look up the book to get its title */
        printf("%-5d %-40.40s %-10s\n", records[i].bookId,  /* print id, title and status in columns */
               /* (continued) title (or a note if the book was deleted) and status */
               b ? b->title : "(book removed)", records[i].status);
    }
    if (!found) printf("No reading history yet.\n");  /* nothing was printed: tell the user */
}                                                     /* end of showUserHistory */

/*
 * displayIssuedBooks
 *   WHAT   : prints which user currently holds which book.
 *   WHY    : admin menu option 6 "View currently issued books".
 *   INPUT  : nothing.  OUTPUT : nothing (text on the screen).
 *   EXAMPLE OUTPUT (sample data):
 *      Issued to    ID    Title
 *      priya        102   Data Structures Using C
 *      rahul        105   Object Oriented Programming with C++
 *   NOTES  : only records whose status is "ISSUED" are shown; RETURNED lines
 *            are skipped with continue.
 */
/* admin report: who currently holds which book */
/* displayIssuedBooks: shows which user holds which book */
void displayIssuedBooks(void) {
    /* n = records, i = loop counter, found = did we print a row yet */
    int n = readRecords(), i, found = 0;
    Book *b;                                          /* b = the book of a record */
    for (i = 0; i < n; i++) {                         /* go through each record */
        if (strcmp(records[i].status, "ISSUED") != 0) continue;  /* skip books that are already returned */
        if (!found) {                                 /* first row: print the heading first */
            printf("%-12s %-5s %-40s\n", "Issued to", "ID", "Title");  /* column titles */
            printf("-----------------------------------------------------\n");
            found = 1;                                /* remember that we have printed something */
        }
        b = findById(records[i].bookId);              /* look up the book to get its title */
        /* print user, id and title in columns */
        printf("%-12s %-5d %-40.40s\n", records[i].user, records[i].bookId,
               /* (continued) title (or a note if the book was deleted) */
               b ? b->title : "(book removed)");
    }
    if (!found) printf("No books are currently issued.\n");  /* nothing issued: tell the user */
}                                                     /* end of displayIssuedBooks */


/* ################ PART 2 : Member 2 - hashing, BST, sorting ################ */
/* ====================================================================
 * PART 2 of library.c
 * OWNER   : Member 2 (Search & Algorithms)
 * PPT     : "Fast title search via Binary Search Tree",
 *           "Instant lookup by unique ID via hashing",
 *           "Sorting by title, author, or popularity"
 * TOPICS  : 1) HASHING (chaining)          -> find a book by its ID
 *           2) BINARY SEARCH TREE (BST)    -> find a book by its title
 *           3) SORTING (bubble sort)       -> by title / author / popularity
 * NOT HERE: linked list & files -> PART 1 above (Member 1)
 *           menus, recommendation -> main.cpp (Member 3)
 *
 * HOW IT FITS: the hash table and the BST only store POINTERS to the
 * linked-list nodes created in PART 1, so no data is copied. They are
 * rebuilt (buildIndexes) whenever a book is added, deleted or loaded.
 *
 * SECTIONS IN THIS FILE
 *   A. small helpers (compareText, containsText)
 *   B. Hashing        (hashInsert, findById)
 *   C. BST            (bstInsert, findByTitle, searchTitleContains)
 *   D. buildIndexes   (fills both structures)
 *   E. Sorting        (showSorted)
 * ==================================================================== */

/* ---------------- A. helpers ---------------- */
/*
 * Small helpers used by search and sort. They compare texts WITHOUT caring about
 * big or small letters, so "harry" finds "Harry Potter". #define BUF 200 is the
 * size of the temporary lower-case copies (enough for every title).
 */
#define BUF 200      /* big enough for any title or search word we compare */

/*
 * toLower
 *   WHAT   : copies src into dest, turning every letter into a small letter.
 *   WHY    : so that "HARRY", "Harry" and "harry" look the same when we compare.
 *   INPUT  : src = original text (not changed), dest = buffer of size BUF.
 *   OUTPUT : nothing; the lower-case text is in dest.
 *   EXAMPLE: toLower("The HoBBit", dest)   ->   dest = "the hobbit"
 *   NOTES  : (unsigned char) is needed because tolower() must not receive a
 *            negative number; (char) turns the answer back to a char.
 *            "i < BUF - 1" keeps one byte for '\0', so a very long text is cut
 *            instead of overflowing the buffer. Without the last line
 *            (dest[i] = '\0') dest would have no end marker.
 */
/* copy src into dest in lower case (so "HARRY" and "harry" look the same) */
static void toLower(const char *src, char *dest) {    /* toLower: copies src into dest in small letters */
    int i;                                            /* i = position inside the text */
    /* copy letter by letter until the end (or the buffer is full) */
    for (i = 0; src[i] != '\0' && i < BUF - 1; i++)
        dest[i] = (char)tolower((unsigned char)src[i]);  /* turn one letter into its small-letter form */
    dest[i] = '\0';                                   /* close dest with the end-of-text marker */
}                                                     /* end of toLower */

/*
 * compareText
 *   WHAT   : compares two texts ignoring big/small letters.
 *   WHY    : the BST and the sorting need a rule "which title comes first".
 *   INPUT  : a, b = the two texts.
 *   OUTPUT : negative = a comes first, 0 = same, positive = b comes first
 *            (same meaning as strcmp).
 *   EXAMPLE: compareText("cosmos", "Wings of Fire")  -> negative ('c' is before 'w')
 *            compareText("THE HOBBIT", "the hobbit")  -> 0
 */
/* compare two texts ignoring upper/lower case. Result like strcmp:
   negative = a comes first, 0 = same, positive = b comes first */
/* compareText: compares two texts, ignoring big/small letters */
static int compareText(const char *a, const char *b) {
    char x[BUF], y[BUF];                              /* x and y = small-letter copies of the two texts */
    toLower(a, x);                                    /* make a lower-case copy of a */
    toLower(b, y);                                    /* make a lower-case copy of b */
    /* strcmp compares the two copies (negative / 0 / positive) */
    return strcmp(x, y);
}                                                     /* end of compareText */

/*
 * containsText
 *   WHAT   : checks whether a word appears anywhere inside a text (any case).
 *   WHY    : partial title search, e.g. "hob" should find "The Hobbit".
 *   INPUT  : text = the title, word = what the user typed.
 *   OUTPUT : 1 = found, 0 = not found.
 *   EXAMPLE: containsText("The Hobbit", "HOB") -> 1 ;  containsText("Cosmos","hob") -> 0
 *   NOTES  : strstr(x, y) returns a pointer to the place where y starts inside x,
 *            or NULL when it is not there. "!= NULL" turns that into 1 or 0.
 */
/* does "text" contain "word" anywhere (ignoring case)? 1 = yes, 0 = no */
/* containsText: is 'word' found inside 'text'? */
static int containsText(const char *text, const char *word) {
    char x[BUF], y[BUF];                              /* x and y = small-letter copies */
    toLower(text, x);                                 /* lower-case copy of the text */
    toLower(word, y);                                 /* lower-case copy of the word */
    return strstr(x, y) != NULL;                      /* strstr finds y inside x; true if found, false if not */
}                                                     /* end of containsText */

/*
 * ------------------------------- HASHING (idea) -------------------------------
 * GOAL   : find a book by its id almost instantly, without walking the list.
 * IDEA   : an array of 101 "slots". A hash function turns an id into a slot:
 *              slot = id % 101        (% = remainder after dividing)
 *          Examples:  105 % 101 = 4     201 % 101 = 100     202 % 101 = 0
 *          To find id 105 we go straight to slot 4. No walking.
 * COLLISION : two different ids may give the same slot. In our data
 *          101 % 101 = 0,  202 % 101 = 0,  303 % 101 = 0   -> all want slot 0.
 *          105 % 101 = 4 and 206 % 101 = 4.
 * CHAINING  : each slot keeps a small linked list of the books that share it.
 *          table[0]   --> [202] --> [303] --> [101] --> NULL
 *          table[4]   --> [206] --> [105] --> NULL
 *          table[100] --> [403] --> [201] --> NULL
 *          table[7]   --> NULL   (empty slot)
 *          (new nodes are put at the FRONT of a chain, so the last one added
 *           comes first)
 * WHY 101 : a prime number spreads the ids more evenly over the slots.
 * TIME    : average O(1) - one jump + a very short chain.
 *           worst case O(n) - if every id fell into the same slot, we would
 *           walk one long chain (just like a linked list).
 * MEMORY  : the chain nodes store POINTERS to the books, not copies.
 *   Where it works:      exact lookup by a unique key.  Example: "show book 202".
 *   Where it does not:   ranges or order.  Example: "ids from 200 to 250" or
 *                        "titles starting with H" - the hash table has no order.
 */
/* ================= B. HASHING (by book ID) =================
   hash function: id % 101 gives the slot number; books that land in the
   same slot are chained in a small linked list (chaining). */
#define HASH_SIZE 101                                 /* number of slots in the hash table */

/*
 * HNode : one node of a hash chain. It holds a pointer to the Book and a pointer
 * to the next HNode. "struct HNode *next" is written with the long name inside
 * because the short name HNode does not exist yet while the struct is being defined.
 */
typedef struct HNode {                                /* a node of a hash chain */
    Book *book;                                       /* the book this node points to */
    struct HNode *next;      /* chaining: books with the same hash value */
} HNode;                                              /* end of HNode type */

/*
 * table : an array of 101 pointers, one per slot. Being static, every slot starts
 * as NULL = "this slot is empty".
 */
/* the hash table: an array of 101 chains (one per slot) */
static HNode *table[HASH_SIZE];

/*
 * hashId
 *   WHAT   : the hash function; turns an id into a slot number 0..100.
 *   INPUT  : id.   OUTPUT : id % 101.
 *   EXAMPLE: hashId(105) = 4,  hashId(201) = 100,  hashId(202) = 0
 */
/* hashId: turns an id into a slot number (remainder after dividing by 101) */
static int hashId(int id) { return id % HASH_SIZE; }

/*
 * hashInsert
 *   WHAT   : adds one book to the hash table (at the front of its slot's chain).
 *   WHY    : so findById can find the book fast.
 *   INPUT  : b = pointer to the book (it is not copied).   OUTPUT : nothing.
 *   EXAMPLE: insert book 105 (slot 4) when table[4] --> [206] --> NULL
 *      new node n --> book 105
 *      n->next = table[4]      n --> [206] --> NULL
 *      table[4] = n            table[4] --> [105] --> [206] --> NULL
 *   WHAT BREAKS if a line is removed:
 *     - the "n->next = table[h]" line : the old chain is lost (cut off) and those
 *       books can no longer be found.
 *     - the NULL check after malloc : out of memory would crash on n->book.
 */
static void hashInsert(Book *b) {                     /* hashInsert: adds one book to the hash table */
    int h = hashId(b->id);                          /* slot number = id % 101 */
    HNode *n = (HNode *)malloc(sizeof(HNode));      /* new chain node */
    if (n == NULL) return;                            /* no memory: do not add */
    n->book = b;                                      /* the node points to the book (no copy) */
    n->next = table[h];                             /* put in FRONT of the slot's chain */
    table[h] = n;                                     /* the slot now starts with this new node */
}                                                     /* end of hashInsert */

/*
 * hashClear
 *   WHAT   : frees every chain node and makes every slot empty again.
 *   WHY    : buildIndexes starts from zero each time. Note: only the HNodes are
 *            freed here, NOT the books (the linked list still owns the books).
 *   INPUT/OUTPUT : nothing.
 *   NOTE   : the next pointer is saved in nxt BEFORE free(cur), same rule as freeList.
 */
static void hashClear(void) {                         /* hashClear: frees all hash nodes */
    int i;                                            /* i = slot number */
    HNode *cur, *nxt;                                 /* cur = node to free, nxt = next node */
    for (i = 0; i < HASH_SIZE; i++) {                 /* go through every slot */
        cur = table[i];                               /* start at the first node of this slot */
        /* free each node of the chain, remembering the next one first */
        while (cur != NULL) { nxt = cur->next; free(cur); cur = nxt; }
        table[i] = NULL;                              /* this slot is empty now */
    }
}                                                     /* end of hashClear */

/*
 * findById
 *   WHAT   : finds a book by its id using the hash table.
 *   WHY    : used by addBook (duplicate check), issueBook, the recommender, ...
 *   INPUT  : id.
 *   OUTPUT : pointer to the Book, or NULL if there is no such id.
 *   EXAMPLE: findById(202): slot = 202 % 101 = 0
 *      table[0] --> [202] --> [303] --> [101] --> NULL
 *      first node has id 202 -> found after 1 step.
 *      findById(101): checks 202 (no), 303 (no), 101 (yes) -> found after 3 steps.
 *      findById(999): slot 999 % 101 = 90, chain empty -> returns NULL.
 *   TIME   : average O(1).
 *   NOTE   : the caller MUST check the answer for NULL before using it.
 */
/* findById: finds a book by its id using the hash table */
Book *findById(int id) {
    HNode *cur = table[hashId(id)];                 /* jump straight to the right slot */
    while (cur != NULL) {                             /* walk along the chain of this slot */
        if (cur->book->id == id) return cur->book;    /* id matches: give back that book */
        cur = cur->next;                              /* otherwise look at the next node */
    }
    return NULL;                                      /* id not in the table: return NULL (nothing) */
}                                                     /* end of findById */

/*
 * ------------------------- BINARY SEARCH TREE (idea) -------------------------
 * GOAL   : find a book by TITLE fast, and list titles in A-Z order.
 * RULE   : for every node, titles that are smaller (A-Z) go to the LEFT,
 *          titles that are bigger go to the RIGHT.
 * EXAMPLE: insert (from our file order) Wings of Fire, Object Oriented
 *          Programming..., Harry Potter and the Sorcerer's Stone, Murder on
 *          the Orient Express   (compared in small letters)
 *
 *                     [Wings of Fire]
 *                     /
 *            [Object Oriented...]
 *                   /
 *         [Harry Potter...]
 *                   \
 *               [Murder on the Orient Express]
 *
 * SEARCH : to find "murder on ...": wings? smaller -> left; object? smaller ->
 *          left; harry? bigger -> right; found. Each step throws away a whole
 *          half of the remaining books.
 * TIME   : average O(log n)  (500 books need about 9 steps).
 *          worst case O(n)   - if titles come in already sorted order the tree
 *          becomes one long line, like a linked list.
 *   Where it works:      search by exact or partial title, and A-Z listing.
 *                        Example: "the hobbit" is found in a few steps.
 *   Where it does not:   a very unbalanced tree (sorted input) is slow, and a
 *                        search for the middle of a title needs a full walk
 *                        (that is why searchTitleContains visits every node).
 */
/* ================= C. BINARY SEARCH TREE (by title) =================
   smaller titles go to the left, bigger ones to the right (A-Z order). */
/*
 * TNode : one node of the tree: a pointer to the Book plus two child pointers.
 * left = the sub-tree with smaller titles, right = bigger titles. A node with
 * both children NULL is a "leaf" (the end of a branch).
 */
typedef struct TNode {                                /* a node of the binary search tree */
    Book *book;                                       /* the book this node points to */
    struct TNode *left, *right;                       /* left = smaller titles, right = bigger titles */
} TNode;                                              /* end of TNode type */

/*
 * root : the top node of the tree. NULL means the tree is empty.
 */
static TNode *root = NULL;                            /* root = top node of the tree (NULL = empty tree) */

/*
 * bstInsert
 *   WHAT   : puts one book into the tree and returns the (new) top of the tree.
 *   WHY    : this is how the tree gets built, book by book.
 *   INPUT  : node = top of the (sub-)tree to insert into, b = the book.
 *   OUTPUT : the same node, or a new node if the tree was empty.
 *   RECURSION (a function that calls itself) with a smaller problem each time:
 *      - empty spot (node == NULL) -> create the node here and return it
 *      - new title smaller -> node->left  = bstInsert(node->left, b)
 *      - otherwise         -> node->right = bstInsert(node->right, b)
 *      Storing the answer back (node->left = ...) is what links the new node in.
 *   EXAMPLE: insert "Murder on the Orient Express" into the tree above:
 *      compare with Wings -> left ; with Object -> left ; with Harry -> right;
 *      Harry's right is NULL -> new node created there.
 *   NOTE   : equal titles go to the right (the else branch), so duplicates are
 *            allowed and none is lost.
 *   WHAT BREAKS if the "node->left = ..." assignment is written without the
 *            "node->left =" part: the new node is created but never linked, so
 *            it is lost (memory leak and the book is not searchable).
 */
/* bstInsert: puts a book into the tree and returns the tree */
static TNode *bstInsert(TNode *node, Book *b) {
    if (node == NULL) {                               /* found an empty spot */
        node = (TNode *)malloc(sizeof(TNode));        /* make a new tree node here */
        if (node == NULL) return NULL;                /* no memory: give up */
        node->book = b;                               /* the node points to the book */
        node->left = node->right = NULL;              /* a new node has no children yet */
        return node;                                  /* return the new node */
    }
    if (compareText(b->title, node->book->title) < 0)  /* does the new title come before this node's title? */
        node->left = bstInsert(node->left, b);        /* yes: insert it into the left side */
    else                                              /* otherwise */
        node->right = bstInsert(node->right, b);      /* insert it into the right side */
    return node;                                      /* return this node (unchanged top) */
}                                                     /* end of bstInsert */

/*
 * bstClear
 *   WHAT   : frees every node of the tree (only the TNodes, not the books).
 *   WHY    : buildIndexes needs an empty tree before rebuilding.
 *   INPUT  : node = top of the sub-tree.   OUTPUT : nothing.
 *   ORDER  : left child -> right child -> the node itself. The children must be
 *            freed FIRST; if the parent were freed first we would lose the
 *            addresses of its children (they are stored inside the parent).
 */
static void bstClear(TNode *node) {                   /* bstClear: frees the whole tree */
    if (node == NULL) return;                         /* empty branch: nothing to free */
    bstClear(node->left);                             /* free everything on the left */
    bstClear(node->right);                            /* free everything on the right */
    free(node);                                       /* free this node itself */
}                                                     /* end of bstClear */

/*
 * findByTitle
 *   WHAT   : finds a book by its EXACT title (any letter case).
 *   WHY    : the first step of "Search by title"; if it fails we try a partial search.
 *   INPUT  : title = the text typed by the user.
 *   OUTPUT : pointer to the Book, or NULL if not found.
 *   EXAMPLE: findByTitle("the hobbit"):
 *      start at root; compareText gives c; c == 0 -> found,
 *      c < 0 -> go left, c > 0 -> go right; NULL reached -> not found.
 *   TIME   : average O(log n), worst case O(n). (A loop is enough here, no
 *            recursion needed, because we only follow ONE path.)
 */
Book *findByTitle(const char *title) {                /* findByTitle: finds a book by its exact title */
    TNode *cur = root;                                /* cur starts at the top of the tree */
    int c;                                            /* c = result of comparing two titles */
    while (cur != NULL) {                             /* keep going while there is a node */
        c = compareText(title, cur->book->title);     /* compare the wanted title with this node's title */
        if (c == 0) return cur->book;                 /* same title: found, give back the book */
        cur = (c < 0) ? cur->left : cur->right;     /* smaller -> left, bigger -> right */
    }
    return NULL;                                      /* title not in the tree: return NULL */
}                                                     /* end of findByTitle */

/*
 * walkContains
 *   WHAT   : visits the tree in A-Z order and prints every book whose title
 *            contains the word.
 *   WHY    : partial search ("hob" finds "The Hobbit") and the output comes out
 *            already sorted.
 *   INPUT  : node = top of the sub-tree, word = the text to look for,
 *            count = pointer to the caller's counter.
 *   OUTPUT : nothing returned; matches are printed and *count is increased.
 *   IN-ORDER WALK = LEFT sub-tree, then THIS node, then RIGHT sub-tree.
 *            Because left holds the smaller titles, they are printed first,
 *            so the result is A-Z.
 *   WHY "int *count" : C passes a COPY of normal variables. By passing the
 *            ADDRESS of the counter the function can change the caller's own
 *            variable.  (*count)++ needs the brackets: *count++ would move the
 *            pointer instead of adding 1 to the number.
 *   TIME   : O(n) - every node is visited once.
 */
/* in-order walk = titles come out in A-Z order */
/* walkContains: visits all titles in A-Z order and prints matches */
static void walkContains(TNode *node, const char *word, int *count) {
    if (node == NULL) return;                         /* empty branch: stop here */
    walkContains(node->left, word, count);          /* 1) everything smaller */
    if (containsText(node->book->title, word)) {      /* does this book's title contain the word? */
        if (*count == 0) printBookHeader();           /* first match: print the table heading */
        displayBook(node->book);                      /* print the matching book */
        (*count)++;                                   /* one more match found */
    }
    walkContains(node->right, word, count);         /* 3) everything bigger */
}                                                     /* end of walkContains */

/*
 * searchTitleContains
 *   WHAT   : prints all books whose title contains the word, A-Z.
 *   INPUT  : word.   OUTPUT : the number of matches (0 = nothing found).
 *   EXAMPLE: searchTitleContains("harry") prints
 *            Harry Potter and the Chamber of Secrets
 *            Harry Potter and the Sorcerer's Stone     and returns 2
 */
/* searchTitleContains: prints all titles containing the word */
int searchTitleContains(const char *word) {
    int count = 0;                                    /* count = number of matches */
    walkContains(root, word, &count);                 /* walk the whole tree and print matches */
    return count;                                     /* tell the caller how many were found */
}                                                     /* end of searchTitleContains */

/*
 * buildIndexes
 *   WHAT   : throws away the old hash table and BST and builds both again from
 *            the linked list.
 *   WHY    : the hash table and the BST hold POINTERS to the list nodes. When the
 *            list changes (a node is added, or freed) they would be wrong, so
 *            the simplest safe way is to rebuild them.
 *   INPUT/OUTPUT : nothing.
 *   PICTURE: linked list (the truth)
 *               head --> [A] --> [B] --> [C] --> NULL
 *                          |        |       |
 *               hash table +--------+-------+   (pointers to the same nodes)
 *               BST        +--------+-------+
 *   WHEN IT IS CALLED : end of loadBooks, addBook and deleteBook.
 *   WHAT BREAKS if it is forgotten:
 *     - after addBook    : new book cannot be found by id or title.
 *     - after deleteBook : the hash table / BST point to FREED memory -> crash.
 *   TIME   : O(n log n) on average (n inserts into the tree).
 *   ORDER  : hashClear and bstClear first, then root = NULL, then the loop.
 *            Without "root = NULL" root would point at the freed tree.
 */
/* ---- D. build / rebuild both structures from the linked list ---- */
void buildIndexes(void) {                             /* buildIndexes: fills the hash table and tree again */
    Book *cur;                                        /* cur = the book being added */
    hashClear();                                      /* remove the old hash table content */
    bstClear(root);                                   /* remove the old tree */
    root = NULL;                                      /* tree is empty now */
    for (cur = getHead(); cur != NULL; cur = cur->next) {  /* go through every book in the linked list */
        hashInsert(cur);                              /* add the book to the hash table */
        root = bstInsert(root, cur);                  /* add the book to the tree */
    }
}                                                     /* end of buildIndexes */

/*
 * ------------------------------ SORTING (bubble sort) ------------------------------
 * IDEA   : compare two NEIGHBOURS; if they are in the wrong order swap them.
 *          Repeat. After each round the biggest item has "bubbled up" to the end,
 *          so the next round can skip it (that is the n - 1 - i in the loop).
 * EXAMPLE (sort 5, 2, 4 from small to big, n = 3):
 *    start    : 5 2 4
 *    round 1  : j=0 compare 5,2 -> swap -> 2 5 4
 *               j=1 compare 5,4 -> swap -> 2 4 5     (5 is now in its place)
 *    round 2  : j=0 compare 2,4 -> OK, no swap       2 4 5
 *    done after n - 1 = 2 rounds.
 * TIME   : O(n^2) - two loops inside each other. For 500 books that is about
 *          125000 comparisons, fine for this project. For millions of items a
 *          faster sort (merge sort, quick sort, O(n log n)) is needed.
 * STABLE : we swap only when strictly "after", so equal items keep their old order.
 *   Where it works:      small lists, easy to understand, easy to write.
 *                        Example: sorting 500 library books.
 *   Where it does not:   big data. Example: 100000 items = about 5 billion steps.
 */
/* ================= E. SORTING (bubble sort) =================
   mode 1 = title A-Z, mode 2 = author A-Z, mode 3 = most issued first */

/*
 * isAfter
 *   WHAT   : decides whether book a must come AFTER book b in the chosen order.
 *   INPUT  : a, b = two books, mode = 1 title, 2 author, 3 popularity.
 *   OUTPUT : 1 = a must come after b (so swap them), 0 = already in order.
 *   EXAMPLE: mode 1: a="Wings of Fire", b="Cosmos"  -> compareText > 0 -> 1 (swap)
 *            mode 3: a.timesIssued=1, b.timesIssued=3 -> 1 < 3 -> 1 (swap),
 *                    so the MORE issued book moves to the front.
 */
/* returns 1 if book a must come AFTER book b for the chosen mode */
static int isAfter(const Book *a, const Book *b, int mode) {  /* isAfter: should book a come after book b? */
    if (mode == 1) return compareText(a->title, b->title) > 0;  /* mode 1: sort by title */
    if (mode == 2) return compareText(a->author, b->author) > 0;  /* mode 2: sort by author */
    return a->timesIssued < b->timesIssued;       /* mode 3: most popular first */
}                                                     /* end of isAfter */

/*
 * showSorted
 *   WHAT   : prints all books in the order chosen by mode.
 *   WHY    : "View all books (sorted)" for both students and admin.
 *   INPUT  : mode = 1 title A-Z, 2 author A-Z, 3 most issued first.
 *   OUTPUT : nothing (a table on the screen).
 *   IDEA   : copy the book POINTERS into an array and sort that array. The
 *            linked list itself is never touched, so nothing can break there.
 *               arr[0] arr[1] arr[2] ...   each cell points to a real node
 *   SWAP   : to swap two cells we need a third place, temp:
 *               temp = arr[j]; arr[j] = arr[j+1]; arr[j+1] = temp;
 *            Without temp, "arr[j] = arr[j+1]" would overwrite the first value
 *            and it would be lost.
 *   The bubble sort rounds are explained in the SORTING block above.
 *   TIME   : O(n^2).
 */
void showSorted(int mode) {                           /* showSorted: prints all books in the chosen order */
    Book *arr[MAX_BOOKS];       /* pointers to the books (the linked list itself is not changed) */
    Book *cur, *temp;                                 /* cur walks the list, temp is used for swapping */
    int n = 0, i, j;                                  /* n = number of books, i and j = loop counters */

    /* copy the book pointers into the array */
    for (cur = getHead(); cur != NULL && n < MAX_BOOKS; cur = cur->next) {
        arr[n] = cur;                                 /* store this book's pointer */
        n++;                                          /* one more book in the array */
    }
    if (n == 0) { printf("No books in the library.\n"); return; }  /* no books: tell the user and stop */

    /*
     * Round picture (3 books, mode 1): [Wings][Cosmos][Hobbit]
     *   i=0: j=0 Wings>Cosmos swap -> [Cosmos][Wings][Hobbit]
     *        j=1 Wings>Hobbit swap -> [Cosmos][Hobbit][Wings]   (Wings is last)
     *   i=1: j=0 Cosmos>Hobbit? no  -> sorted.
     */
    /* bubble sort: compare neighbours, swap them if they are in the wrong order.
       After each round the "biggest" item has moved to the end. */
    for (i = 0; i < n - 1; i++) {                     /* outer loop: one round for each pass */
        /* inner loop: compare neighbours (the sorted tail is skipped) */
        for (j = 0; j < n - 1 - i; j++) {
            if (isAfter(arr[j], arr[j + 1], mode)) {  /* are the two neighbours in the wrong order? */
                temp = arr[j];                        /* keep the first one safe in temp */
                arr[j] = arr[j + 1];                  /* move the second one to the first place */
                arr[j + 1] = temp;                    /* put the saved one in the second place */
            }
        }
    }

    printBookHeader();                                /* print the table heading */
    for (i = 0; i < n; i++) displayBook(arr[i]);      /* print each book in the sorted order */
}                                                     /* end of showSorted */
