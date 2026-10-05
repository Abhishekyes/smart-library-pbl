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
