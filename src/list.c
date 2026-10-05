/* list.c - Linked list of books, file handling, issue / return.
   Concepts used: linked list, file handling (flat text files). */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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

/* ---------------- books file ---------------- */
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

void loadBooks(void) {
    FILE *f = fopen(BOOKS_FILE, "r");
    char line[300];
    Book *b;
    freeList();
    if (f == NULL) {
        printf("(No %s found - starting with an empty library)\n", BOOKS_FILE);
        buildIndexes();
        return;
    }
    while (fgets(line, sizeof(line), f) != NULL) {
        b = (Book *)malloc(sizeof(Book));
        if (b == NULL) break;
        if (sscanf(line, "%d|%59[^|]|%39[^|]|%29[^|]|%d|%d",
                   &b->id, b->title, b->author, b->genre,
                   &b->available, &b->timesIssued) == 6) {
            appendNode(b);
        } else {
            free(b);   /* skip bad / blank line */
        }
    }
    fclose(f);
    buildIndexes();
}

void saveBooks(void) {
    FILE *f = fopen(BOOKS_FILE, "w");
    Book *cur;
    if (f == NULL) { printf("Error: cannot write %s\n", BOOKS_FILE); return; }
    for (cur = head; cur != NULL; cur = cur->next)
        fprintf(f, "%d|%s|%s|%s|%d|%d\n", cur->id, cur->title, cur->author,
                cur->genre, cur->available, cur->timesIssued);
    fclose(f);
}

/* ---------------- add / delete ---------------- */
int addBook(int id, const char *title, const char *author, const char *genre) {
    Book *b;
    if (findById(id) != NULL) return 0;          /* duplicate id */
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

/* ---------------- display ---------------- */
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

void displayAllBooks(void) {
    Book *cur;
    if (head == NULL) { printf("No books in the library.\n"); return; }
    printBookHeader();
    for (cur = head; cur != NULL; cur = cur->next) displayBook(cur);
}

/* ---------------- issue history file ---------------- */
static int readRecords(void) {
    FILE *f = fopen(ISSUES_FILE, "r");
    char line[200];
    int n = 0;
    if (f == NULL) return 0;
    while (n < MAX_RECORDS && fgets(line, sizeof(line), f) != NULL) {
        if (sscanf(line, "%29[^|]|%d|%9s",
                   records[n].user, &records[n].bookId, records[n].status) == 3)
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
