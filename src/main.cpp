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
// ===================== BEGINNER GLOSSARY for the C++ part =====================
// Words used again and again in this file. Each is explained ONCE here.
//
// 1) CLASS
//    A class is a blueprint that bundles DATA (variables) and FUNCTIONS (called
//    member functions or methods) together. An OBJECT is one real thing made
//    from the blueprint.  Example: class Student is the blueprint,
//    Student("priya") is one object.  (struct and class are almost the same;
//    the only difference: members of a class are private unless you say public.)
//
// 2) public / protected / private   (who is allowed to use a member)
//    +-----------+----------------------------------------------------------+
//    | public    | everybody (any code outside the class)                   |
//    | protected | the class itself AND its children (Student, Admin)       |
//    | private   | only the class itself                                    |
//    +-----------+----------------------------------------------------------+
//    Example here: username is protected, so Student::menu can print it, but
//    main() cannot touch it. addNewBook is private in Admin: only Admin uses it.
//
// 3) INHERITANCE   class Student : public User
//    Student is a CHILD (derived class) of User (the base / parent class). The
//    child gets everything the parent has and may add more.
//             User  (name, searchByTitle, searchById, viewSorted)
//        +----+----+
//        Student  Admin     (each adds its own menu, and Admin more jobs)
//    Why: write the common code ONCE in User, reuse it in both children.
//
// 4) VIRTUAL FUNCTION and POLYMORPHISM   (poly = many, morph = forms)
//    A function marked "virtual" is chosen while the program RUNS, by looking at
//    the REAL object, not at the pointer type.
//        User *u = new Student("priya");   u->menu();  -> runs Student::menu
//        User *u = new Admin("admin");     u->menu();  -> runs Admin::menu
//    The same line of code u->menu() behaves differently for each object.
//
// 5) ABSTRACT CLASS and PURE VIRTUAL FUNCTION   (virtual ... = 0;)
//    "= 0" means "this function has NO body here; every child MUST write its
//    own". A class with at least one such function is ABSTRACT: you cannot make
//    an object of it (User u("x"); gives a compile error). It only exists to be
//    a parent. That is correct here, because a user is always a Student or
//    an Admin, never "just a User".
//
// 6) std::string, cout, cin, getline
//    std::string = a text type that grows by itself (in C we needed char arrays).
//    It has helpers: s.empty(), s.length(), s.find('|'), s.c_str().
//    s.c_str() gives the old C-style text (const char *) that the C functions in
//    library.c need.  cout << ... prints; cin reads. getline(cin, s) reads a
//    WHOLE line including spaces ("The Hobbit"), while cin >> s would stop at the
//    first space.
//
// 7) extern "C"   (see library.h)
//    C++ changes the real names of functions inside the compiled file ("name
//    mangling", so that two functions may share a name). Plain C does not. The
//    block  extern "C" { ... }  in library.h tells the C++ compiler:
//    "these functions come from C, look for their PLAIN names".
//    Without it the linker says  "undefined reference to addBook".
//    (library.c is compiled by gcc, main.cpp by g++, then both are joined.)
//
// 8) new / delete
//    new Student(name) creates an object in free memory and returns a pointer to
//    it. Every new needs one delete (like malloc/free in library.c).
//
// 9) CONSTRUCTOR and DESTRUCTOR
//    Constructor = special function with the class name; runs when the object is
//    created (Student(name) : User(name) {} passes the name up to the parent).
//    Destructor  = ~ClassName(); runs when the object is deleted.
//
// 10) REFERENCE   const string &name
//    The & means "another name for the SAME variable": no copy is made (fast).
//    const means the function promises not to change it.
//
// 11) using namespace std; lets us write cout / string instead of std::cout / std::string.
#include <iostream>                                   // cout / cin: console input and output
#include <cstdio>                                     // printf and other C-style printing
#include <cstdlib>                                    // atoi and exit
#include <cstring>                                    // strcmp (compare texts)
#include <string>                                     // the string class (easy text)
#include "library.h"                                  // our header with the C functions of library.c

using namespace std;                                  // use std names (cout, string...) without writing std::

// ================= 1. CLASS DECLARATIONS =================
// #define USERS_FILE : text replacement. Every USERS_FILE becomes "data/users.txt".
// Line format in that file:   name|password|role     e.g.  priya|priya123|student
#define USERS_FILE "data/users.txt"                   // file that stores the user accounts

// ---------- small input helpers ----------
// DECLARATION vs DEFINITION: the two lines below only DECLARE the helpers (they
// tell the compiler "these functions exist, with this signature"). The code
// ("definition") is written later in this file. A function must be declared
// before it is used, so declarations are put at the top.
std::string readLine(const char *prompt);             // readLine: asks a question and returns the typed text
int readInt(const char *prompt);                      // readInt: asks a question and returns the typed number

