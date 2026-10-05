#!/bin/bash
# ====================================================================
# run_tests.sh   OWNER: Member 2 (Testing)
# Feeds scripted keyboard input to ./library and checks the output text.
# Every test starts from fresh sample data (data_sample/).
# Usage:  make  &&  bash run_tests.sh
# ====================================================================
PASS=0; FAIL=0
clean() {   # remove menus/prompts so only the results remain
  sed -E 's/(Choice|Username|Password|New student username|Book ID \(number\)|Title|Author|Genre): /\n/g; s/Enter [^:]*: /\n/g' \
  | grep -vE '^[0-9]\. |^=====|^$|^-{5,}$|^Welcome.*\)$'
}
# run NAME INPUT EXPECTED1 [EXPECTED2 ...]   (each EXPECTED text must appear in order-free way)
run() {
  local name="$1" input="$2"; shift 2
  rm -rf data && cp -r data_sample data
  local out; out=$(printf "$input" | ./library | clean)
  echo "=== $name"; echo "$out"
  local ok=1
  for e in "$@"; do echo "$out" | grep -qF -- "$e" || { ok=0; echo "   MISSING: $e"; }; done
  if [ $ok = 1 ]; then echo ">>> PASS"; PASS=$((PASS+1)); else echo ">>> FAIL"; FAIL=$((FAIL+1)); fi
  echo
}
notin() {   # notin NAME INPUT UNWANTED  -> passes when the text is NOT in the output
  local name="$1" input="$2" bad="$3"
  rm -rf data && cp -r data_sample data
  local out; out=$(printf "$input" | ./library | clean)
  if echo "$out" | grep -qF -- "$bad"; then echo "=== $name"; echo ">>> FAIL (found: $bad)"; FAIL=$((FAIL+1));
  else echo "=== $name"; echo ">>> PASS (not present: $bad)"; PASS=$((PASS+1)); fi; echo
}

run "T1 wrong login rejected"                "1\nhacker\nbad\n0\n"                        "Wrong username or password."
run "T2 BST title search: exact, partial, none" "1\npriya\npriya123\n1\nthe hobbit\n1\nharry\n1\nxyz\n0\n0\n" "203   The Hobbit" "202   Harry Potter and the Chamber of Secrets" "201   Harry Potter and the Sorcerer's Stone" "No book found"
run "T3 hashing search by ID"                "1\npriya\npriya123\n2\n105\n2\n999\n0\n0\n"  "105   Object Oriented Programming with C++" "No book with ID 999"
run "T4a sort by title (first = A Brief History)"  "1\npriya\npriya123\n3\n1\n0\n0\n"     "501   A Brief History of Time"
run "T4b sort by author (first = A P J Abdul Kalam)" "1\npriya\npriya123\n3\n2\n0\n0\n"  "301   Wings of Fire"
run "T4c sort by popularity (issued 3 times first)" "1\npriya\npriya123\n3\n3\n0\n0\n"    "201   Harry Potter and the Sorcerer's Stone"
run "T5 issue / issue again / wrong id / return / return again" "1\nrahul\nrahul123\n4\n201\n4\n201\n4\n999\n5\n201\n5\n201\n0\n0\n" "Book issued successfully." "already issued" "No book with ID 999" "Book returned successfully." "not issued"
run "T6 recommendation (genre + author) for priya" "1\npriya\npriya123\n7\n0\n0\n"        "202   Harry Potter and the Chamber of Secrets  J K Rowling          Fantasy      7" "204   The Lord of the Rings                    J R R Tolkien        Fantasy      7" "101   The C Programming Language" "genre and author"
notin "T6b recommendation never shows already-read books" "1\npriya\npriya123\n7\n0\n0\n" "203   The Hobbit"
run "T7 new student with no history gets a clear message" "1\nadmin\nadmin123\n7\nnewbie\nnew123\n0\n1\nnewbie\nnew123\n7\n0\n0\n" "Student account created." "No reading history yet"
run "T8 admin add / duplicate / delete issued / delete free" "1\nadmin\nadmin123\n1\n601\nClean Code\nRobert Martin\nProgramming\n1\n601\nX\nY\nZ\n2\n105\n2\n601\n0\n0\n" "Book added." "already exists" "currently issued" "Book deleted."
run "T9 admin issued report + duplicate username" "1\nadmin\nadmin123\n6\n7\npriya\nabc\n0\n0\n" "priya        102" "rahul        105" "username already exists"

run "T11 invalid menu choice does not crash"  "1\nadmin\nadmin123\n99\nabc\n0\n0\n"        "Invalid choice." "Goodbye"

run "T13 very long search word does not crash" "1\npriya\npriya123\n1\n$(printf 'a%.0s' $(seq 1 400))\n0\n0\n" "No book found"

# T14: data files with Windows line endings (CRLF) still work
rm -rf data && cp -r data_sample data
sed -i 's/$/\r/' data/books.txt data/users.txt data/issues.txt
out=$(printf "1\npriya\npriya123\n7\n0\n0\n" | ./library | clean)
echo "=== T14 Windows (CRLF) data files"
if echo "$out" | grep -qF "204   The Lord of the Rings"; then echo ">>> PASS"; PASS=$((PASS+1)); else echo "$out"; echo ">>> FAIL"; FAIL=$((FAIL+1)); fi; echo

# T15: missing books file -> program starts with an empty library, no crash
rm -rf data && cp -r data_sample data && rm data/books.txt
out=$(printf "1\nadmin\nadmin123\n3\n1\n0\n0\n" | ./library | clean)
echo "=== T15 missing books.txt"
if echo "$out" | grep -qF "starting with an empty library" && echo "$out" | grep -qF "No books in the library."; then echo ">>> PASS"; PASS=$((PASS+1)); else echo "$out"; echo ">>> FAIL"; FAIL=$((FAIL+1)); fi; echo

# T12: data really saved in the files
rm -rf data && cp -r data_sample data
printf "1\nrahul\nrahul123\n4\n201\n0\n0\n" | ./library > /dev/null
if grep -q '^201|.*|0|4$' data/books.txt && grep -q '^rahul|201|ISSUED$' data/issues.txt; then
  echo "=== T12 data saved to files"; echo ">>> PASS"; PASS=$((PASS+1)); else echo "=== T12 data saved to files"; echo ">>> FAIL"; FAIL=$((FAIL+1)); fi
rm -rf data && cp -r data_sample data
echo; echo "RESULT: $PASS passed, $FAIL failed"
[ $FAIL = 0 ]
