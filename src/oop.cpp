// ====================================================================
// FILE    : oop.cpp
// OWNER   : Member 3 (OOP & Recommendation)
// TOPICS  : classes, inheritance, polymorphism (see oop.h for the class map)
// NOT HERE: linked list & files (list.c), hashing / BST / sorting (search.c)
//           - this file only CALLS those C functions.
//
// SECTIONS IN THIS FILE
//   A. input helpers
//   B. recommendation rules + Recommender   (genre, author, reading history)
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

int readInt(const char *prompt) {
    string s = readLine(prompt);
    return atoi(s.c_str());
}

// ================= B. recommendation rules =================
// Rule stores the user's reading history; children decide the scoring.
Rule::Rule(const int ids[], int n) {
    count = n;
    for (int i = 0; i < n; i++) history[i] = ids[i];
}

// +2 points for every book in the history that has the SAME GENRE
int GenreRule::score(const Book *b) {
    int s = 0;
    for (int i = 0; i < count; i++) {
        Book *old = findById(history[i]);          // hashing lookup (search.c)
        if (old != NULL && strcmp(old->genre, b->genre) == 0) s += 2;
    }
    return s;
}

// +3 points for every book in the history that has the SAME AUTHOR
int AuthorRule::score(const Book *b) {
    int s = 0;
    for (int i = 0; i < count; i++) {
        Book *old = findById(history[i]);
        if (old != NULL && strcmp(old->author, b->author) == 0) s += 3;
    }
    return s;
}

// Recommend books for one user:
//   1. read the user's reading history (data/issues.txt via list.c)
//   2. give every book the user has NOT read a score = genre points + author points
//   3. sort by score (highest first) and print the top N
void Recommender::recommend(const string &user, int topN) {
    int history[MAX_HISTORY];
    int n = getUserHistory(user.c_str(), history, MAX_HISTORY);

    if (n == 0) {                       // recommendation needs reading history
        cout << "No reading history yet. Issue a few books first,\n"
             << "then ask again - suggestions are based on what you have read.\n";
        return;
    }

    // two different rules, both used through the SAME base-class pointer type
    Rule *rules[2];
    rules[0] = new GenreRule(history, n);
    rules[1] = new AuthorRule(history, n);

    int total = 0;                      // count the books in the library
    for (Book *b = getHead(); b != NULL; b = b->next) total++;

    Book **cand = new Book *[total + 1];    // candidate books
    int *score = new int[total + 1];        // their scores
    int m = 0;

    for (Book *b = getHead(); b != NULL; b = b->next) {
        bool alreadyRead = false;
        for (int i = 0; i < n; i++)
            if (history[i] == b->id) alreadyRead = true;
        if (alreadyRead) continue;          // never recommend a book already read

        int s = 0;
        for (int r = 0; r < 2; r++) s += rules[r]->score(b);   // POLYMORPHIC call
        if (s > 0) {                        // keep only books that match something
            cand[m] = b;
            score[m] = s;
            m++;
        }
    }

    // insertion sort, highest score first (equal scores keep file order)
    for (int i = 1; i < m; i++) {
        Book *kb = cand[i];
        int ks = score[i];
        int j = i - 1;
        while (j >= 0 && score[j] < ks) {
            cand[j + 1] = cand[j];
            score[j + 1] = score[j];
            j--;
        }
        cand[j + 1] = kb;
        score[j + 1] = ks;
    }

    if (m == 0) {
        cout << "No similar unread books found right now.\n";
    } else {
        cout << "Based on your reading history (genre and author):\n";
        printf("%-5s %-40s %-20s %-12s %s\n", "ID", "Title", "Author", "Genre", "Score");
        printf("---------------------------------------------------------------------------------------\n");
        for (int i = 0; i < m && i < topN; i++)
            printf("%-5d %-40.40s %-20.20s %-12.12s %d\n", cand[i]->id, cand[i]->title,
                   cand[i]->author, cand[i]->genre, score[i]);
    }

    delete[] cand;
    delete[] score;
    for (int i = 0; i < 2; i++) delete rules[i];
}

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

void User::searchById() {
    int id = readInt("Enter book ID: ");
    Book *b = findById(id);                       // hashing
    if (b == NULL) { cout << "No book with ID " << id << ".\n"; return; }
    printBookHeader();
    displayBook(b);
}

void User::viewSorted() {
    cout << "Sort by: 1) Title  2) Author  3) Popularity\n";
    int mode = readInt("Choice: ");
    if (mode < 1 || mode > 3) { cout << "Invalid choice.\n"; return; }
    showSorted(mode);
}

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
        else if (choice == 7) rec.recommend(username, 5);
        else if (choice != 0) cout << "Invalid choice.\n";
    } while (choice != 0);
}

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
    if (addBook(id, title.c_str(), author.c_str(), genre.c_str()))
        cout << "Book added.\n";
    else
        cout << "A book with this ID already exists.\n";
}

void Admin::removeBook() {
    int id = readInt("Enter ID of the book to delete: ");
    int r = deleteBook(id);
    if (r == 1) cout << "Book deleted.\n";
    else if (r == 0) cout << "No book with ID " << id << ".\n";
    else cout << "This book is currently issued, so it cannot be deleted.\n";
}

// users.txt line:  name|password|role
static bool userExists(const string &name) {
    FILE *f = fopen(USERS_FILE, "r");
    char line[200], u[NAME_LEN];
    if (f == NULL) return false;
    while (fgets(line, sizeof(line), f) != NULL) {
        if (sscanf(line, "%29[^|]|", u) == 1 && name == u) { fclose(f); return true; }
    }
    fclose(f);
    return false;
}

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

// ================= F. login (role-based access) =================
// users.txt decides the role; a Student or an Admin object is created and
// returned as a User* - the caller does not need to know which one it is.
User *loginUser(const string &name, const string &password) {
    FILE *f = fopen(USERS_FILE, "r");
    char line[200], u[NAME_LEN], p[NAME_LEN], role[20];
    if (f == NULL) return NULL;
    while (fgets(line, sizeof(line), f) != NULL) {
        if (sscanf(line, "%29[^|]|%29[^|]|%19s", u, p, role) != 3) continue;
        if (name == u && password == p) {
            fclose(f);
            if (strcmp(role, "admin") == 0) return new Admin(name);
            return new Student(name);
        }
    }
    fclose(f);
    return NULL;
}
