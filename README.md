# Online-Judge

A LeetCode-style code judging platform built in C++.

## Features
- Automated code compilation and execution
- Time limit enforcement using Windows CreateProcess API
- SQLite database for problems and submissions
- Queue-based submission processing worker

## Tech Stack
- C++ (judge engine, worker, queue)
- SQLite (database layer)
- Windows CreateProcess API (process management)
- HTML/CSS/JS (frontend — in progress)

## Architecture
User submits code → Queue table (SQLite) → Worker picks up 
→ Judge compiles + runs with timeout → Verdict stored → Frontend displays result

## Phases
- Phase 1 - Judge Engine (compile, run, evaluate, timeout)
- Phase 2 - Database (problems + submissions tables)
- Phase 3 - Queue System (worker processes submissions one by one)
- Phase 4 - Frontend (in progress)
