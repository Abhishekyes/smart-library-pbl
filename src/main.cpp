// ====================================================================
// FILE    : main.cpp  (C++ part of the project)
// OWNER   : Member 3 (OOP & Recommendation); main() = start-up
// PPT     : OOP model with inheritance & polymorphism, role-based access
//           (Student/Admin), recommendations by genre, author & reading history
// TOPICS  : 1) CLASSES      - User, Student, Admin, Recommender
//           2) INHERITANCE  - Student and Admin inherit User
//           3) POLYMORPHISM - virtual menu(): same call, different menu per role
// CALLS   : the C functions of library.c through library.h
//
// SECTIONS
//   1. class declarations   2. input helpers + Recommender
//   3. User / Student / Admin code   4. login   5. main()
// ====================================================================
#include <iostream>                                   // cout / cin: console input and output
#include <cstdio>                                     // printf and other C-style printing
#include <cstdlib>                                    // atoi and exit
#include <cstring>                                    // strcmp (compare texts)
#include <string>                                     // the string class (easy text)
#include "library.h"                                  // our header with the C functions of library.c

using namespace std;                                  // use std names (cout, string...) without writing std::

// ================= 1. CLASS DECLARATIONS =================
#define USERS_FILE "data/users.txt"                   // file that stores the user accounts

// ---------- small input helpers ----------
std::string readLine(const char *prompt);             // readLine: asks a question and returns the typed text
int readInt(const char *prompt);                      // readInt: asks a question and returns the typed number

// ---------- recommendation (genre + author + reading history) ----------
// Gives every book the user has NOT read a score and prints the best ones:
//   +2 points for every book in the history with the same genre
//   +3 points for every book in the history with the same author
#define TOP_BOOKS 5            // how many suggestions to show

class Recommender {                                   // Recommender: class that suggests books
public:                                               // public = can be used from outside the class
    void recommend(const std::string &user);          // recommend: suggest books for one user
};                                                    // end of Recommender class

// ---------- users (INHERITANCE + role-based access) ----------
class User {                                          // User: base class for every kind of user
protected:                                            // protected = only this class and its children can see it
    std::string username;                             // the user's name
public:                                               // public = can be used from outside the class
    User(const std::string &name) : username(name) {}  // constructor: saves the name when a User is made
    virtual ~User() {}                                // virtual destructor: cleans up correctly
    std::string getName() const { return username; }  // getName: returns the user's name
    virtual std::string getRole() = 0;     // "Student" or "Admin"
    virtual void menu() = 0;               // each role has its OWN menu (polymorphism)

    // features common to every user (written once here, used by both children)
    void searchByTitle();      // calls the BST in library.c (PART 2)
    void searchById();         // calls the hash table in library.c (PART 2)
    void viewSorted();         // calls the sorting in library.c (PART 2)
};                                                    // end of User class

class Student : public User {                         // Student is a kind of User (inheritance)
public:                                               // public = can be used from outside the class
    Student(const std::string &name) : User(name) {}  // constructor: passes the name to User
    std::string getRole() { return "Student"; }       // tells this user is a Student
    void menu();                                      // Student's own menu (code is written below)
};                                                    // end of Student class

class Admin : public User {                           // Admin is a kind of User (inheritance)
public:                                               // public = can be used from outside the class
    Admin(const std::string &name) : User(name) {}    // constructor: passes the name to User
    std::string getRole() { return "Admin"; }         // tells this user is an Admin
    void menu();                                      // Admin's own menu (code is written below)
private:                                              // private = only Admin itself can use these
    void addNewBook();                                // admin job: add a book
    void removeBook();                                // admin job: delete a book
    void addStudentAccount();                         // admin job: create a student login
};                                                    // end of Admin class

// login: returns a new Student or Admin object, or NULL if name/password is wrong
// declaration: loginUser is written later in this file
User *loginUser(const std::string &name, const std::string &password);

