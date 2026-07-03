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

    // ── Job Object for memory limit ───────────────────────

    // create a job object
    HANDLE hJob = CreateJobObject(NULL, NULL);

    if(hJob != NULL) {
        // set memory limit — 256MB
        JOBOBJECT_EXTENDED_LIMIT_INFORMATION jeli;
        ZeroMemory(&jeli, sizeof(jeli));

        jeli.BasicLimitInformation.LimitFlags =
            JOB_OBJECT_LIMIT_PROCESS_MEMORY;

        // 256MB in bytes
        jeli.ProcessMemoryLimit = 256 * 1024 * 1024;

        SetInformationJobObject(
            hJob,
            JobObjectExtendedLimitInformation,
            &jeli,
            sizeof(jeli)
        );

        // assign solution process to job
        AssignProcessToJobObject(hJob, pi.hProcess);
    }

    // ── time limit check (same as before) ─────────────────

    DWORD result = WaitForSingleObject(pi.hProcess,
                                       timeLimit * 1000);

    if(result == WAIT_TIMEOUT) {
        TerminateProcess(pi.hProcess, 0);
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
        if(hJob) CloseHandle(hJob);
        return "Time Limit Exceeded";
    }

    // ── check if process was killed by memory limit ────────

    DWORD exitCode;
    GetExitCodeProcess(pi.hProcess, &exitCode);

    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    if(hJob) CloseHandle(hJob);

    // exit code 1 with no timeout = likely memory limit hit
    // Windows kills the process when memory limit exceeded
    if(exitCode != 0) {
        // check if it was a memory issue by trying to
        // read output — if empty and non-zero exit, MLE
        string outStr = readFile("output.txt");
        if(trim(outStr).empty()) {
            return "Memory Limit Exceeded";
        }
    }

    // ── compare output ─────────────────────────────────────

    string outStr  = readFile("output.txt");
    string expStr  = readFile("expected.txt");

    if(trim(outStr) == trim(expStr)) return "Accepted";
    return "Wrong Answer";
}

// ── judge all test cases ──────────────────────────────────

string compileError = "";

string judgeSubmission(sqlite3* db, string code, int problemId) {
    writeFile("solution.cpp", code);
    system("taskkill /F /IM solution.exe >nul 2>&1");

    // use full path so redirection works regardless of directory
    int compileResult = system(
        "cmd.exe /c \"cd C:\\Users\\HP\\.vscode\\Code^ Judge "
        "&& g++ solution.cpp -o solution.exe "
        "2>compile_error.txt\"");

    if(compileResult != 0) {
        compileError = readFile(
            "C:\\Users\\HP\\.vscode\\Code Judge\\compile_error.txt");
        cout << "Compile error: " << compileError << endl;
        return "Compilation Error";
    }

    compileError = "";
    loadTestCases(db, problemId);

    if(testCases.empty()) return "No Test Cases Found";

    cout << "  Running " << testCases.size()
         << " test case(s)..." << endl;

    for(int i = 0; i < testCases.size(); i++) {
        cout << "  Test case " << i + 1 << ": ";
        string verdict = runTestCase(
            testCases[i].input,
            testCases[i].expectedOutput
        );
        cout << verdict << endl;
        if(verdict != "Accepted") return verdict;
    }

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

    // escape compile error message too
    string escapedError = "";
    for(char c : compileError) {
        if(c == '\'') escapedError += "''";
        else escapedError += c;
    }

    // store verdict + error message together
    string fullVerdict = verdict;
    if(verdict == "Compilation Error" && !compileError.empty()) {
        fullVerdict = "Compilation Error: " + compileError;
    }

    // escape the full verdict
    string escapedVerdict = "";
    for(char c : fullVerdict) {
        if(c == '\'') escapedVerdict += "''";
        else escapedVerdict += c;
    }

    string query =
        "INSERT INTO submissions (problem_id, code, verdict) "
        "VALUES (" + to_string(problemId) + ", '"
        + escaped + "', '" + escapedVerdict + "');";
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
