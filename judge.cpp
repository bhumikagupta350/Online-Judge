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

string expectedOutput = "";
int timeLimit = 2;  //2 second time limit

static int problemCallback(void*, int cols, char** data, char**) {
    expectedOutput = data[0];
    timeLimit = atoi(data[1]);
    return 0;
}

void loadProblem(sqlite3* db, int problemId) {
    string query = "SELECT expected_output, time_limit FROM problems WHERE id = " 
                   + to_string(problemId) + ";";
    char* errMsg;
    sqlite3_exec(db, query.c_str(), problemCallback, NULL, &errMsg);
}

void writeExpectedFile() {
    ofstream exp("expected.txt");
    exp << expectedOutput;
    exp.close();
}

void saveSubmission(sqlite3* db, int problemId, string verdict) {
    string insertSubmission = 
        "INSERT INTO submissions (problem_id, code, verdict) VALUES ("
        + to_string(problemId) + ", 'sample code', '" + verdict + "');";
    char* errMsg;
    sqlite3_exec(db, insertSubmission.c_str(), NULL, NULL, &errMsg);
}

int main() {
    sqlite3* db;
    sqlite3_open("judge.db", &db);

    int problemId = 1; // hardcoded for now, we'll make this dynamic later
    loadProblem(db, problemId);
    writeExpectedFile();

    cout << "Loaded problem. Expected output: " << expectedOutput << endl;

    // kill any previously running solution.exe before compiling
    system("taskkill /F /IM solution.exe >nul 2>&1");

    // step 1 - compile
    cout << "Compiling..." << endl;
    int compileResult = system("g++ solution.cpp -o solution.exe");
    
    if(compileResult != 0) {
        cout << "Verdict: Compilation Error" << endl;
        saveSubmission(db, problemId, "Compilation Error");
        sqlite3_close(db);
        return 0;
    }
    cout << "Compilation successful" << endl;

    // step 2 - run with time limit
    cout << "Running..." << endl;

    STARTUPINFOA si;
    PROCESS_INFORMATION pi;
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);

    char cmd[] = "cmd.exe /c solution.exe < input.txt > output.txt";
    char workingDir[] = "C:\\Users\\HP\\.vscode\\Code Judge";

    BOOL success = CreateProcessA(
        NULL, cmd, NULL, NULL,
        FALSE, 0, NULL,
        workingDir,
        &si, &pi
    );

    if(!success) {
        cout << "Execution failed. Error: " << GetLastError() << endl;
        sqlite3_close(db);
        return 1;
    }

    // wait max 2 seconds
    DWORD result = WaitForSingleObject(pi.hProcess, 2000);

    if(result == WAIT_TIMEOUT) {
        TerminateProcess(pi.hProcess, 0);
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
        cout << "Verdict: Time Limit Exceeded" << endl;
        saveSubmission(db, problemId, "Time Limit Exceeded");
        sqlite3_close(db);
        return 0;
    }

    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);

    // step 3 - compare output
    cout << "Checking output..." << endl;

    ifstream out("output.txt");
    ifstream exp("expected.txt");

    if(!out.is_open()) {
        cout << "Error: output.txt not found" << endl;
        sqlite3_close(db);
        return 1;
    }
    if(!exp.is_open()) {
        cout << "Error: expected.txt not found" << endl;
        sqlite3_close(db);
        return 1;
    }

    string outStr, expStr;
    getline(out, outStr);
    getline(exp, expStr);

    out.close();
    exp.close();

    string verdict;
    if(trim(outStr) == trim(expStr)) {
        verdict = "Accepted";
        cout << "Verdict: Accepted" << endl;
    } else {
        verdict = "Wrong Answer";
        cout << "Verdict: Wrong Answer" << endl;
        cout << "Your output: " << outStr << endl;
        cout << "Expected:    " << expStr << endl;
    }

    saveSubmission(db, problemId, verdict);
    sqlite3_close(db);

    return 0;
}