// ================= A. input helpers =================
string readLine(const char *prompt) {                 // readLine: shows the question and reads one line
    string s;                                         // s = the text typed by the user
    cout << prompt;                                   // show the question on screen
    if (!getline(cin, s)) {            // input closed (Ctrl+D / end of test file)
        cout << "\nInput ended. Exiting.\n";          // tell the user the input is over
        exit(0);                                      // stop the program normally
    }
    return s;                                         // give back the typed text
}                                                     // end of readLine

int readInt(const char *prompt) {                     // readInt: shows the question and reads a number
    string s = readLine(prompt);                      // read the typed line first
    return atoi(s.c_str());                           // convert the text into a number (atoi)
}                                                     // end of readInt

// ================= B. Recommender =================
// Recommend books for one user:
//   1. read the user's reading history (data/issues.txt, through list.c)
//   2. every book the user has NOT read gets a score:
//        +2 for each history book with the same genre
//        +3 for each history book with the same author
//   3. sort by score (highest first) and print the top books
void Recommender::recommend(const string &user) {     // recommend: suggests books using the user's history
    int history[MAX_HISTORY];          // ids of books this user borrowed before
    int n = getUserHistory(user.c_str(), history, MAX_HISTORY);   // C function; returns how many

    if (n == 0) {                       // no history = nothing to learn from
        cout << "No reading history yet. Issue a few books first,\n"  // tell the user there is no history yet
             // (continued) second line of the message
             << "then ask again - suggestions are based on what you have read.\n";
        return;                                       // leave the function
    }

    Book *cand[MAX_BOOKS];              // books that can be recommended
    int score[MAX_BOOKS];               // score of each of those books
    int m = 0;                          // how many candidates we have

    // ---- step 2: score every book ----
    // go through every book of the library (b = current book)
    for (Book *b = getHead(); b != NULL && m < MAX_BOOKS; b = b->next) {
        int s = 0;                                    // s = score of this book
        bool alreadyRead = false;                     // alreadyRead = true if the user read this book
        for (int i = 0; i < n; i++) {                 // look at every book in the history
            // user already read it: mark it and stop checking
            if (history[i] == b->id) { alreadyRead = true; break; }
            Book *old = findById(history[i]);          // a book the user read earlier
            if (old == NULL) continue;                 // that book was deleted later
            if (strcmp(old->genre, b->genre) == 0) s += 2;  // same genre: add 2 points
            if (strcmp(old->author, b->author) == 0) s += 3;  // same author: add 3 points
        }
        if (alreadyRead || s == 0) continue;           // skip read books and no-match books
        cand[m] = b;                                  // remember this book as a candidate
        score[m] = s;                                 // remember its score
        m++;                                          // one more candidate
    }

    // ---- step 3: bubble sort, highest score first ----
    for (int i = 0; i < m - 1; i++) {                 // outer loop: one round per pass
        for (int j = 0; j < m - 1 - i; j++) {         // inner loop: compare neighbours
            if (score[j] < score[j + 1]) {             // wrong order -> swap both arrays
                Book *tb = cand[j];  cand[j] = cand[j + 1];   cand[j + 1] = tb;  // swap the two books
                // swap the two scores in the same way
                int ts = score[j];   score[j] = score[j + 1]; score[j + 1] = ts;
            }
        }
    }

    // ---- print ----
    if (m == 0) {                                     // no candidates found
        cout << "No similar unread books found right now.\n";  // tell the user
        return;                                       // leave the function
    }
    cout << "Based on your reading history (genre and author):\n";  // print a heading line
    // print the column titles
    printf("%-5s %-40s %-20s %-12s %s\n", "ID", "Title", "Author", "Genre", "Score");
    printf("---------------------------------------------------------------------------------------\n");
    for (int i = 0; i < m && i < TOP_BOOKS; i++)      // show at most TOP_BOOKS candidates
        // print one row: id, title, ...
        printf("%-5d %-40.40s %-20.20s %-12.12s %d\n", cand[i]->id, cand[i]->title,
               cand[i]->author, cand[i]->genre, score[i]);  // (continued) author, genre, score
}                                                     // end of recommend

