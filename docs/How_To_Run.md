# How to run and see every feature, one by one

Run everything from the **project folder** (the one that has `src/`, `data/`, `Makefile`).

## Step 0 - Build and auto-check (1 command)
| Command | What you see |
|---|---|
| `make` | Builds `library` (no warnings = good) |
| `make test` | Runs 17 automatic tests; last line `RESULT: 17 passed, 0 failed` |
| `./library` | Starts the real program |

Reset the data any time: `cp data_sample/*.txt data/`

## Step 1 - Student features (login `priya` / `priya123`)
Start `./library`, type `1` (Login), then username and password. Type the number shown after "Choice:".

| Step | Type | What you should see | Code that runs |
|---|---|---|---|
| 1 | `1` | Search by title. Type `harry` | 2 Harry Potter books in A-Z order (BST walk, library.c PART 2) |
| 2 | `1`, `the hobbit` | Exact match, 1 row (BST find) | `findByTitle` |
| 3 | `2`, `105` | One book row (hash table). Try `999` -> "No book" | `findById` |
| 4 | `3`, `1` / `2` / `3` | All books sorted by title / author / popularity | `showSorted` (bubble sort) |
| 5 | `4`, `204` | "Book issued successfully." Do it again -> "already issued" | `issueBook` |
| 6 | `5`, `204` | "Book returned successfully." | `returnBook` |
| 7 | `6` | Your reading history table | `showUserHistory` |
| 8 | `7` | Recommendations: 204 and 202 (score 7), then 3 books with score 2 | `Recommender::recommend` |
| 9 | `0` | Logout | |

## Step 2 - Admin features (login `admin` / `admin123`)
| Type | What you should see |
|---|---|
| `1` | Add book: ID `300`, any title/author/genre -> "Book added." Same ID again -> "already exists" |
| `2`, `300` | Deleted. Delete an issued book (e.g. `105`) -> refused |
| `6` | List of books currently issued and who has them |
| `7` | Create student account (then log in with it; its recommendation says "No reading history yet") |

## Step 3 - See the data files change live
Open a second terminal in the project folder while the program runs:
```
cat data/books.txt      # after issuing: that book's "available" becomes 0, issued count +1
cat data/issues.txt     # new line  user|bookId|ISSUED
```

## Step 4 - Run one test at a time
`run_tests.sh` has a short test for each feature (T1 login ... T15 missing file). Run all with `make test`; for the full text of every test: `bash run_tests.sh` (saved copy: `docs/test_output.txt`).

## Step 5 - Follow the code while it runs (debug)
| Tool | How |
|---|---|
| Print | add `printf("DEBUG id=%d\n", id);` in `library.c`, `make`, run |
| gdb | `g++ -g` / `gcc -g`, then `gdb ./library`, `break issueBook`, `run`, `next`, `print id` |
| Code::Blocks | click left of a line number (red dot), press **F8**, step with **F7** |
More in `Debugging_Guide.md`.

## Where to look in the code for each step
| Feature | File / part |
|---|---|
| Linked list, files, issue/return | `library.c` PART 1 (Member 1) |
| Hash, BST, sorting | `library.c` PART 2 (Member 2) |
| Menus, login, recommender | `main.cpp` (Member 3) |
