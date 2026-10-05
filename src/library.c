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
#include <stdio.h>                                    /* standard input/output: printf, fopen, fgets */
#include <stdlib.h>                                   /* memory and number helpers: malloc, free, atoi */
#include <string.h>                                   /* text helpers: strcmp, strcpy, strtok */
#include <ctype.h>                                    /* letter helpers: tolower */
#include "library.h"                                  /* our own header: Book structure and function list */

#define MAX_RECORDS 2000                              /* most issue-history lines we can keep in memory */

static Book *head = NULL;   /* head = first book; the whole list hangs from it (NULL = empty) */

/* one line of data/issues.txt:  user|bookId|ISSUED  (or RETURNED) */
typedef struct {                                      /* a Record = one line of issues.txt */
    char user[NAME_LEN];                              /* name of the user who borrowed the book */
    int  bookId;                                      /* id of the borrowed book */
    char status[10];                                  /* text ISSUED or RETURNED */
} Record;                                             /* end of Record type */

static Record records[MAX_RECORDS];   /* issues.txt lines kept in memory */

Book *getHead(void) { return head; }   /* other files read the list through this */

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

/* how many books are in the linked list right now */
static int countBooks(void) {                         /* countBooks: counts the nodes in the list */
    int n = 0;                                        /* n = running total of books */
    Book *cur;                                        /* cur = the book we are looking at now */
    /* start at the first book and step to the next one; count each */
    for (cur = head; cur != NULL; cur = cur->next) n++;
    return n;                                         /* give back the total */
}                                                     /* end of countBooks */

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

static void appendNode(Book *node) {                  /* appendNode: adds one book to the end of the list */
    Book *cur;                                        /* cur = helper pointer used to walk the list */
    node->next = NULL;                         /* new node will be the last one */
    if (head == NULL) { head = node; return; }   /* empty list: node becomes first */
    cur = head;                                       /* start walking from the first book */
    while (cur->next != NULL) cur = cur->next;   /* walk to the last node */
    cur->next = node;                          /* hook the new node at the end */
}                                                     /* end of appendNode */

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
/* printBookHeader: prints the column titles of the table */
void printBookHeader(void) {
    printf("%-5s %-40s %-20s %-12s %-9s %s\n",        /* print the titles in columns of fixed width */
           "ID", "Title", "Author", "Genre", "Status", "Issued");  /* (continued) the title words */
    printf("-----------------------------------------------------------------------------------------------\n");
}                                                     /* end of printBookHeader */

void displayBook(const Book *b) {                     /* displayBook: prints one book as a table row */
    printf("%-5d %-40.40s %-20.20s %-12.12s %-9s %d\n",  /* print id, title, author and genre in neat columns */
           b->id, b->title, b->author, b->genre,      /* (continued) the book's id, title, author, genre */
           /* (continued) Available/Issued word and times issued */
           b->available ? "Available" : "Issued", b->timesIssued);
}                                                     /* end of displayBook */

/* ---------------- E + F. issue history file and issue / return ----------------
   data/issues.txt keeps one line per borrow:  user|bookId|ISSUED or RETURNED.
   This same file is the "reading history" used by the recommendation engine. */
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
#define BUF 200      /* big enough for any title or search word we compare */

/* copy src into dest in lower case (so "HARRY" and "harry" look the same) */
static void toLower(const char *src, char *dest) {    /* toLower: copies src into dest in small letters */
    int i;                                            /* i = position inside the text */
    /* copy letter by letter until the end (or the buffer is full) */
    for (i = 0; src[i] != '\0' && i < BUF - 1; i++)
        dest[i] = (char)tolower((unsigned char)src[i]);  /* turn one letter into its small-letter form */
    dest[i] = '\0';                                   /* close dest with the end-of-text marker */
}                                                     /* end of toLower */

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

