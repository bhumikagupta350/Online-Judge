#include <iostream>
#include <windows.h>
#include <fstream>
#include <string>
#include <vector>
#include "sqlite3.h"
using namespace std;

string trim(string s) {
    int start = s.find_first_not_of(" \t\r\n");
    int end = s.find_last_not_of(" \t\r\n");
    if(start == string::npos) return "";
    return s.substr(start, end - start + 1);
}

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

// ── test case struct ─────────────────────────────────────

struct TestCase {
    string input;
    string expectedOutput;
};

// ── load test cases from database ────────────────────────

vector<TestCase> testCases;

static int testCaseCallback(void*, int, char** data, char**) {
    TestCase tc;
    tc.input          = data[0];
    tc.expectedOutput = data[1];
    testCases.push_back(tc);
    return 0;
}

void loadTestCases(sqlite3* db, int problemId) {
    testCases.clear();
    string query =
        "SELECT input, expected_output FROM test_cases "
        "WHERE problem_id = " + to_string(problemId) +
        " ORDER BY id ASC;";
    char* errMsg;
    sqlite3_exec(db, query.c_str(), testCaseCallback, NULL, &errMsg);
}

// ── load time limit ───────────────────────────────────────

int timeLimit = 2;

static int problemCallback(void*, int, char** data, char**) {
    timeLimit = atoi(data[0]);
    return 0;
}

void loadProblem(sqlite3* db, int problemId) {
    string query =
        "SELECT time_limit FROM problems WHERE id = " +
        to_string(problemId) + ";";
    char* errMsg;
    sqlite3_exec(db, query.c_str(), problemCallback, NULL, &errMsg);
}

// ── run one test case ─────────────────────────────────────

string runTestCase(string input, string expectedOutput) {
    writeFile("input.txt", input);
    writeFile("expected.txt", expectedOutput);

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

    string outStr  = readFile("output.txt");
    string expStr  = readFile("expected.txt");

    if(trim(outStr) == trim(expStr)) return "Accepted";
    return "Wrong Answer";
}

// ── judge all test cases ──────────────────────────────────

string judgeSubmission(sqlite3* db, string code, int problemId) {
    // write code to file
    writeFile("solution.cpp", code);

    // kill stale process
    system("taskkill /F /IM solution.exe >nul 2>&1");

    // compile once
    int compileResult = system(
        "g++ solution.cpp -o solution.exe >nul 2>&1");
    if(compileResult != 0) return "Compilation Error";

    // load test cases
    loadTestCases(db, problemId);

    if(testCases.empty()) {
        return "No Test Cases Found";
    }

    cout << "  Running " << testCases.size()
         << " test case(s)..." << endl;

    // run each test case
    for(int i = 0; i < testCases.size(); i++) {
        cout << "  Test case " << i + 1 << ": ";

        string verdict = runTestCase(
            testCases[i].input,
            testCases[i].expectedOutput
        );

        cout << verdict << endl;

        // first failure stops everything
        if(verdict != "Accepted") return verdict;
    }

    // all passed
    return "Accepted";
}

// ── database helpers ──────────────────────────────────────

struct Submission {
    int id;
    int problemId;
    string code;
};

Submission pendingSubmission;
bool foundSubmission = false;

static int submissionCallback(void*, int, char** data, char**) {
    pendingSubmission.id        = atoi(data[0]);
    pendingSubmission.problemId = atoi(data[1]);
    pendingSubmission.code      = data[2];
    foundSubmission             = true;
    return 0;
}

void updateQueueStatus(sqlite3* db, int id,
                       string status, string verdict) {
    string query =
        "UPDATE queue SET status = '" + status + "', "
        "verdict = '" + verdict + "' WHERE id = "
        + to_string(id) + ";";
    char* errMsg;
    sqlite3_exec(db, query.c_str(), NULL, NULL, &errMsg);
}

void saveToSubmissions(sqlite3* db, int problemId,
                       string code, string verdict) {
    string escaped = "";
    for(char c : code) {
        if(c == '\'') escaped += "''";
        else escaped += c;
    }
    string query =
        "INSERT INTO submissions (problem_id, code, verdict) "
        "VALUES (" + to_string(problemId) + ", '"
        + escaped + "', '" + verdict + "');";
    char* errMsg;
    sqlite3_exec(db, query.c_str(), NULL, NULL, &errMsg);
}

// ── main loop ─────────────────────────────────────────────

int main() {
    sqlite3* db;
    sqlite3_open("judge.db", &db);

    cout << "Worker started. Waiting for submissions..." << endl;

    while(true) {
        foundSubmission = false;

        sqlite3_exec(db,
            "SELECT id, problem_id, code FROM queue "
            "WHERE status = 'pending' "
            "ORDER BY timestamp ASC LIMIT 1;",
            submissionCallback, NULL, NULL);

        if(foundSubmission) {
            cout << "\nPickup up submission ID: "
                 << pendingSubmission.id << endl;

            updateQueueStatus(db, pendingSubmission.id,
                            "processing", "");

            loadProblem(db, pendingSubmission.problemId);

            string verdict = judgeSubmission(
                db,
                pendingSubmission.code,
                pendingSubmission.problemId
            );

            cout << "Final Verdict: " << verdict << endl;

            updateQueueStatus(db, pendingSubmission.id,
                            "done", verdict);
            saveToSubmissions(db, pendingSubmission.problemId,
                            pendingSubmission.code, verdict);

            cout << "Submission " << pendingSubmission.id
                 << " done. Waiting for next..." << endl;
        }

        Sleep(1000);
    }

    sqlite3_close(db);
    return 0;
}
