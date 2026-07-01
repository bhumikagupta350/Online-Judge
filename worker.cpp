#include <iostream>
#include <windows.h>
#include <fstream>
#include <string>
#include "sqlite3.h"
using namespace std;

string trim(string s) {
    int start = s.find_first_not_of(" \t\r\n");
    int end = s.find_last_not_of(" \t\r\n");
    if(start == string::npos) return "";
    return s.substr(start, end - start + 1);
}

// ── helpers ──────────────────────────────────────────────

void writeFile(string filename, string content) {
    ofstream f(filename);
    f << content;
    f.close();
}

string readFile(string filename) {
    ifstream f(filename);
    string content, line;
    while(getline(f, line)) content += line + "\n";
    f.close();
    return content;
}

// ── judge one submission ──────────────────────────────────

string judgeSubmission(string code, string expectedOutput, int timeLimit) {
    // write code to solution.cpp
    writeFile("solution.cpp", code);

    // write expected output
    writeFile("expected.txt", expectedOutput);

    // kill any stale process
    system("taskkill /F /IM solution.exe >nul 2>&1");

    // compile
    int compileResult = system("g++ solution.cpp -o solution.exe >nul 2>&1");
    if(compileResult != 0) return "Compilation Error";

    // run with time limit
    STARTUPINFOA si;
    PROCESS_INFORMATION pi;
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);

    char cmd[] = "cmd.exe /c solution.exe < input.txt > output.txt";
    char workingDir[] = "C:\\Users\\HP\\.vscode\\Code Judge";

    BOOL success = CreateProcessA(
        NULL, cmd, NULL, NULL,
        FALSE, 0, NULL,
        workingDir, &si, &pi
    );

    if(!success) return "Runtime Error";

    DWORD result = WaitForSingleObject(pi.hProcess, timeLimit * 1000);

    if(result == WAIT_TIMEOUT) {
        TerminateProcess(pi.hProcess, 0);
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
        return "Time Limit Exceeded";
    }

    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);

    // compare output
    string outStr = readFile("output.txt");
    string expStr = readFile("expected.txt");

    if(trim(outStr) == trim(expStr)) return "Accepted";
    return "Wrong Answer";
}

// ── database helpers ─────────────────────────────────────

struct Submission {
    int id;
    int problemId;
    string code;
};

struct Problem {
    string expectedOutput;
    int timeLimit;
};

Submission pendingSubmission;
bool foundSubmission = false;

static int submissionCallback(void*, int, char** data, char**) {
    pendingSubmission.id        = atoi(data[0]);
    pendingSubmission.problemId = atoi(data[1]);
    pendingSubmission.code      = data[2];
    foundSubmission = true;
    return 0;
}

Problem currentProblem;

static int problemCallback(void*, int, char** data, char**) {
    currentProblem.expectedOutput = data[0];
    currentProblem.timeLimit      = atoi(data[1]);
    return 0;
}

void updateQueueStatus(sqlite3* db, int id, string status, string verdict) {
    string query =
        "UPDATE queue SET status = '" + status + "', "
        "verdict = '" + verdict + "' WHERE id = " + to_string(id) + ";";
    char* errMsg;
    sqlite3_exec(db, query.c_str(), NULL, NULL, &errMsg);
}

void saveToSubmissions(sqlite3* db, int problemId, string code, string verdict) {
    // escape single quotes in code
    string escaped = "";
    for(char c : code) {
        if(c == '\'') escaped += "''";
        else escaped += c;
    }
    string query =
        "INSERT INTO submissions (problem_id, code, verdict) VALUES ("
        + to_string(problemId) + ", '" + escaped + "', '" + verdict + "');";
    char* errMsg;
    sqlite3_exec(db, query.c_str(), NULL, NULL, &errMsg);
}

// ── main loop ────────────────────────────────────────────

int main() {
    sqlite3* db;
    sqlite3_open("judge.db", &db);

    cout << "Worker started. Waiting for submissions..." << endl;

    while(true) {
        // pick oldest pending submission
        foundSubmission = false;

        sqlite3_exec(db,
            "SELECT id, problem_id, code FROM queue "
            "WHERE status = 'pending' "
            "ORDER BY timestamp ASC LIMIT 1;",
            submissionCallback, NULL, NULL);

        if(foundSubmission) {
            cout << "\nPickup up submission ID: " 
                 << pendingSubmission.id << endl;

            // mark as processing
            updateQueueStatus(db, pendingSubmission.id, 
                            "processing", "");

            // load problem details
            string query =
                "SELECT expected_output, time_limit FROM problems "
                "WHERE id = " + 
                to_string(pendingSubmission.problemId) + ";";
            char* errMsg;
            sqlite3_exec(db, query.c_str(), 
                        problemCallback, NULL, &errMsg);

            // judge it
            string verdict = judgeSubmission(
                pendingSubmission.code,
                currentProblem.expectedOutput,
                currentProblem.timeLimit
            );

            cout << "Verdict: " << verdict << endl;

            // mark as done in queue
            updateQueueStatus(db, pendingSubmission.id, 
                            "done", verdict);

            // save to submissions table
            saveToSubmissions(db, pendingSubmission.problemId,
                            pendingSubmission.code, verdict);

            cout << "Submission " << pendingSubmission.id 
                 << " done. Waiting for next..." << endl;
        }

        // wait 1 second before checking again
        Sleep(1000);
    }

    sqlite3_close(db);
    return 0;
}