// ---------- recommendation (genre + author + reading history) ----------
// Gives every book the user has NOT read a score and prints the best ones:
//   +2 points for every book in the history with the same genre
//   +3 points for every book in the history with the same author
#define TOP_BOOKS 5            // how many suggestions to show

// class Recommender
//   WHAT   : a small class whose only job is to suggest books.
//   WHY    : keeps the recommendation code together in one place (OOP idea:
//            one class = one responsibility).
//   public : the function recommend() can be called from outside, e.g.
//            rec.recommend("priya") in the student menu.
//   NOTE   : the real code of recommend is written below with
//            Recommender::recommend ("::" = "the function belongs to this class").
class Recommender {                                   // Recommender: class that suggests books
public:                                               // public = can be used from outside the class
    void recommend(const std::string &user);          // recommend: suggest books for one user
};                                                    // end of Recommender class

// ---------- users (INHERITANCE + role-based access) ----------
// class User  (the BASE / PARENT class)
//   WHAT   : the common part of every user: a name and the features that both
//            students and admins have (search by title, search by id, sorted view).
//   protected  : username can be used by User and by its children, not by others.
//   constructor User(...) : username(name) {}
//            ": username(name)" is an "initializer list": it stores the given name
//            in the member username when the object is created.
//   virtual ~User() {}  : VIRTUAL DESTRUCTOR. We delete objects through a User
//            pointer (delete u in main). Without "virtual", only User's cleanup
//            would run and the child's cleanup would be skipped (a classic bug).
//   getName() const : "const" at the end = this function does not change the object.
//   = 0  (getRole and menu) : PURE VIRTUAL. User has no code for them; Student and
//            Admin must write their own. That makes User an ABSTRACT class:
//            "User u(...)" is not allowed, only Student / Admin objects exist.
//   PICTURE (one parent, two children):
//                  User   [abstract: getRole()=0, menu()=0]
//          +----------+----------+
//         Student            Admin
//         getRole()="Student"  getRole()="Admin"
//         menu() = student menu menu() = admin menu
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

// class Student : public User
//   WHAT   : a normal library member. It IS-A User (inheritance), so it can use
//            searchByTitle(), searchById(), viewSorted() and username.
//   WHY    : adds only what is different: the role text and the student menu.
//   Student(...) : User(name) {} : the constructor passes the name to the parent.
//   getRole() : the pure virtual function of User, now filled in.
//   menu()    : declared here, written later as Student::menu.
class Student : public User {                         // Student is a kind of User (inheritance)
public:                                               // public = can be used from outside the class
    Student(const std::string &name) : User(name) {}  // constructor: passes the name to User
    std::string getRole() { return "Student"; }       // tells this user is a Student
    void menu();                                      // Student's own menu (code is written below)
};                                                    // end of Student class

// class Admin : public User
//   WHAT   : the librarian. Also IS-A User, so it gets the common features, and it
//            has extra jobs for managing books and accounts.
//   private: addNewBook, removeBook and addStudentAccount are used only inside
//            Admin::menu. A Student object does not have them at all and no outside
//            code can call them. This is the "role based access" of the project.
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

// loginUser (declaration)
//   The function returns "User *" = a pointer to the BASE class. It can really
//   point to a Student or to an Admin object, and the caller does not need to
//   know which. NULL means the name/password was wrong.
// login: returns a new Student or Admin object, or NULL if name/password is wrong
// declaration: loginUser is written later in this file
User *loginUser(const std::string &name, const std::string &password);

// ================= A. input helpers =================
// readLine
//   WHAT   : shows a question, reads one whole line typed by the user, returns it.
//   WHY    : every menu needs input; one helper keeps the code short and safe.
//   INPUT  : prompt = the question to show, e.g. "Choice: ".
//   OUTPUT : the typed text as a std::string (without the newline).
//   EXAMPLE: user types   The Hobbit   -> returns "The Hobbit" (space kept).
//   WHAT BREAKS if the "if (!getline...)" test is removed:
//     - when the input ends (Ctrl+D, or a test file that has no more lines)
//       getline keeps failing and every menu loop would repeat for ever
//       (endless loop). exit(0) stops the program cleanly instead.
string readLine(const char *prompt) {                 // readLine: shows the question and reads one line
    string s;                                         // s = the text typed by the user
    cout << prompt;                                   // show the question on screen
    if (!getline(cin, s)) {            // input closed (Ctrl+D / end of test file)
        cout << "\nInput ended. Exiting.\n";          // tell the user the input is over
        exit(0);                                      // stop the program normally
    }
    return s;                                         // give back the typed text
}                                                     // end of readLine

