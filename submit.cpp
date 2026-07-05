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

int main(int argc, char* argv[]) {
    // get problem_id from command line argument
    if(argc < 2) {
        cout << "Usage: submit.exe <problem_id>" << endl;
        return 1;
    }

    int problemId = atoi(argv[1]);

    sqlite3* db;
    sqlite3_open("judge.db", &db);

    string code = readFile("solution.cpp");

    if(code.empty()) {
        cout << "Error: solution.cpp is empty or not found" << endl;
        return 1;
    }

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
    int result = sqlite3_exec(db, query.c_str(),
                             NULL, NULL, &errMsg);

    if(result != SQLITE_OK) {
        cout << "Error adding to queue: " << errMsg << endl;
        sqlite3_free(errMsg);
        return 1;
    }

    cout << "Submission added to queue for problem "
         << problemId << endl;

    sqlite3_close(db);
    return 0;
}
