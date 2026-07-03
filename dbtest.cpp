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

    // insert problem 1
    sqlite3_exec(db,
        "INSERT INTO problems (title, description, difficulty, time_limit) "
        "SELECT 'Square a Number', "
        "'Given an integer N, print its square.', "
        "'Easy', 2 "
        "WHERE NOT EXISTS "
        "(SELECT 1 FROM problems WHERE title = 'Square a Number');",
        NULL, NULL, &errMsg);

    // insert problem 2
    sqlite3_exec(db,
        "INSERT INTO problems (title, description, difficulty, time_limit) "
        "SELECT 'Check Even or Odd', "
        "'Given an integer N, print Even if it is even, otherwise print Odd.', "
        "'Easy', 2 "
        "WHERE NOT EXISTS "
        "(SELECT 1 FROM problems WHERE title = 'Check Even or Odd');",
        NULL, NULL, &errMsg);
    cout << "Problems inserted" << endl;

    // insert test cases for problem 1 (Square a Number)
    // 3 test cases
    sqlite3_exec(db,
        "INSERT INTO test_cases (problem_id, input, expected_output) "
        "SELECT 1, '5', '25' "
        "WHERE NOT EXISTS "
        "(SELECT 1 FROM test_cases WHERE problem_id = 1 AND input = '5');",
        NULL, NULL, &errMsg);

    sqlite3_exec(db,
        "INSERT INTO test_cases (problem_id, input, expected_output) "
        "SELECT 1, '3', '9' "
        "WHERE NOT EXISTS "
        "(SELECT 1 FROM test_cases WHERE problem_id = 1 AND input = '3');",
        NULL, NULL, &errMsg);

    sqlite3_exec(db,
        "INSERT INTO test_cases (problem_id, input, expected_output) "
        "SELECT 1, '10', '100' "
        "WHERE NOT EXISTS "
        "(SELECT 1 FROM test_cases WHERE problem_id = 1 AND input = '10');",
        NULL, NULL, &errMsg);

    // insert test cases for problem 2 (Even or Odd)
    // 3 test cases
    sqlite3_exec(db,
        "INSERT INTO test_cases (problem_id, input, expected_output) "
        "SELECT 2, '4', 'Even' "
        "WHERE NOT EXISTS "
        "(SELECT 1 FROM test_cases WHERE problem_id = 2 AND input = '4');",
        NULL, NULL, &errMsg);

    sqlite3_exec(db,
        "INSERT INTO test_cases (problem_id, input, expected_output) "
        "SELECT 2, '7', 'Odd' "
        "WHERE NOT EXISTS "
        "(SELECT 1 FROM test_cases WHERE problem_id = 2 AND input = '7');",
        NULL, NULL, &errMsg);

    sqlite3_exec(db,
        "INSERT INTO test_cases (problem_id, input, expected_output) "
        "SELECT 2, '0', 'Even' "
        "WHERE NOT EXISTS "
        "(SELECT 1 FROM test_cases WHERE problem_id = 2 AND input = '0');",
        NULL, NULL, &errMsg);

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