// readInt
//   WHAT   : asks a question and returns the typed NUMBER.
//   WHY    : menu choices and ids are numbers. We read a whole line first and then
//            convert, so a wrong input cannot get the program stuck.
//   INPUT  : prompt.   OUTPUT : the number.
//   EXAMPLE: "12" -> 12 ;  "abc" -> 0 ;  "" -> 0  (atoi gives 0 when there is no
//            number; 0 is "Logout/Exit" in the menus, which is a safe choice).
//   NOTE   : s.c_str() gives the C-style text that atoi needs.
int readInt(const char *prompt) {                     // readInt: shows the question and reads a number
    string s = readLine(prompt);                      // read the typed line first
    return atoi(s.c_str());                           // convert the text into a number (atoi)
}                                                     // end of readInt

// ================= B. Recommender =================
// Recommend books for one user:
//   1. read the user's reading history (data/issues.txt, through library.c)
//   2. every book the user has NOT read gets a score:
//        +2 for each history book with the same genre
//        +3 for each history book with the same author
//   3. sort by score (highest first) and print the top books
// Recommender::recommend
//   WHAT   : suggests up to TOP_BOOKS unread books that are like the books the
//            user read before.
//   WHY    : the "smart" feature of the project (student menu option 7).
//   INPUT  : user = name of the student (const string & = no copy, no change).
//   OUTPUT : nothing returned; a table is printed.
//   STEPS  : 1) history = ids from getUserHistory (C function)
//            2) for every book of the library: give a score
//            3) bubble sort the candidates, highest score first
//            4) print the first 5
//   SCORE RULE : for every history book:  same genre +2,   same author +3.
//   WORKED EXAMPLE (priya, history = 201, 203, 102):
//        201 = Rowling, Fantasy      203 = Tolkien, Fantasy
//        102 = Reema Thareja, Programming
//      candidate 202 "Harry Potter and the Chamber of Secrets" (Rowling, Fantasy):
//          vs 201 : same genre +2, same author +3   = 5
//          vs 203 : same genre +2                   = 2
//          vs 102 : nothing                         = 0
//          TOTAL score = 7
//      candidate 204 "The Lord of the Rings" (Tolkien, Fantasy):
//          2 + (2+3) + 0 = 7
//      candidate 103 "Let Us C" (Programming): only 102 matches the genre = 2
//      candidate 206 "To Kill a Mockingbird" (Fiction): 0 -> skipped (s == 0)
//      books 201, 203, 102 are skipped (alreadyRead).
//      After the sort, 204 (7) and 202 (7) come first. Equal scores keep the
//      order of the library list, because we swap only when strictly smaller.
//   TIME   : scoring O(n * h) (n books, h history size; findById is O(1)),
//            sorting O(m^2) for m candidates.
//   Where it works:      users with some history. Example: priya read Tolkien, so
//                        other Fantasy books score high.
//   Where it does not:   a new user (no history = no suggestions), or a taste
//                        that does not repeat genre/author. Example: someone who
//                        reads one book of each genre gets many small, equal scores.
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

    // Scoring loop: for ONE library book b we compare it with every history book.
    //   - "break" stops the history loop as soon as we know b was already read
    //     (the half-finished score s is then thrown away by the "continue" below).
    //   - findById(history[i]) returns NULL if that old book was deleted later; we
    //     must check it before using old->genre, otherwise the program crashes.
    //   - strcmp(...) == 0 means "the two texts are equal".
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

    // Bubble sort again (see PART 2 of library.c). Here the order is HIGH to LOW and
    // two arrays move together, so BOTH are swapped, otherwise a score would stay with
    // the wrong book.   Example scores 2, 7, 2:
    //    j=0: 2 < 7 -> swap -> 7 2 2        j=1: 2 < 2? no.   Result 7 2 2.
    //    (Time O(m^2); fine because the candidate list is small.)
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

    // Printing: m == 0 means no candidate got a score (nothing similar to suggest).
    // Otherwise we print at most TOP_BOOKS rows.  printf %-40.40s = left aligned,
    // 40 wide, cut after 40 letters.
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
// User::searchByTitle
//   WHAT   : asks for a title and shows the matching book(s).
//   WHY    : feature shared by Student and Admin, written ONCE in the parent.
//            ("User::" means this function belongs to class User.)
//   INPUT  : nothing (it asks the user).    OUTPUT : nothing (prints).
//   FLOW   : 1) exact search with the BST (findByTitle), O(log n) on average
//            2) if no exact match, a partial search (searchTitleContains)
//   EXAMPLE: typing "the hobbit" -> exact match -> one row.
//            typing "harry" -> no exact title -> the partial search prints 2 rows.
//            typing "xyz" -> prints  No book found with "xyz".
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

