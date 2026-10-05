# Viva questions and simple answers

## Overall
- **Why both C and C++?** C core is fast and close to memory (lists, trees, hash). C++ gives classes, inheritance, polymorphism for users and recommendations. They are compiled separately and linked; `extern "C"` in `library.h` lets C++ call C.
- **Why text files, not a database?** Course scope; simple, human-readable. Limit: whole file rewritten each change, no multi-user safety.

## Member 1 (list.c)
- **Why a linked list?** Easy add/delete without shifting. Weakness: no direct jump to n-th book; append walks the list (O(n)).
- **How is a book deleted?** Find the node, link previous node to next, `free` it. Not allowed if the book is issued.
- **How does return work?** Find the `ISSUED` line for that user and book in `issues.txt`, change to `RETURNED`, mark book available.
- **Why `%59[^|]` in sscanf?** Reads text up to the `|` separator and never more than 59 characters (prevents overflow).

## Member 2 (search.c, main.cpp)
- **Hash function?** `id % 101`. Collisions handled by chaining (a small list per slot). Average lookup O(1).
- **Why is BST compared ignoring case?** So `HARRY` and `harry` match. In-order walk gives A-Z order.
- **BST weakness?** Not balanced: titles inserted in sorted order make it a chain (O(n)). Fix: AVL tree. (Sample data is deliberately unsorted.)
- **Why insertion sort?** Simple, stable, fine for hundreds of books; O(n^2) for big data.
- **Why do hash table and BST store pointers?** No duplicate data; an issue/return updates the book everywhere. They are rebuilt on add/delete/load.

## Member 3 (oop.cpp)
- **Where is inheritance?** `Student` and `Admin` inherit `User`; `GenreRule` and `AuthorRule` inherit `Rule`.
- **Where is polymorphism?** `user->menu()` runs the Student or Admin menu; `rules[r]->score(book)` runs the right rule.
- **Why a virtual destructor?** So `delete` on a base pointer cleans the child object correctly.
- **How does recommendation score?** Genre +2 per matching past book, author +3 per matching past book. Already-read books excluded, only scores above 0 shown, top 5.
- **New user with no history?** Recommendation is based on reading history, so the program asks them to issue a few books first.
- **Weakness?** Ignores book content; weak with very little history.