/* does "text" contain "word" anywhere (ignoring case)? 1 = yes, 0 = no */
/* containsText: is 'word' found inside 'text'? */
static int containsText(const char *text, const char *word) {
    char x[BUF], y[BUF];                              /* x and y = small-letter copies */
    toLower(text, x);                                 /* lower-case copy of the text */
    toLower(word, y);                                 /* lower-case copy of the word */
    return strstr(x, y) != NULL;                      /* strstr finds y inside x; true if found, false if not */
}                                                     /* end of containsText */

/* ================= B. HASHING (by book ID) =================
   hash function: id % 101 gives the slot number; books that land in the
   same slot are chained in a small linked list (chaining). */
#define HASH_SIZE 101                                 /* number of slots in the hash table */

typedef struct HNode {                                /* a node of a hash chain */
    Book *book;                                       /* the book this node points to */
    struct HNode *next;      /* chaining: books with the same hash value */
} HNode;                                              /* end of HNode type */

/* the hash table: an array of 101 chains (one per slot) */
static HNode *table[HASH_SIZE];

/* hashId: turns an id into a slot number (remainder after dividing by 101) */
static int hashId(int id) { return id % HASH_SIZE; }

static void hashInsert(Book *b) {                     /* hashInsert: adds one book to the hash table */
    int h = hashId(b->id);                          /* slot number = id % 101 */
    HNode *n = (HNode *)malloc(sizeof(HNode));      /* new chain node */
    if (n == NULL) return;                            /* no memory: do not add */
    n->book = b;                                      /* the node points to the book (no copy) */
    n->next = table[h];                             /* put in FRONT of the slot's chain */
    table[h] = n;                                     /* the slot now starts with this new node */
}                                                     /* end of hashInsert */

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

/* findById: finds a book by its id using the hash table */
Book *findById(int id) {
    HNode *cur = table[hashId(id)];                 /* jump straight to the right slot */
    while (cur != NULL) {                             /* walk along the chain of this slot */
        if (cur->book->id == id) return cur->book;    /* id matches: give back that book */
        cur = cur->next;                              /* otherwise look at the next node */
    }
    return NULL;                                      /* id not in the table: return NULL (nothing) */
}                                                     /* end of findById */

/* ================= C. BINARY SEARCH TREE (by title) =================
   smaller titles go to the left, bigger ones to the right (A-Z order). */
typedef struct TNode {                                /* a node of the binary search tree */
    Book *book;                                       /* the book this node points to */
    struct TNode *left, *right;                       /* left = smaller titles, right = bigger titles */
} TNode;                                              /* end of TNode type */

static TNode *root = NULL;                            /* root = top node of the tree (NULL = empty tree) */

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

static void bstClear(TNode *node) {                   /* bstClear: frees the whole tree */
    if (node == NULL) return;                         /* empty branch: nothing to free */
    bstClear(node->left);                             /* free everything on the left */
    bstClear(node->right);                            /* free everything on the right */
    free(node);                                       /* free this node itself */
}                                                     /* end of bstClear */

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

/* searchTitleContains: prints all titles containing the word */
int searchTitleContains(const char *word) {
    int count = 0;                                    /* count = number of matches */
    walkContains(root, word, &count);                 /* walk the whole tree and print matches */
    return count;                                     /* tell the caller how many were found */
}                                                     /* end of searchTitleContains */

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

/* ================= E. SORTING (bubble sort) =================
   mode 1 = title A-Z, mode 2 = author A-Z, mode 3 = most issued first */

/* returns 1 if book a must come AFTER book b for the chosen mode */
static int isAfter(const Book *a, const Book *b, int mode) {  /* isAfter: should book a come after book b? */
    if (mode == 1) return compareText(a->title, b->title) > 0;  /* mode 1: sort by title */
    if (mode == 2) return compareText(a->author, b->author) > 0;  /* mode 2: sort by author */
    return a->timesIssued < b->timesIssued;       /* mode 3: most popular first */
}                                                     /* end of isAfter */

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
