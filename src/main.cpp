// ====================================================================
// FILE    : main.cpp
// OWNER   : Member 2 (program flow) - small file, uses everyone's code
// PURPOSE : starting point. Flow of the whole program:
//             1. loadBooks()  [list.c]  -> books file into the linked list,
//                                          hash table + BST are built
//             2. login screen  [oop.cpp] -> Student or Admin object
//             3. user->menu()            -> the menu of that role (polymorphism)
// ====================================================================
#include <iostream>
#include "oop.h"

using namespace std;

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
            User *u = loginUser(name, pass);
            if (u == NULL) {
                cout << "Wrong username or password.\n";
            } else {
                cout << "\nWelcome, " << u->getName() << " (" << u->getRole() << ")\n";
                u->menu();          // polymorphism: Student or Admin menu runs
                delete u;
            }
        } else if (choice != 0) {
            cout << "Invalid choice.\n";
        }
    } while (choice != 0);

    cout << "Thank you. Goodbye!\n";
    return 0;
}