// User::searchById
//   WHAT   : asks for a book id and shows that book.
//   WHY    : uses the hash table, so it is O(1) on average.
//   EXAMPLE: id 202 -> findById(202) -> prints the row of
//            "Harry Potter and the Chamber of Secrets".
//            id 999 -> findById gives NULL -> "No book with ID 999."
//   NOTE   : the NULL check is essential. Without it, printing a NULL book crashes.
void User::searchById() {                             // searchById: asks for an id and shows the book
    int id = readInt("Enter book ID: ");              // ask for the id number
    Book *b = findById(id);                       // hashing
    // no such book: tell the user and stop
    if (b == NULL) { cout << "No book with ID " << id << ".\n"; return; }
    printBookHeader();                                // print the table heading
    displayBook(b);                                   // print the book
}                                                     // end of searchById

// User::viewSorted
//   WHAT   : asks how to sort (1 title, 2 author, 3 popularity) and shows all books.
//   WHY    : the sorting itself is in showSorted (C, bubble sort); this function
//            only checks the choice. Anything outside 1..3 is rejected so
//            showSorted never gets a wrong mode.
void User::viewSorted() {                             // viewSorted: shows all books in a chosen order
    cout << "Sort by: 1) Title  2) Author  3) Popularity\n";  // show the sort choices
    int mode = readInt("Choice: ");                   // read the user's choice
    if (mode < 1 || mode > 3) { cout << "Invalid choice.\n"; return; }  // not 1, 2 or 3: tell the user and stop
    showSorted(mode);                                 // show the books sorted by that choice
}                                                     // end of viewSorted

// ================= D. Student (child of User) =================
// Student::menu
//   WHAT   : the menu a student sees after login; it repeats until Logout.
//   WHY    : this is the student's whole user interface.
//   "Student::menu" = the body of menu() declared in class Student. It is the
//   Student's own version of the virtual function User::menu (polymorphism).
//   FLOW   : do { show menu; read choice; run the chosen job } while (choice != 0);
//            A do-while runs the body at least once, which a menu needs.
//   CHOICES: +--------+-----------------------------------------+
//            | 1      | searchByTitle()  (inherited from User)  |
//            | 2      | searchById()     (inherited from User)  |
//            | 3      | viewSorted()     (inherited from User)  |
//            | 4      | issueBook(...)   (C function)           |
//            | 5      | returnBook(...)  (C function)           |
//            | 6      | showUserHistory(...)                    |
//            | 7      | rec.recommend(...)                      |
//            | 0      | leave the loop (Logout)                 |
//            +--------+-----------------------------------------+
//   EXAMPLE: choice 4, id 201 -> issueBook("priya", 201):
//            returns 1 "Book issued successfully." / 0 "No book with ID" /
//            -1 "already issued to someone else".
//   WHAT BREAKS if "while (choice != 0)" is wrong: the user can never log out,
//            or the menu closes at once.
//   NOTE   : username.c_str() converts the std::string to a C text for the C code.
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
// Admin::addNewBook
//   WHAT   : asks the admin for id, title, author, genre and adds the book.
//   WHY    : check the input HERE (C++ side) so library.c only gets valid data.
//   INPUT  : nothing (it asks).   OUTPUT : nothing (prints the result).
//   CHECKS : id must be > 0; title, author and genre must not be empty.
//   EXAMPLE: id 999, "Clean Code", "Robert Martin", "Programming"
//            -> addBook returns 1 -> "Book added."
//            id 201 -> addBook returns 0 -> "A book with this ID already exists."
//            (return codes of addBook: 1 added, 0 duplicate, -1 library full)
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

// Admin::removeBook
//   WHAT   : asks for an id and deletes that book.
//   EXAMPLE: id 105 while issued -> deleteBook returns -1 -> "...cannot be deleted."
//            id 999 -> returns 0 -> "No book with ID 999."
//            free id -> returns 1 -> "Book deleted."
//            (the C function also rebuilds the hash table and the BST)
void Admin::removeBook() {                            // removeBook: asks for an id and deletes that book
    int id = readInt("Enter ID of the book to delete: ");  // ask which book id to delete
    int r = deleteBook(id);                           // try to delete; r = result
    if (r == 1) cout << "Book deleted.\n";            // 1 = deleted
    else if (r == 0) cout << "No book with ID " << id << ".\n";  // 0 = id not found
    else cout << "This book is currently issued, so it cannot be deleted.\n";  // otherwise: the book is on loan
}                                                     // end of removeBook

