// ====================================================================
// FILE    : oop.h
// OWNER   : Member 3 (OOP & Recommendation)
// PPT     : "OOP model (Book, User, Library) with inheritance & polymorphism",
//           "Role-based access for Student and Admin accounts",
//           "Personalized recommendations by genre, author & reading history"
// TOPICS  : 1) CLASSES       - User, Student, Admin, Recommender
//           2) INHERITANCE   - Student and Admin inherit User
//           3) POLYMORPHISM  - virtual menu(): same call, different menu per role
// NOT HERE: linked list / files -> list.c, hashing / BST / sorting -> search.c
//
// CLASS MAP
//   User (abstract)        Recommender (separate class: suggests books)
//    |-- Student
//    |-- Admin
// ====================================================================
#ifndef OOP_H
#define OOP_H

#include <string>
#include "library.h"

#define USERS_FILE "data/users.txt"

// ---------- small input helpers ----------
std::string readLine(const char *prompt);
int readInt(const char *prompt);

// ---------- recommendation (genre + author + reading history) ----------
// Gives every book the user has NOT read a score and prints the best ones:
//   +2 points for every book in the history with the same genre
//   +3 points for every book in the history with the same author
#define TOP_BOOKS 5            // how many suggestions to show

class Recommender {
public:
    void recommend(const std::string &user);
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
