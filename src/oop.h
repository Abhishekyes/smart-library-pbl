// ====================================================================
// FILE    : oop.h
// OWNER   : Member 3 (OOP & Recommendation)
// PPT     : "OOP model (Book, User, Library) with inheritance & polymorphism",
//           "Role-based access for Student and Admin accounts",
//           "Personalized recommendations by genre, author & reading history"
// TOPICS  : 1) CLASSES       - User, Student, Admin, Rule, Recommender
//           2) INHERITANCE   - Student and Admin inherit User;
//                              GenreRule and AuthorRule inherit Rule
//           3) POLYMORPHISM  - virtual menu() and virtual score()
// NOT HERE: linked list / files -> list.c, hashing / BST / sorting -> search.c
//
// CLASS MAP
//   User (abstract)               Rule (abstract)
//    |-- Student                   |-- GenreRule
//    |-- Admin                     |-- AuthorRule
//                                Recommender  (uses the Rule classes)
// ====================================================================
#ifndef OOP_H
#define OOP_H

#include <string>
#include "library.h"

#define USERS_FILE "data/users.txt"

// ---------- small input helpers ----------
std::string readLine(const char *prompt);
int readInt(const char *prompt);

// ---------- recommendation rules (POLYMORPHISM) ----------
// A Rule gives a score to a book. Each child class decides HOW it scores,
// but the Recommender calls them all in the same way: rule->score(book).
class Rule {
protected:
    int history[MAX_HISTORY];      // ids of the books the user has borrowed before
    int count;                     // how many ids are stored
public:
    Rule(const int ids[], int n);
    virtual ~Rule() {}
    virtual int score(const Book *b) = 0;   // pure virtual: children must define it
};

class GenreRule : public Rule {            // +2 for every past book of the same genre
public:
    GenreRule(const int ids[], int n) : Rule(ids, n) {}
    int score(const Book *b);
};

class AuthorRule : public Rule {           // +3 for every past book of the same author
public:
    AuthorRule(const int ids[], int n) : Rule(ids, n) {}
    int score(const Book *b);
};

// Reads the user's reading history, scores every unread book with the
// rules above and prints the best ones.
class Recommender {
public:
    void recommend(const std::string &user, int topN);
};

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

class Student : public User {
public:
    Student(const std::string &name) : User(name) {}
    std::string getRole() { return "Student"; }
    void menu();
};

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

// login: returns a new Student or Admin object, or NULL if name/password is wrong
User *loginUser(const std::string &name, const std::string &password);

#endif