// ================= C. common user features (User class) =================
void User::searchByTitle() {                          // searchByTitle: asks for a title and shows the result
    string word = readLine("Enter title (or part of it): ");  // ask the user for the title
    if (word.empty()) { cout << "Nothing entered.\n"; return; }  // nothing typed: tell the user and stop

    Book *exact = findByTitle(word.c_str());      // BST exact lookup
    if (exact != NULL) {                              // if an exact match was found
        printBookHeader();                            // print the table heading
        displayBook(exact);                           // print the book
        return;                                       // stop - no need to look further
    }
    if (searchTitleContains(word.c_str()) == 0)   // BST walk for partial matches
        cout << "No book found with \"" << word << "\".\n";  // nothing matched: tell the user
}                                                     // end of searchByTitle

void User::searchById() {                             // searchById: asks for an id and shows the book
    int id = readInt("Enter book ID: ");              // ask for the id number
    Book *b = findById(id);                       // hashing
    // no such book: tell the user and stop
    if (b == NULL) { cout << "No book with ID " << id << ".\n"; return; }
    printBookHeader();                                // print the table heading
    displayBook(b);                                   // print the book
}                                                     // end of searchById

void User::viewSorted() {                             // viewSorted: shows all books in a chosen order
    cout << "Sort by: 1) Title  2) Author  3) Popularity\n";  // show the sort choices
    int mode = readInt("Choice: ");                   // read the user's choice
    if (mode < 1 || mode > 3) { cout << "Invalid choice.\n"; return; }  // not 1, 2 or 3: tell the user and stop
    showSorted(mode);                                 // show the books sorted by that choice
}                                                     // end of viewSorted

// ================= D. Student (child of User) =================
void Student::menu() {                                // Student::menu: the menu shown to a student
    Recommender rec;                                  // rec = object that gives recommendations
    int choice;                                       // choice = number typed by the user
    do {                                              // repeat the menu again and again
        // print the menu title with the user's name
        cout << "\n===== STUDENT MENU (" << username << ") =====\n"
             << "1. Search book by title\n"           // menu option text
             << "2. Search book by ID\n"              // menu option text
             << "3. View all books (sorted)\n"        // menu option text
             << "4. Issue a book\n"                   // menu option text
             << "5. Return a book\n"                  // menu option text
             << "6. My reading history\n"             // menu option text
             << "7. Recommend books for me\n"         // menu option text
             << "0. Logout\n";                        // menu option text
        choice = readInt("Choice: ");                 // read the user's choice
        cout << "\n";                                 // print an empty line
        if (choice == 1) searchByTitle();      // inherited from User
        else if (choice == 2) searchById();           // choice 2: search by id
        else if (choice == 3) viewSorted();           // choice 3: show sorted books
        else if (choice == 4) {                       // choice 4: issue a book
            int id = readInt("Enter ID of the book to issue: ");  // ask which book id to issue
            int r = issueBook(username.c_str(), id);  // try to issue it; r = result (1, 0 or -1)
            if (r == 1) cout << "Book issued successfully.\n";  // 1 = success
            else if (r == 0) cout << "No book with ID " << id << ".\n";  // 0 = no such book
            // otherwise: the book is already on loan
            else cout << "Sorry, this book is already issued to someone else.\n";
        }
        else if (choice == 5) {                       // choice 5: return a book
            int id = readInt("Enter ID of the book to return: ");  // ask which book id to return
            // returnBook gives 1 (true) on success
            if (returnBook(username.c_str(), id)) cout << "Book returned successfully.\n";
            else cout << "You have not issued a book with this ID.\n";  // otherwise: the user never issued it
        }
        else if (choice == 6) showUserHistory(username.c_str());  // choice 6: show this user's history
        else if (choice == 7) rec.recommend(username);  // choice 7: show recommendations
        else if (choice != 0) cout << "Invalid choice.\n";  // any other number except 0 is invalid
    } while (choice != 0);                            // repeat until the user chooses 0 (logout)
}                                                     // end of Student::menu