// userExists
//   WHAT   : checks if a username is already stored in data/users.txt.
//   WHY    : two accounts with the same name would make login unclear.
//   INPUT  : name.   OUTPUT : true = found, false = not found (or no file).
//   EXAMPLE: userExists("priya") -> true.   userExists("amit") -> false.
//   NOTES  : "static" = private to this file. fopen "r" opens for reading and
//            returns NULL if the file is missing. splitLine (from library.c) cuts
//            "priya|priya123|student" into 3 pieces; parts[0] is the name.
//            name == parts[0] compares a std::string with a C text correctly
//            (in C++ the string class makes == compare the letters).
//            fclose is called on EVERY way out, otherwise the file stays open.
// users.txt line:  name|password|role
// returns true if a user with this name is already in the file
static bool userExists(const string &name) {          // userExists: is this username already in users.txt?
    FILE *f = fopen(USERS_FILE, "r");                 // open the users file for reading
    char line[200];                                   // line = one line of the file
    char *parts[3];                                   // parts = pieces of a line
    if (f == NULL) return false;                      // no file: nobody exists, return false
    while (fgets(line, sizeof(line), f) != NULL) {    // read the file line by line
        if (splitLine(line, parts, 3) == 3 && name == parts[0]) {   // splitLine is in library.c
            fclose(f);                                // close the file
            return true;                              // found: return true
        }
    }
    fclose(f);                                        // close the file
    return false;                                     // not found: return false
}                                                     // end of userExists

// Admin::addStudentAccount
//   WHAT   : creates a new student login (adds a line to data/users.txt).
//   INPUT  : nothing (asks for username and password).
//   OUTPUT : nothing (prints a message).
//   CHECKS : not empty; no '|' (it is our field separator, one would break the
//            line); username shorter than NAME_LEN (30); name not already used.
//   EXAMPLE: "amit" / "amit123" -> the line  amit|amit123|student  is appended.
//            "a|b" -> rejected because of the '|'.
//   NOTES  : fopen mode "a" (append) keeps the old accounts. If "w" were used,
//            every existing account would be erased.
//            string::npos means "not found" for s.find(); so "find(..) != npos"
//            means "the character IS in the text".
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

// Admin::menu
//   WHAT   : the menu an admin sees after login; repeats until Logout.
//   WHY    : the Admin's own version of the virtual menu(); different options from
//            the student (add/delete books, view issued books, create accounts).
//   CHOICES: 1 add book | 2 delete book | 3 sorted view | 4 search title |
//            5 search id | 6 issued books | 7 create student account | 0 logout
//   NOTE   : addNewBook(), removeBook() and addStudentAccount() are private, which
//            is fine because Admin::menu is part of the same class.
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
// loginUser
//   WHAT   : checks the typed name and password against data/users.txt and
//            creates the right kind of object.
//   WHY    : this is the "role based access": the file decides the role, the
//            program decides what the user can do.
//   INPUT  : name, password (const string & = no copy, no change).
//   OUTPUT : User * pointing to a NEW Admin or NEW Student object,
//            or NULL if the login failed.
//   EXAMPLE: "priya" / "priya123" -> line priya|priya123|student -> new Student
//            "admin" / "admin123"  -> role "admin"             -> new Admin
//            "priya" / "wrong"     -> no match                 -> NULL
//   POLYMORPHISM: both objects are returned as the same type User *. The caller
//            only calls u->menu(); the right menu runs.
//   MEMORY : "new" creates the object; the caller MUST delete it later (main does
//            that after logout). Forgetting delete = memory leak.
//   NOTES  : strcmp(parts[2], "admin") == 0 tests the role text.
//            fclose(f) is called before every return, so the file is never left open.
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

// main
//   WHAT   : the start of the whole program.
//   FLOW   : loadBooks() (read the file, build list + hash + BST)
//            -> banner -> main loop: 1 Login / 0 Exit
//            -> loginUser -> u->menu() -> delete u -> back to the main loop.
//   KEY POINTS:
//     - User *u may point to a Student or an Admin; u->menu() runs the correct
//       one (polymorphism, thanks to virtual).
//     - "if (u == NULL)" : checking for NULL before using u. Without it a wrong
//       password would crash the program at u->getName().
//     - "delete u" : frees the object after logout. Without it every login leaks
//       memory. It works correctly for Student and Admin because ~User is virtual.
//     - "return 0" : tells the operating system that the program ended normally.
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
