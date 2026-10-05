/* search.c - Hashing (search by ID), Binary Search Tree (search by title),
   sorting (title / author / popularity).
   Both the hash table and the BST only store pointers to the linked-list nodes,
   so they are rebuilt whenever a book is added, deleted or the file is loaded. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "library.h"

/* ---------------- helpers ---------------- */
/* compare two texts ignoring upper/lower case (like strcmp) */
static int compareText(const char *a, const char *b) {
    while (*a && *b) {
        int x = tolower((unsigned char)*a), y = tolower((unsigned char)*b);
        if (x != y) return x - y;
        a++; b++;
    }
    return tolower((unsigned char)*a) - tolower((unsigned char)*b);
}

/* does "text" contain "word" (ignoring case)? */
static int containsText(const char *text, const char *word) {
    int i, j, tl = (int)strlen(text), wl = (int)strlen(word);
    for (i = 0; i + wl <= tl; i++) {
        for (j = 0; j < wl; j++)
            if (tolower((unsigned char)text[i + j]) != tolower((unsigned char)word[j])) break;
        if (j == wl) return 1;
    }
    return 0;
}

/* ================= HASHING (by book ID) ================= */
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

/* ================= BINARY SEARCH TREE (by title) ================= */
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

/* ---- build / rebuild both structures ---- */
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

/* ================= SORTING (insertion sort) ================= */
/* returns >0 if a should come AFTER b for the chosen mode */
static int isAfter(const Book *a, const Book *b, int mode) {
    if (mode == 1) return compareText(a->title, b->title) > 0;
    if (mode == 2) return compareText(a->author, b->author) > 0;
    return a->timesIssued < b->timesIssued;       /* mode 3: most popular first */
}

void showSorted(int mode) {
    int n = 0, i, j;
    Book *cur, *key, **arr;
    for (cur = getHead(); cur != NULL; cur = cur->next) n++;
    if (n == 0) { printf("No books in the library.\n"); return; }
    arr = (Book **)malloc(n * sizeof(Book *));
    if (arr == NULL) return;
    i = 0;
    for (cur = getHead(); cur != NULL; cur = cur->next) arr[i++] = cur;

    for (i = 1; i < n; i++) {            /* insertion sort */
        key = arr[i];
        j = i - 1;
        while (j >= 0 && isAfter(arr[j], key, mode)) {
            arr[j + 1] = arr[j];
            j--;
        }
        arr[j + 1] = key;
    }
    printBookHeader();
    for (i = 0; i < n; i++) displayBook(arr[i]);
    free(arr);
}
