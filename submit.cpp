#include <iostream>
#include <fstream>
#include <string>
#include "sqlite3.h"
using namespace std;

string readFile(string filename) {
    ifstream file(filename);
    string content, line;
    while(getline(file, line)) {
        content += line + "\n";
    }
    file.close();
    return content;
}

int main() {
    sqlite3* db;
    sqlite3_open("judge.db", &db);

    // read the submitted code from solution.cpp
    string code = readFile("solution.cpp");
    
    if(code.empty()) {
        cout << "Error: solution.cpp is empty or not found" << endl;
        return 1;
    }

    int problemId = 1; // hardcoded for now

    // escape single quotes in code to avoid SQL errors
    string escapedCode = "";
    for(char c : code) {
        if(c == '\'') escapedCode += "''";
        else escapedCode += c;
    }

    string query = 
        "INSERT INTO queue (problem_id, code, status) VALUES ("
        + to_string(problemId) + ", '"
        + escapedCode + "', 'pending');";

    char* errMsg;
    int result = sqlite3_exec(db, query.c_str(), NULL, NULL, &errMsg);

    if(result != SQLITE_OK) {
        cout << "Error adding to queue: " << errMsg << endl;
        sqlite3_free(errMsg);
        return 1;
    }

    cout << "Submission added to queue successfully" << endl;
    cout << "Code submitted: " << endl << code << endl;

    sqlite3_close(db);
    return 0;
}