// oop.h - C++ OOP layer: User / Student / Admin and the recommendation rules.
// Concepts used: classes, inheritance, polymorphism (virtual functions).
#ifndef OOP_H
#define OOP_H

#include <string>
#include "library.h"

#define USERS_FILE "data/users.txt"

// ---------- small input helpers ----------
std::string readLine(const char *prompt);
int readInt(const char *prompt);

// ---------- recommendation rules (polymorphism) ----------
// Every rule gives a score to a book. The Recommender adds up the scores.
class Rule {
protected:
    int history[MAX_HISTORY];      // book ids the user has already borrowed
    int count;
public:
    Rule(const int ids[], int n);
    virtual ~Rule() {}
    virtual int score(const Book *b) = 0;   // each child rule decides its own score
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

class PopularityRule : public Rule {       // + number of times the book was issued
public:
    PopularityRule(const int ids[], int n) : Rule(ids, n) {}
    int score(const Book *b);
};

class Recommender {
public:
    void recommend(const std::string &user, int topN);
};

// ---------- users ----------
class User {
protected:
    std::string username;
public:
    User(const std::string &name) : username(name) {}
    virtual ~User() {}
    std::string getName() const { return username; }
    virtual std::string getRole() = 0;     // "Student" or "Admin"
    virtual void menu() = 0;               // each role has its own menu

    // common features for every user
    void searchByTitle();
    void searchById();
    void viewSorted();
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

// login: returns a new Student/Admin object, or NULL if name/password is wrong
User *loginUser(const std::string &name, const std::string &password);

#endif