// ================= E. Admin (child of User) =================
void Admin::addNewBook() {                            // addNewBook: asks the admin for book details and adds it
    int id = readInt("Book ID (number): ");           // ask for the book id
    // zero or negative id is not allowed: stop
    if (id <= 0) { cout << "ID must be a positive number.\n"; return; }
    string title  = readLine("Title: ");              // ask for the title
    string author = readLine("Author: ");             // ask for the author
    string genre  = readLine("Genre: ");              // ask for the genre
    if (title.empty() || author.empty() || genre.empty()) {  // if any of the three is empty
        cout << "Title, author and genre cannot be empty.\n";  // tell the admin
        return;                                       // stop without adding
    }
    int r = addBook(id, title.c_str(), author.c_str(), genre.c_str());  // try to add the book; r = result
    if (r == 1) cout << "Book added.\n";              // 1 = added
    else if (r == 0) cout << "A book with this ID already exists.\n";  // 0 = id already used
    // otherwise: the library is full
    else cout << "The library is full (maximum " << MAX_BOOKS << " books).\n";
}                                                     // end of addNewBook

void Admin::removeBook() {                            // removeBook: asks for an id and deletes that book
    int id = readInt("Enter ID of the book to delete: ");  // ask which book id to delete
    int r = deleteBook(id);                           // try to delete; r = result
    if (r == 1) cout << "Book deleted.\n";            // 1 = deleted
    else if (r == 0) cout << "No book with ID " << id << ".\n";  // 0 = id not found
    else cout << "This book is currently issued, so it cannot be deleted.\n";  // otherwise: the book is on loan
}                                                     // end of removeBook

// users.txt line:  name|password|role
// returns true if a user with this name is already in the file
static bool userExists(const string &name) {          // userExists: is this username already in users.txt?
    FILE *f = fopen(USERS_FILE, "r");                 // open the users file for reading
    char line[200];                                   // line = one line of the file
    char *parts[3];                                   // parts = pieces of a line
    if (f == NULL) return false;                      // no file: nobody exists, return false
    while (fgets(line, sizeof(line), f) != NULL) {    // read the file line by line
        if (splitLine(line, parts, 3) == 3 && name == parts[0]) {   // splitLine is in list.c
            fclose(f);                                // close the file
            return true;                              // found: return true
        }
    }
    fclose(f);                                        // close the file
    return false;                                     // not found: return false
}                                                     // end of userExists

void Admin::addStudentAccount() {                     // addStudentAccount: creates a new student login
    string name = readLine("New student username: ");  // ask for the new username
    string pass = readLine("Password: ");             // ask for the password
    if (name.empty() || pass.empty() ||               // invalid if the name or password is empty, or
        // if either contains '|' (our separator), or
        name.find('|') != string::npos || pass.find('|') != string::npos ||
        name.length() >= NAME_LEN) {                  // if the username is too long
        // tell the admin what is wrong
        cout << "Invalid username/password (no '|' allowed, username under 30 characters).\n";
        return;                                       // stop without creating
    }
    // username already taken: tell the admin and stop
    if (userExists(name)) { cout << "This username already exists.\n"; return; }
    FILE *f = fopen(USERS_FILE, "a");                 // open the users file to add a line at the end
    // cannot open the file: tell the admin and stop
    if (f == NULL) { cout << "Error: cannot write users file.\n"; return; }
    fprintf(f, "%s|%s|student\n", name.c_str(), pass.c_str());  // write the new line  name|password|student
    fclose(f);                                        // close the file
    cout << "Student account created.\n";             // tell the admin it worked
}                                                     // end of addStudentAccount

