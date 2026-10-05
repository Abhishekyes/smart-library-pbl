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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "library.h"

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

/* how many books are in the linked list right now */
static int countBooks(void) {
    int n = 0;
    Book *cur;
    for (cur = head; cur != NULL; cur = cur->next) n++;
    return n;
}

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

static void appendNode(Book *node) {
    Book *cur;
    node->next = NULL;
    if (head == NULL) { head = node; return; }
    cur = head;
    while (cur->next != NULL) cur = cur->next;
    cur->next = node;
}

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

static void writeRecords(int n) {
    FILE *f = fopen(ISSUES_FILE, "w");
    int i;
    if (f == NULL) { printf("Error: cannot write %s\n", ISSUES_FILE); return; }
    for (i = 0; i < n; i++)
        fprintf(f, "%s|%d|%s\n", records[i].user, records[i].bookId, records[i].status);
    fclose(f);
}

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

/* ids of all books this user has ever borrowed (used by the recommender) */
int getUserHistory(const char *user, int ids[], int max) {
    int n = readRecords(), i, count = 0;
    for (i = 0; i < n && count < max; i++)
        if (strcmp(records[i].user, user) == 0) ids[count++] = records[i].bookId;
    return count;
}

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


/* ################ PART 2 : Member 2 - hashing, BST, sorting ################ */
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

/* ---------------- A. helpers ---------------- */
#define BUF 200      /* big enough for any title or search word we compare */

/* copy src into dest in lower case (so "HARRY" and "harry" look the same) */
static void toLower(const char *src, char *dest) {
    int i;
    for (i = 0; src[i] != '\0' && i < BUF - 1; i++)
        dest[i] = (char)tolower((unsigned char)src[i]);
    dest[i] = '\0';
}

/* compare two texts ignoring upper/lower case. Result like strcmp:
   negative = a comes first, 0 = same, positive = b comes first */
static int compareText(const char *a, const char *b) {
    char x[BUF], y[BUF];
    toLower(a, x);
    toLower(b, y);
    return strcmp(x, y);
}

/* does "text" contain "word" anywhere (ignoring case)? 1 = yes, 0 = no */
static int containsText(const char *text, const char *word) {
    char x[BUF], y[BUF];
    toLower(text, x);
    toLower(word, y);
    return strstr(x, y) != NULL;
}

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

static void hashInsert(Book *b) {
    int h = hashId(b->id);
    HNode *n = (HNode *)malloc(sizeof(HNode));
    if (n == NULL) return;
    n->book = b;
    n->next = table[h];
    table[h] = n;
}

static void hashClear(void) {
    int i;
    HNode *cur, *nxt;
    for (i = 0; i < HASH_SIZE; i++) {
        cur = table[i];
        while (cur != NULL) { nxt = cur->next; free(cur); cur = nxt; }
        table[i] = NULL;
    }
}

Book *findById(int id) {
    HNode *cur = table[hashId(id)];
    while (cur != NULL) {
        if (cur->book->id == id) return cur->book;
        cur = cur->next;
    }
    return NULL;
}

/* ================= C. BINARY SEARCH TREE (by title) =================
   smaller titles go to the left, bigger ones to the right (A-Z order). */
typedef struct TNode {
    Book *book;
    struct TNode *left, *right;
} TNode;

static TNode *root = NULL;

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

static void bstClear(TNode *node) {
    if (node == NULL) return;
    bstClear(node->left);
    bstClear(node->right);
    free(node);
}

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

/* returns 1 if book a must come AFTER book b for the chosen mode */
static int isAfter(const Book *a, const Book *b, int mode) {
    if (mode == 1) return compareText(a->title, b->title) > 0;
    if (mode == 2) return compareText(a->author, b->author) > 0;
    return a->timesIssued < b->timesIssued;       /* mode 3: most popular first */
}

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
