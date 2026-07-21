#include <iostream>
#include <fstream>
#include <string>
#include "sqlite3.h"
using namespace std;

int main() {
    sqlite3* db;
    char* errMsg;

    int result = sqlite3_open("judge.db", &db);
    if(result != SQLITE_OK) {
        cout << "Failed to open database" << endl;
        return 1;
    }
    cout << "Database opened successfully" << endl;

    // users table
    sqlite3_exec(db,
        "CREATE TABLE IF NOT EXISTS users ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "username TEXT UNIQUE NOT NULL,"
        "password TEXT NOT NULL,"
        "created_at DATETIME DEFAULT CURRENT_TIMESTAMP);",
        NULL, NULL, &errMsg);
    cout << "Users table created" << endl;

    // sessions table
    sqlite3_exec(db,
        "CREATE TABLE IF NOT EXISTS sessions ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "user_id INTEGER NOT NULL,"
        "token TEXT UNIQUE NOT NULL,"
        "created_at DATETIME DEFAULT CURRENT_TIMESTAMP);",
        NULL, NULL, &errMsg);
    cout << "Sessions table created" << endl;

    // add user_id to submissions if not exists
    sqlite3_exec(db,
        "ALTER TABLE submissions ADD COLUMN "
        "user_id INTEGER DEFAULT 0;",
        NULL, NULL, &errMsg);
    // ignore error if column already exists
    cout << "Submissions table updated" << endl;

    // create problems table (no expected_output here anymore)
    sqlite3_exec(db,
        "CREATE TABLE IF NOT EXISTS problems ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "title TEXT NOT NULL,"
        "description TEXT NOT NULL,"
        "difficulty TEXT DEFAULT 'Easy',"
        "time_limit INTEGER DEFAULT 2);",
        NULL, NULL, &errMsg);
    cout << "Problems table created" << endl;

    // create test_cases table
    sqlite3_exec(db,
        "CREATE TABLE IF NOT EXISTS test_cases ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "problem_id INTEGER NOT NULL,"
        "input TEXT NOT NULL,"
        "expected_output TEXT NOT NULL);",
        NULL, NULL, &errMsg);
    cout << "Test cases table created" << endl;

    // create submissions table
    sqlite3_exec(db,
        "CREATE TABLE IF NOT EXISTS submissions ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "problem_id INTEGER NOT NULL,"
        "code TEXT NOT NULL,"
        "verdict TEXT NOT NULL,"
        "timestamp DATETIME DEFAULT CURRENT_TIMESTAMP);",
        NULL, NULL, &errMsg);
    cout << "Submissions table created" << endl;

    // create queue table
    sqlite3_exec(db,
        "CREATE TABLE IF NOT EXISTS queue ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "problem_id INTEGER NOT NULL,"
        "code TEXT NOT NULL,"
        "status TEXT DEFAULT 'pending',"
        "verdict TEXT DEFAULT '',"
        "timestamp DATETIME DEFAULT CURRENT_TIMESTAMP);",
        NULL, NULL, &errMsg);
    cout << "Queue table created" << endl;

    // ── Insert Problems ───────────────────────────────────────

    // existing problem 1
    sqlite3_exec(db,
        "INSERT INTO problems (title, description, difficulty, time_limit) "
        "SELECT 'Square a Number', "
        "'Given an integer N, print its square.', "
        "'Easy', 2 "
        "WHERE NOT EXISTS "
        "(SELECT 1 FROM problems WHERE title = 'Square a Number');",
        NULL, NULL, &errMsg);

    // existing problem 2
    sqlite3_exec(db,
        "INSERT INTO problems (title, description, difficulty, time_limit) "
        "SELECT 'Check Even or Odd', "
        "'Given an integer N, print Even if it is even, otherwise print Odd.', "
        "'Easy', 2 "
        "WHERE NOT EXISTS "
        "(SELECT 1 FROM problems WHERE title = 'Check Even or Odd');",
        NULL, NULL, &errMsg);

    // new problems
    sqlite3_exec(db,
        "INSERT INTO problems (title, description, difficulty, time_limit) "
        "SELECT 'Reverse a String', "
        "'Given a string, print it in reverse.', "
        "'Easy', 2 "
        "WHERE NOT EXISTS "
        "(SELECT 1 FROM problems WHERE title = 'Reverse a String');",
        NULL, NULL, &errMsg);

    sqlite3_exec(db,
        "INSERT INTO problems (title, description, difficulty, time_limit) "
        "SELECT 'Check Prime', "
        "'Given an integer N, print Prime if it is prime, otherwise print Not Prime.', "
        "'Easy', 2 "
        "WHERE NOT EXISTS "
        "(SELECT 1 FROM problems WHERE title = 'Check Prime');",
        NULL, NULL, &errMsg);

    sqlite3_exec(db,
        "INSERT INTO problems (title, description, difficulty, time_limit) "
        "SELECT 'Fibonacci Nth Term', "
        "'Given N, print the Nth Fibonacci number. Sequence starts 0 1 1 2 3 5 8...', "
        "'Easy', 2 "
        "WHERE NOT EXISTS "
        "(SELECT 1 FROM problems WHERE title = 'Fibonacci Nth Term');",
        NULL, NULL, &errMsg);

    sqlite3_exec(db,
        "INSERT INTO problems (title, description, difficulty, time_limit) "
        "SELECT 'Factorial', "
        "'Given an integer N, print its factorial.', "
        "'Easy', 2 "
        "WHERE NOT EXISTS "
        "(SELECT 1 FROM problems WHERE title = 'Factorial');",
        NULL, NULL, &errMsg);

    sqlite3_exec(db,
        "INSERT INTO problems (title, description, difficulty, time_limit) "
        "SELECT 'Palindrome Check', "
        "'Given an integer N, print Yes if it is a palindrome, otherwise print No.', "
        "'Easy', 2 "
        "WHERE NOT EXISTS "
        "(SELECT 1 FROM problems WHERE title = 'Palindrome Check');",
        NULL, NULL, &errMsg);

    sqlite3_exec(db,
        "INSERT INTO problems (title, description, difficulty, time_limit) "
        "SELECT 'Count Vowels', "
        "'Given a string, print the count of vowels in it.', "
        "'Easy', 2 "
        "WHERE NOT EXISTS "
        "(SELECT 1 FROM problems WHERE title = 'Count Vowels');",
        NULL, NULL, &errMsg);

    sqlite3_exec(db,
        "INSERT INTO problems (title, description, difficulty, time_limit) "
        "SELECT 'Find Maximum in Array', "
        "'First line contains N. Second line contains N integers. Print the maximum.', "
        "'Easy', 2 "
        "WHERE NOT EXISTS "
        "(SELECT 1 FROM problems WHERE title = 'Find Maximum in Array');",
        NULL, NULL, &errMsg);

    sqlite3_exec(db,
        "INSERT INTO problems (title, description, difficulty, time_limit) "
        "SELECT 'Sum of Digits', "
        "'Given an integer N, print the sum of its digits.', "
        "'Easy', 2 "
        "WHERE NOT EXISTS "
        "(SELECT 1 FROM problems WHERE title = 'Sum of Digits');",
        NULL, NULL, &errMsg);

    sqlite3_exec(db,
        "INSERT INTO problems (title, description, difficulty, time_limit) "
        "SELECT 'Armstrong Number', "
        "'Given an integer N, print Yes if it is an Armstrong number, otherwise No. "
        "An Armstrong number equals the sum of its digits each raised to the power "
        "of the number of digits. Example: 153 = 1^3 + 5^3 + 3^3.', "
        "'Easy', 2 "
        "WHERE NOT EXISTS "
        "(SELECT 1 FROM problems WHERE title = 'Armstrong Number');",
        NULL, NULL, &errMsg);

    cout << "Problems inserted" << endl;

    // ── Insert Test Cases ─────────────────────────────────────

    // Problem 1 - Square a Number
    sqlite3_exec(db,
        "INSERT INTO test_cases (problem_id, input, expected_output) "
        "SELECT 1, '5', '25' WHERE NOT EXISTS "
        "(SELECT 1 FROM test_cases WHERE problem_id=1 AND input='5');",
        NULL, NULL, &errMsg);
    sqlite3_exec(db,
        "INSERT INTO test_cases (problem_id, input, expected_output) "
        "SELECT 1, '3', '9' WHERE NOT EXISTS "
        "(SELECT 1 FROM test_cases WHERE problem_id=1 AND input='3');",
        NULL, NULL, &errMsg);
    sqlite3_exec(db,
        "INSERT INTO test_cases (problem_id, input, expected_output) "
        "SELECT 1, '10', '100' WHERE NOT EXISTS "
        "(SELECT 1 FROM test_cases WHERE problem_id=1 AND input='10');",
        NULL, NULL, &errMsg);

    // Problem 2 - Check Even or Odd
    sqlite3_exec(db,
        "INSERT INTO test_cases (problem_id, input, expected_output) "
        "SELECT 2, '4', 'Even' WHERE NOT EXISTS "
        "(SELECT 1 FROM test_cases WHERE problem_id=2 AND input='4');",
        NULL, NULL, &errMsg);
    sqlite3_exec(db,
        "INSERT INTO test_cases (problem_id, input, expected_output) "
        "SELECT 2, '7', 'Odd' WHERE NOT EXISTS "
        "(SELECT 1 FROM test_cases WHERE problem_id=2 AND input='7');",
        NULL, NULL, &errMsg);
    sqlite3_exec(db,
        "INSERT INTO test_cases (problem_id, input, expected_output) "
        "SELECT 2, '0', 'Even' WHERE NOT EXISTS "
        "(SELECT 1 FROM test_cases WHERE problem_id=2 AND input='0');",
        NULL, NULL, &errMsg);

    // Problem 3 - Reverse a String
    sqlite3_exec(db,
        "INSERT INTO test_cases (problem_id, input, expected_output) "
        "SELECT 3, 'hello', 'olleh' WHERE NOT EXISTS "
        "(SELECT 1 FROM test_cases WHERE problem_id=3 AND input='hello');",
        NULL, NULL, &errMsg);
    sqlite3_exec(db,
        "INSERT INTO test_cases (problem_id, input, expected_output) "
        "SELECT 3, 'world', 'dlrow' WHERE NOT EXISTS "
        "(SELECT 1 FROM test_cases WHERE problem_id=3 AND input='world');",
        NULL, NULL, &errMsg);
    sqlite3_exec(db,
        "INSERT INTO test_cases (problem_id, input, expected_output) "
        "SELECT 3, 'abcde', 'edcba' WHERE NOT EXISTS "
        "(SELECT 1 FROM test_cases WHERE problem_id=3 AND input='abcde');",
        NULL, NULL, &errMsg);

    // Problem 4 - Check Prime
    sqlite3_exec(db,
        "INSERT INTO test_cases (problem_id, input, expected_output) "
        "SELECT 4, '7', 'Prime' WHERE NOT EXISTS "
        "(SELECT 1 FROM test_cases WHERE problem_id=4 AND input='7');",
        NULL, NULL, &errMsg);
    sqlite3_exec(db,
        "INSERT INTO test_cases (problem_id, input, expected_output) "
        "SELECT 4, '4', 'Not Prime' WHERE NOT EXISTS "
        "(SELECT 1 FROM test_cases WHERE problem_id=4 AND input='4');",
        NULL, NULL, &errMsg);
    sqlite3_exec(db,
        "INSERT INTO test_cases (problem_id, input, expected_output) "
        "SELECT 4, '1', 'Not Prime' WHERE NOT EXISTS "
        "(SELECT 1 FROM test_cases WHERE problem_id=4 AND input='1');",
        NULL, NULL, &errMsg);

    // Problem 5 - Fibonacci Nth Term
    sqlite3_exec(db,
        "INSERT INTO test_cases (problem_id, input, expected_output) "
        "SELECT 5, '1', '0' WHERE NOT EXISTS "
        "(SELECT 1 FROM test_cases WHERE problem_id=5 AND input='1');",
        NULL, NULL, &errMsg);
    sqlite3_exec(db,
        "INSERT INTO test_cases (problem_id, input, expected_output) "
        "SELECT 5, '5', '3' WHERE NOT EXISTS "
        "(SELECT 1 FROM test_cases WHERE problem_id=5 AND input='5');",
        NULL, NULL, &errMsg);
    sqlite3_exec(db,
        "INSERT INTO test_cases (problem_id, input, expected_output) "
        "SELECT 5, '7', '8' WHERE NOT EXISTS "
        "(SELECT 1 FROM test_cases WHERE problem_id=5 AND input='7');",
        NULL, NULL, &errMsg);

    // Problem 6 - Factorial
    sqlite3_exec(db,
        "INSERT INTO test_cases (problem_id, input, expected_output) "
        "SELECT 6, '5', '120' WHERE NOT EXISTS "
        "(SELECT 1 FROM test_cases WHERE problem_id=6 AND input='5');",
        NULL, NULL, &errMsg);
    sqlite3_exec(db,
        "INSERT INTO test_cases (problem_id, input, expected_output) "
        "SELECT 6, '0', '1' WHERE NOT EXISTS "
        "(SELECT 1 FROM test_cases WHERE problem_id=6 AND input='0');",
        NULL, NULL, &errMsg);
    sqlite3_exec(db,
        "INSERT INTO test_cases (problem_id, input, expected_output) "
        "SELECT 6, '7', '5040' WHERE NOT EXISTS "
        "(SELECT 1 FROM test_cases WHERE problem_id=6 AND input='7');",
        NULL, NULL, &errMsg);

    // Problem 7 - Palindrome Check
    sqlite3_exec(db,
        "INSERT INTO test_cases (problem_id, input, expected_output) "
        "SELECT 7, '121', 'Yes' WHERE NOT EXISTS "
        "(SELECT 1 FROM test_cases WHERE problem_id=7 AND input='121');",
        NULL, NULL, &errMsg);
    sqlite3_exec(db,
        "INSERT INTO test_cases (problem_id, input, expected_output) "
        "SELECT 7, '123', 'No' WHERE NOT EXISTS "
        "(SELECT 1 FROM test_cases WHERE problem_id=7 AND input='123');",
        NULL, NULL, &errMsg);
    sqlite3_exec(db,
        "INSERT INTO test_cases (problem_id, input, expected_output) "
        "SELECT 7, '1001', 'Yes' WHERE NOT EXISTS "
        "(SELECT 1 FROM test_cases WHERE problem_id=7 AND input='1001');",
        NULL, NULL, &errMsg);

    // Problem 8 - Count Vowels
    sqlite3_exec(db,
        "INSERT INTO test_cases (problem_id, input, expected_output) "
        "SELECT 8, 'hello', '2' WHERE NOT EXISTS "
        "(SELECT 1 FROM test_cases WHERE problem_id=8 AND input='hello');",
        NULL, NULL, &errMsg);
    sqlite3_exec(db,
        "INSERT INTO test_cases (problem_id, input, expected_output) "
        "SELECT 8, 'aeiou', '5' WHERE NOT EXISTS "
        "(SELECT 1 FROM test_cases WHERE problem_id=8 AND input='aeiou');",
        NULL, NULL, &errMsg);
    sqlite3_exec(db,
        "INSERT INTO test_cases (problem_id, input, expected_output) "
        "SELECT 8, 'rhythm', '0' WHERE NOT EXISTS "
        "(SELECT 1 FROM test_cases WHERE problem_id=8 AND input='rhythm');",
        NULL, NULL, &errMsg);

    // Problem 9 - Find Maximum in Array
    sqlite3_exec(db,
        "INSERT INTO test_cases (problem_id, input, expected_output) "
        "SELECT 9, '5\n3 7 1 9 4', '9' WHERE NOT EXISTS "
        "(SELECT 1 FROM test_cases WHERE problem_id=9 AND expected_output='9');",
        NULL, NULL, &errMsg);
    sqlite3_exec(db,
        "INSERT INTO test_cases (problem_id, input, expected_output) "
        "SELECT 9, '3\n10 20 15', '20' WHERE NOT EXISTS "
        "(SELECT 1 FROM test_cases WHERE problem_id=9 AND expected_output='20');",
        NULL, NULL, &errMsg);
    sqlite3_exec(db,
        "INSERT INTO test_cases (problem_id, input, expected_output) "
        "SELECT 9, '4\n-1 -5 -2 -3', '-1' WHERE NOT EXISTS "
        "(SELECT 1 FROM test_cases WHERE problem_id=9 AND expected_output='-1');",
        NULL, NULL, &errMsg);

    // Problem 10 - Sum of Digits
    sqlite3_exec(db,
        "INSERT INTO test_cases (problem_id, input, expected_output) "
        "SELECT 10, '1234', '10' WHERE NOT EXISTS "
        "(SELECT 1 FROM test_cases WHERE problem_id=10 AND input='1234');",
        NULL, NULL, &errMsg);
    sqlite3_exec(db,
        "INSERT INTO test_cases (problem_id, input, expected_output) "
        "SELECT 10, '999', '27' WHERE NOT EXISTS "
        "(SELECT 1 FROM test_cases WHERE problem_id=10 AND input='999');",
        NULL, NULL, &errMsg);
    sqlite3_exec(db,
        "INSERT INTO test_cases (problem_id, input, expected_output) "
        "SELECT 10, '100', '1' WHERE NOT EXISTS "
        "(SELECT 1 FROM test_cases WHERE problem_id=10 AND input='100');",
        NULL, NULL, &errMsg);

    // Problem 11 - Armstrong Number
    sqlite3_exec(db,
        "INSERT INTO test_cases (problem_id, input, expected_output) "
        "SELECT 11, '153', 'Yes' WHERE NOT EXISTS "
        "(SELECT 1 FROM test_cases WHERE problem_id=11 AND input='153');",
        NULL, NULL, &errMsg);
    sqlite3_exec(db,
        "INSERT INTO test_cases (problem_id, input, expected_output) "
        "SELECT 11, '123', 'No' WHERE NOT EXISTS "
        "(SELECT 1 FROM test_cases WHERE problem_id=11 AND input='123');",
        NULL, NULL, &errMsg);
    sqlite3_exec(db,
        "INSERT INTO test_cases (problem_id, input, expected_output) "
        "SELECT 11, '370', 'Yes' WHERE NOT EXISTS "
        "(SELECT 1 FROM test_cases WHERE problem_id=11 AND input='370');",
        NULL, NULL, &errMsg);

    // Problem 12 - Two Sum
    sqlite3_exec(db,
        "INSERT INTO problems (title, description, difficulty, time_limit) "
        "SELECT 'Two Sum', "
        "'Given an array of N integers and a target T, find two numbers "
        "that add up to T. Print their indices (0-based), smaller index "
        "first. Exactly one solution is guaranteed.', "
        "'Medium', 2 "
        "WHERE NOT EXISTS "
        "(SELECT 1 FROM problems WHERE title = 'Two Sum');",
        NULL, NULL, &errMsg);

    // Problem 13 - Find Middle of Linked List
    sqlite3_exec(db,
        "INSERT INTO problems (title, description, difficulty, time_limit) "
        "SELECT 'Find Middle of Linked List', "
        "'Given N nodes of a linked list, find and print the value of the "
        "middle node. For even length lists, print the second middle node.', "
        "'Medium', 2 "
        "WHERE NOT EXISTS "
        "(SELECT 1 FROM problems WHERE title = "
        "'Find Middle of Linked List');",
        NULL, NULL, &errMsg);

    cout << "Medium problems inserted" << endl;

    // Test cases for Problem 12 - Two Sum
    sqlite3_exec(db,
        "INSERT INTO test_cases (problem_id, input, expected_output) "
        "SELECT 12, '4 9\n2 7 11 15', '0 1' WHERE NOT EXISTS "
        "(SELECT 1 FROM test_cases WHERE problem_id=12 "
        "AND expected_output='0 1');",
        NULL, NULL, &errMsg);

    sqlite3_exec(db,
        "INSERT INTO test_cases (problem_id, input, expected_output) "
        "SELECT 12, '4 6\n3 2 4 6', '1 2' WHERE NOT EXISTS "
        "(SELECT 1 FROM test_cases WHERE problem_id=12 "
        "AND expected_output='1 2');",
        NULL, NULL, &errMsg);

    sqlite3_exec(db,
        "INSERT INTO test_cases (problem_id, input, expected_output) "
        "SELECT 12, '3 6\n3 3 4', '0 1' WHERE NOT EXISTS "
        "(SELECT 1 FROM test_cases WHERE problem_id=12 "
        "AND expected_output='0 1' AND input='3 6\n3 3 4');",
        NULL, NULL, &errMsg);

    sqlite3_exec(db,
        "INSERT INTO test_cases (problem_id, input, expected_output) "
        "SELECT 12, '5 10\n1 4 3 6 2', '1 3' WHERE NOT EXISTS "
        "(SELECT 1 FROM test_cases WHERE problem_id=12 "
        "AND expected_output='1 3');",
        NULL, NULL, &errMsg);

    // Test cases for Problem 13 - Find Middle of Linked List
    sqlite3_exec(db,
        "INSERT INTO test_cases (problem_id, input, expected_output) "
        "SELECT 13, '5\n1 2 3 4 5', '3' WHERE NOT EXISTS "
        "(SELECT 1 FROM test_cases WHERE problem_id=13 "
        "AND input='5\n1 2 3 4 5');",
        NULL, NULL, &errMsg);

    sqlite3_exec(db,
        "INSERT INTO test_cases (problem_id, input, expected_output) "
        "SELECT 13, '4\n1 2 3 4', '3' WHERE NOT EXISTS "
        "(SELECT 1 FROM test_cases WHERE problem_id=13 "
        "AND input='4\n1 2 3 4');",
        NULL, NULL, &errMsg);

    sqlite3_exec(db,
        "INSERT INTO test_cases (problem_id, input, expected_output) "
        "SELECT 13, '1\n5', '5' WHERE NOT EXISTS "
        "(SELECT 1 FROM test_cases WHERE problem_id=13 "
        "AND input='1\n5');",
        NULL, NULL, &errMsg);

    sqlite3_exec(db,
        "INSERT INTO test_cases (problem_id, input, expected_output) "
        "SELECT 13, '6\n1 2 3 4 5 6', '4' WHERE NOT EXISTS "
        "(SELECT 1 FROM test_cases WHERE problem_id=13 "
        "AND input='6\n1 2 3 4 5 6');",
        NULL, NULL, &errMsg);

    cout << "Medium test cases inserted" << endl;

    // Problem 14 - Number of Islands
    sqlite3_exec(db,
        "INSERT INTO problems (title, description, difficulty, time_limit) "
        "SELECT 'Number of Islands', "
        "'Given a grid of R rows and C columns containing 0s and 1s, "
        "count the number of islands. An island is a group of connected "
        "1s (horizontally or vertically adjacent).', "
        "'Hard', 2 "
        "WHERE NOT EXISTS "
        "(SELECT 1 FROM problems WHERE title = 'Number of Islands');",
        NULL, NULL, &errMsg);

    // Problem 15 - Shortest Path in Binary Grid
    sqlite3_exec(db,
        "INSERT INTO problems (title, description, difficulty, time_limit) "
        "SELECT 'Shortest Path in Binary Grid', "
        "'Given an N x N grid of 0s and 1s, find the shortest path from "
        "top-left (0,0) to bottom-right (N-1,N-1). Move in 8 directions. "
        "0 = free, 1 = blocked. Print path length or -1 if impossible.', "
        "'Hard', 2 "
        "WHERE NOT EXISTS "
        "(SELECT 1 FROM problems WHERE title = "
        "'Shortest Path in Binary Grid');",
        NULL, NULL, &errMsg);

    cout << "Hard problems inserted" << endl;

    // Test cases for Problem 14 - Number of Islands
    sqlite3_exec(db,
        "INSERT INTO test_cases (problem_id, input, expected_output) "
        "SELECT 14, '4 5\n1 1 0 0 0\n1 1 0 0 0\n0 0 1 0 0\n0 0 0 1 1', '3' "
        "WHERE NOT EXISTS "
        "(SELECT 1 FROM test_cases WHERE problem_id=14 "
        "AND expected_output='3');",
        NULL, NULL, &errMsg);

    sqlite3_exec(db,
        "INSERT INTO test_cases (problem_id, input, expected_output) "
        "SELECT 14, '3 3\n1 1 1\n0 1 0\n1 1 1', '1' "
        "WHERE NOT EXISTS "
        "(SELECT 1 FROM test_cases WHERE problem_id=14 "
        "AND expected_output='1');",
        NULL, NULL, &errMsg);

    sqlite3_exec(db,
        "INSERT INTO test_cases (problem_id, input, expected_output) "
        "SELECT 14, '3 3\n1 0 1\n0 0 0\n1 0 1', '4' "
        "WHERE NOT EXISTS "
        "(SELECT 1 FROM test_cases WHERE problem_id=14 "
        "AND expected_output='4');",
        NULL, NULL, &errMsg);

    sqlite3_exec(db,
        "INSERT INTO test_cases (problem_id, input, expected_output) "
        "SELECT 14, '2 2\n0 0\n0 0', '0' "
        "WHERE NOT EXISTS "
        "(SELECT 1 FROM test_cases WHERE problem_id=14 "
        "AND expected_output='0');",
        NULL, NULL, &errMsg);

    // Test cases for Problem 15 - Shortest Path in Binary Grid
    sqlite3_exec(db,
        "INSERT INTO test_cases (problem_id, input, expected_output) "
        "SELECT 15, '3\n0 0 0\n1 1 0\n1 1 0', '4' "
        "WHERE NOT EXISTS "
        "(SELECT 1 FROM test_cases WHERE problem_id=15 "
        "AND expected_output='4');",
        NULL, NULL, &errMsg);

    sqlite3_exec(db,
        "INSERT INTO test_cases (problem_id, input, expected_output) "
        "SELECT 15, '3\n0 1 0\n1 1 0\n1 1 0', '-1' "
        "WHERE NOT EXISTS "
        "(SELECT 1 FROM test_cases WHERE problem_id=15 "
        "AND expected_output='-1');",
        NULL, NULL, &errMsg);

    sqlite3_exec(db,
        "INSERT INTO test_cases (problem_id, input, expected_output) "
        "SELECT 15, '1\n0', '1' "
        "WHERE NOT EXISTS "
        "(SELECT 1 FROM test_cases WHERE problem_id=15 "
        "AND expected_output='1');",
        NULL, NULL, &errMsg);

    sqlite3_exec(db,
        "INSERT INTO test_cases (problem_id, input, expected_output) "
        "SELECT 15, '4\n0 0 0 0\n1 1 1 0\n1 1 1 0\n1 1 1 0', '6' "
        "WHERE NOT EXISTS "
        "(SELECT 1 FROM test_cases WHERE problem_id=15 "
        "AND expected_output='6');",
        NULL, NULL, &errMsg);

    cout << "Hard test cases inserted" << endl;

    cout << "Test cases inserted" << endl;

    // verify problems
    cout << "\n--- Problems ---" << endl;
    sqlite3_exec(db,
        "SELECT id, title, difficulty FROM problems;",
        [](void*, int, char** data, char**) -> int {
            cout << "ID: " << data[0]
                 << " | Title: " << data[1]
                 << " | Difficulty: " << data[2] << endl;
            return 0;
        }, NULL, &errMsg);

    // verify test cases
    cout << "\n--- Test Cases ---" << endl;
    sqlite3_exec(db,
        "SELECT id, problem_id, input, expected_output FROM test_cases;",
        [](void*, int, char** data, char**) -> int {
            cout << "TC ID: " << data[0]
                 << " | Problem: " << data[1]
                 << " | Input: " << data[2]
                 << " | Expected: " << data[3] << endl;
            return 0;
        }, NULL, &errMsg);

    // submission statistics
    cout << "\n--- Submission Statistics ---" << endl;
    sqlite3_exec(db,
        "SELECT "
        "p.title, "
        "COUNT(s.id) as total, "
        "SUM(CASE WHEN s.verdict = 'Accepted' THEN 1 ELSE 0 END) as accepted, "
        "ROUND(SUM(CASE WHEN s.verdict = 'Accepted' THEN 1 ELSE 0 END) * 100.0 "
        "/ COUNT(s.id), 1) as acceptance_rate "
        "FROM problems p "
        "LEFT JOIN submissions s ON p.id = s.problem_id "
        "GROUP BY p.id;",
        [](void*, int, char** data, char**) -> int {
            cout << "Problem: "         << data[0] << endl;
            cout << "Total Submissions: "<< data[1] << endl;
            cout << "Accepted: "        << data[2] << endl;
            cout << "Acceptance Rate: " << data[3] << "%" << endl;
            cout << "---" << endl;
            return 0;
        }, NULL, &errMsg);        

    sqlite3_close(db);
    cout << "\nDone" << endl;
    return 0;
}