void Admin::menu() {                                  // Admin::menu: the menu shown to an admin
    int choice;                                       // choice = number typed by the user
    do {                                              // repeat the menu again and again
        cout << "\n===== ADMIN MENU (" << username << ") =====\n"  // print the menu title with the admin's name
             << "1. Add a book\n"                     // menu option text
             << "2. Delete a book\n"                  // menu option text
             << "3. View all books (sorted)\n"        // menu option text
             << "4. Search book by title\n"           // menu option text
             << "5. Search book by ID\n"              // menu option text
             << "6. View currently issued books\n"    // menu option text
             << "7. Create a student account\n"       // menu option text
             << "0. Logout\n";                        // menu option text
        choice = readInt("Choice: ");                 // read the user's choice
        cout << "\n";                                 // print an empty line
        if (choice == 1) addNewBook();                // choice 1: add a book
        else if (choice == 2) removeBook();           // choice 2: delete a book
        else if (choice == 3) viewSorted();           // choice 3: show sorted books
        else if (choice == 4) searchByTitle();        // choice 4: search by title
        else if (choice == 5) searchById();           // choice 5: search by id
        else if (choice == 6) displayIssuedBooks();   // choice 6: show issued books
        else if (choice == 7) addStudentAccount();    // choice 7: create a student account
        else if (choice != 0) cout << "Invalid choice.\n";  // any other number except 0 is invalid
    } while (choice != 0);                            // repeat until the admin chooses 0 (logout)
}                                                     // end of Admin::menu

// ================= F. login (role-based access) =================
// users.txt decides the role; a Student or an Admin object is created and
// returned as a User* - the caller does not need to know which one it is.
// loginUser: checks name and password, returns the right object
User *loginUser(const string &name, const string &password) {
    FILE *f = fopen(USERS_FILE, "r");                 // open the users file for reading
    char line[200];                                   // line = one line of the file
    char *parts[3];                     // parts[0]=name, parts[1]=password, parts[2]=role
    if (f == NULL) return NULL;                       // no file: login fails, return NULL
    while (fgets(line, sizeof(line), f) != NULL) {    // read the file line by line
        if (splitLine(line, parts, 3) != 3) continue;      // skip bad / blank line
        if (name == parts[0] && password == parts[1]) {    // both must match
            fclose(f);                                // close the file
            // role 'admin': make and return an Admin object
            if (strcmp(parts[2], "admin") == 0) return new Admin(name);
            return new Student(name);                 // otherwise make and return a Student object
        }
    }
    fclose(f);                                        // close the file
    return NULL;                                      // no match found: return NULL (login failed)
}                                                     // end of loginUser

// ================= 5. main() : program start =================
int main() {                                          // main: the program starts here
    loadBooks();     // C core: read books file into the linked list + build hash/BST

    cout << "=====================================================\n"  // print the welcome banner
         << "  Intelligent Digital Library & Book Recommendation\n"  // (continued) banner title
         << "=====================================================\n";  // (continued) banner bottom line

    int choice;                                       // choice = number typed in the main menu
    do {                                              // repeat the main menu
        cout << "\n1. Login\n0. Exit\n";              // show the main menu
        choice = readInt("Choice: ");                 // read the user's choice
        if (choice == 1) {                            // choice 1 = login
            string name = readLine("Username: ");     // ask for the username
            string pass = readLine("Password: ");     // ask for the password
            User *u = loginUser(name, pass);    // Student or Admin object, or NULL
            if (u == NULL) {                          // login failed
                cout << "Wrong username or password.\n";  // tell the user
            } else {                                  // login worked
                // greet the user with name and role
                cout << "\nWelcome, " << u->getName() << " (" << u->getRole() << ")\n";
                u->menu();          // polymorphism: Student or Admin menu runs
                delete u;                       // free the object after logout
            }
        } else if (choice != 0) {                     // any other number except 0 is invalid
            cout << "Invalid choice.\n";              // tell the user
        }
    } while (choice != 0);                            // repeat until the user chooses 0 (exit)

    cout << "Thank you. Goodbye!\n";                  // say goodbye
    return 0;                                         // 0 = the program ended normally
}                                                     // end of main
