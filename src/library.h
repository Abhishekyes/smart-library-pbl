/* library.h - shared header for the C core (list.c, search.c) and the C++ layer.
   The extern "C" part lets C++ code call these C functions. */
#ifndef LIBRARY_H
#define LIBRARY_H

#ifdef __cplusplus
extern "C" {
#endif

#define TITLE_LEN   60
#define AUTHOR_LEN  40
#define GENRE_LEN   30
#define NAME_LEN    30
#define MAX_HISTORY 200

#define BOOKS_FILE  "data/books.txt"
#define ISSUES_FILE "data/issues.txt"

/* One book = one node of the linked list */
typedef struct Book {
    int  id;
    char title[TITLE_LEN];
    char author[AUTHOR_LEN];
    char genre[GENRE_LEN];
    int  available;     /* 1 = on shelf, 0 = issued */
    int  timesIssued;   /* popularity counter */
    struct Book *next;
} Book;

/* ---------- list.c : linked list, file handling, issue/return ---------- */
void  loadBooks(void);
void  saveBooks(void);
Book *getHead(void);
int   addBook(int id, const char *title, const char *author, const char *genre);
int   deleteBook(int id);              /* 1 ok, 0 not found, -1 book is issued */
void  printBookHeader(void);
void  displayBook(const Book *b);
void  displayAllBooks(void);
int   issueBook(const char *user, int id);   /* 1 ok, 0 not found, -1 not available */
int   returnBook(const char *user, int id);  /* 1 ok, 0 no such issue record */
int   getUserHistory(const char *user, int ids[], int max);
void  showUserHistory(const char *user);
void  displayIssuedBooks(void);

/* ---------- search.c : hashing, BST, sorting ---------- */
void  buildIndexes(void);                  /* rebuild hash table + BST from list */
Book *findById(int id);                    /* hashing */
Book *findByTitle(const char *title);      /* BST, exact title */
int   searchTitleContains(const char *word); /* BST in-order walk, prints matches */
void  showSorted(int mode);                /* 1 title, 2 author, 3 popularity */

#ifdef __cplusplus
}
#endif
#endif
