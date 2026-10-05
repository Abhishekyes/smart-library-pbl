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
#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include "library.h"

using namespace std;

// ================= 1. CLASS DECLARATIONS =================
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
    void searchByTitle();      // calls the BST in library.c (PART 2)
    void searchById();         // calls the hash table in library.c (PART 2)
    void viewSorted();         // calls the sorting in library.c (PART 2)
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

// ================= B. Recommender =================
// Recommend books for one user:
//   1. read the user's reading history (data/issues.txt, through list.c)
//   2. every book the user has NOT read gets a score:
//        +2 for each history book with the same genre
//        +3 for each history book with the same author
//   3. sort by score (highest first) and print the top books
void Recommender::recommend(const string &user) {
    int history[MAX_HISTORY];          // ids of books this user borrowed before
    int n = getUserHistory(user.c_str(), history, MAX_HISTORY);   // C function; returns how many

    if (n == 0) {                       // no history = nothing to learn from
        cout << "No reading history yet. Issue a few books first,\n"
             << "then ask again - suggestions are based on what you have read.\n";
        return;
    }

    Book *cand[MAX_BOOKS];              // books that can be recommended
    int score[MAX_BOOKS];               // score of each of those books
    int m = 0;                          // how many candidates we have

    // ---- step 2: score every book ----
    for (Book *b = getHead(); b != NULL && m < MAX_BOOKS; b = b->next) {
        int s = 0;
        bool alreadyRead = false;
        for (int i = 0; i < n; i++) {
            if (history[i] == b->id) { alreadyRead = true; break; }
            Book *old = findById(history[i]);          // a book the user read earlier
            if (old == NULL) continue;                 // that book was deleted later
            if (strcmp(old->genre, b->genre) == 0) s += 2;
            if (strcmp(old->author, b->author) == 0) s += 3;
        }
        if (alreadyRead || s == 0) continue;           // skip read books and no-match books
        cand[m] = b;
        score[m] = s;
        m++;
    }

    // ---- step 3: bubble sort, highest score first ----
    for (int i = 0; i < m - 1; i++) {
        for (int j = 0; j < m - 1 - i; j++) {
            if (score[j] < score[j + 1]) {             // wrong order -> swap both arrays
                Book *tb = cand[j];  cand[j] = cand[j + 1];   cand[j + 1] = tb;
                int ts = score[j];   score[j] = score[j + 1]; score[j + 1] = ts;
            }
        }
    }

    // ---- print ----
    if (m == 0) {
        cout << "No similar unread books found right now.\n";
        return;
    }
    cout << "Based on your reading history (genre and author):\n";
    printf("%-5s %-40s %-20s %-12s %s\n", "ID", "Title", "Author", "Genre", "Score");
    printf("---------------------------------------------------------------------------------------\n");
    for (int i = 0; i < m && i < TOP_BOOKS; i++)
        printf("%-5d %-40.40s %-20.20s %-12.12s %d\n", cand[i]->id, cand[i]->title,
               cand[i]->author, cand[i]->genre, score[i]);
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
        if (choice == 1) searchByTitle();      // inherited from User
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
        else if (choice == 7) rec.recommend(username);
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
    int r = addBook(id, title.c_str(), author.c_str(), genre.c_str());
    if (r == 1) cout << "Book added.\n";
    else if (r == 0) cout << "A book with this ID already exists.\n";
    else cout << "The library is full (maximum " << MAX_BOOKS << " books).\n";
}

void Admin::removeBook() {
    int id = readInt("Enter ID of the book to delete: ");
    int r = deleteBook(id);
    if (r == 1) cout << "Book deleted.\n";
    else if (r == 0) cout << "No book with ID " << id << ".\n";
    else cout << "This book is currently issued, so it cannot be deleted.\n";
}

// users.txt line:  name|password|role
// returns true if a user with this name is already in the file
static bool userExists(const string &name) {
    FILE *f = fopen(USERS_FILE, "r");
    char line[200];
    char *parts[3];
    if (f == NULL) return false;
    while (fgets(line, sizeof(line), f) != NULL) {
        if (splitLine(line, parts, 3) == 3 && name == parts[0]) {   // splitLine is in list.c
            fclose(f);
            return true;
        }
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
    char line[200];
    char *parts[3];                     // parts[0]=name, parts[1]=password, parts[2]=role
    if (f == NULL) return NULL;
    while (fgets(line, sizeof(line), f) != NULL) {
        if (splitLine(line, parts, 3) != 3) continue;      // skip bad / blank line
        if (name == parts[0] && password == parts[1]) {    // both must match
            fclose(f);
            if (strcmp(parts[2], "admin") == 0) return new Admin(name);
            return new Student(name);
        }
    }
    fclose(f);
    return NULL;
}

// ================= 5. main() : program start =================
int main() {
    loadBooks();     // C core: read books file into the linked list + build hash/BST

    cout << "=====================================================\n"
         << "  Intelligent Digital Library & Book Recommendation\n"
         << "=====================================================\n";

    int choice;
    do {
        cout << "\n1. Login\n0. Exit\n";
        choice = readInt("Choice: ");
        if (choice == 1) {
            string name = readLine("Username: ");
            string pass = readLine("Password: ");
            User *u = loginUser(name, pass);    // Student or Admin object, or NULL
            if (u == NULL) {
                cout << "Wrong username or password.\n";
            } else {
                cout << "\nWelcome, " << u->getName() << " (" << u->getRole() << ")\n";
                u->menu();          // polymorphism: Student or Admin menu runs
                delete u;                       // free the object after logout
            }
        } else if (choice != 0) {
            cout << "Invalid choice.\n";
        }
    } while (choice != 0);

    cout << "Thank you. Goodbye!\n";
    return 0;
}
