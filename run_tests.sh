#!/bin/bash
# Runs scripted test cases against ./library. Each test starts from fresh sample data.
# Usage: bash run_tests.sh
run() {   # $1 = test name, $2 = input text
  rm -rf data && cp -r data_sample data
  echo "=================================================="
  echo "TEST: $1"
  echo "=================================================="
  printf "$2" | ./library | sed -E 's/(Choice|Username|Password|New student username|Book ID \(number\)|Title|Author|Genre): /\n/g; s/Enter [^:]*: /\n/g' | grep -vE '^[0-9]\. |^=====|^$|^-{5,}$|^Welcome.*\)$' 
}
run "T1 Wrong login rejected" "1\nhacker\nbad\n0\n"
run "T2 Student search by title (exact + partial, case-insensitive)" "1\npriya\npriya123\n1\nthe hobbit\n1\nharry\n1\nxyz\n0\n0\n"
run "T3 Student search by ID (hashing)" "1\npriya\npriya123\n2\n105\n2\n999\n0\n0\n"
run "T4 Sorting by title, author, popularity" "1\npriya\npriya123\n3\n1\n3\n2\n3\n3\n0\n0\n"
run "T5 Issue, issue-again, return flow" "1\nrahul\nrahul123\n4\n201\n4\n201\n4\n999\n5\n201\n5\n201\n0\n0\n"
run "T6 Recommendation for student with history (priya: Fantasy + Programming)" "1\npriya\npriya123\n6\n7\n0\n0\n"
run "T7 Recommendation for new student with no history (most popular)" "1\nadmin\nadmin123\n7\nnewbie\nnew123\n0\n1\nnewbie\nnew123\n7\n0\n0\n"
run "T8 Admin add book, duplicate id, delete issued book, delete free book" "1\nadmin\nadmin123\n1\n601\nClean Code\nRobert Martin\nProgramming\n1\n601\nX\nY\nZ\n2\n105\n2\n601\n3\n1\n0\n0\n"
run "T9 Admin views issued books and duplicate student username" "1\nadmin\nadmin123\n6\n7\npriya\nabc\n0\n0\n"
run "T10 Data saved in files (reload check)" "1\nrahul\nrahul123\n4\n201\n0\n0\n"
echo "--- books.txt line for 201 after T10 (should be 0|4) ---"; grep '^201' data/books.txt
echo "--- issues.txt after T10 ---"; cat data/issues.txt
rm -rf data && cp -r data_sample data
