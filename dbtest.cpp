#include <iostream>
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

    // create problems table
    sqlite3_exec(db,
        "CREATE TABLE IF NOT EXISTS problems ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "title TEXT NOT NULL,"
        "description TEXT NOT NULL,"
        "expected_output TEXT NOT NULL,"
        "time_limit INTEGER DEFAULT 2);",
        NULL, NULL, &errMsg);
    cout << "Problems table created" << endl;

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

    // insert only if empty
    sqlite3_exec(db,
        "INSERT INTO problems (title, description, expected_output, time_limit) "
        "SELECT 'Square a Number', 'Read an integer and print its square', '25', 2 "
        "WHERE NOT EXISTS (SELECT 1 FROM problems WHERE title = 'Square a Number');",
        NULL, NULL, &errMsg);
    cout << "Problem inserted" << endl;

    // read back
    sqlite3_exec(db,
        "SELECT id, title, expected_output FROM problems;",
        [](void*, int cols, char** data, char**) -> int {
            cout << "ID: " << data[0] << endl;
            cout << "Title: " << data[1] << endl;
            cout << "Expected Output: " << data[2] << endl;
            return 0;
        }, NULL, &errMsg);

        // read back submissions
        cout << "\n--- Submissions ---" << endl;
        sqlite3_exec(db,
            "SELECT id, problem_id, verdict, timestamp FROM submissions;",
            [](void*, int cols, char** data, char**) -> int {
                cout << "Submission ID: " << data[0] 
                    << " | Problem: " << data[1] 
                    << " | Verdict: " << data[2] 
                    << " | Time: " << data[3] << endl;
                return 0;
            }, NULL, &errMsg);

    sqlite3_close(db);
    cout << "Done" << endl;
    return 0;
}