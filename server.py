import http.server
import json
import sqlite3
import subprocess
import os
import urllib.parse
from http.server import HTTPServer, BaseHTTPRequestHandler

# path to your Code Judge folder
BASE_DIR = r"C:\Users\HP\.vscode\Code Judge"
DB_PATH  = os.path.join(BASE_DIR, "judge.db")

class JudgeHandler(BaseHTTPRequestHandler):

    # ── silence default request logs ─────────────────────
    def log_message(self, format, *args):
        pass

    # ── helper: send JSON response ────────────────────────
    def send_json(self, data, status=200):
        body = json.dumps(data).encode()
        self.send_response(status)
        self.send_header("Content-Type", "application/json")
        self.send_header("Access-Control-Allow-Origin", "*")
        self.send_header("Content-Length", len(body))
        self.end_headers()
        self.wfile.write(body)

    # ── helper: read POST body ────────────────────────────
    def read_body(self):
        length = int(self.headers.get("Content-Length", 0))
        return json.loads(self.rfile.read(length))

    # ── handle OPTIONS (browser preflight) ───────────────
    def do_OPTIONS(self):
        self.send_response(200)
        self.send_header("Access-Control-Allow-Origin", "*")
        self.send_header("Access-Control-Allow-Methods",
                         "GET, POST, OPTIONS")
        self.send_header("Access-Control-Allow-Headers",
                         "Content-Type")
        self.end_headers()

    # ── GET requests ──────────────────────────────────────
    def do_GET(self):
        parsed = urllib.parse.urlparse(self.path)
        path   = parsed.path
        params = urllib.parse.parse_qs(parsed.query)

        # GET /problems — return all problems
        if path == "/problems":
            conn = sqlite3.connect(DB_PATH)
            conn.row_factory = sqlite3.Row
            cur  = conn.cursor()

            cur.execute("""
                SELECT p.id, p.title, p.description,
                       p.difficulty, p.time_limit,
                       COUNT(s.id) as total,
                       SUM(CASE WHEN s.verdict='Accepted'
                           THEN 1 ELSE 0 END) as accepted
                FROM problems p
                LEFT JOIN submissions s ON p.id = s.problem_id
                GROUP BY p.id
            """)

            problems = []
            for row in cur.fetchall():
                total    = row["total"] or 0
                accepted = row["accepted"] or 0
                rate     = round(accepted * 100.0 / total, 1) \
                           if total > 0 else 0
                problems.append({
                    "id":          row["id"],
                    "title":       row["title"],
                    "difficulty":  row["difficulty"],
                    "total":       total,
                    "accepted":    accepted,
                    "rate":        rate
                })

            conn.close()
            self.send_json(problems)

        # GET /problem?id=1 — return one problem + test cases
        elif path == "/problem":
            problem_id = params.get("id", [None])[0]
            if not problem_id:
                self.send_json({"error": "id required"}, 400)
                return

            conn = sqlite3.connect(DB_PATH)
            conn.row_factory = sqlite3.Row
            cur  = conn.cursor()

            cur.execute(
                "SELECT * FROM problems WHERE id=?",
                (problem_id,)
            )
            row = cur.fetchone()

            if not row:
                self.send_json({"error": "not found"}, 404)
                conn.close()
                return

            # get first 2 test cases as examples
            # (hide the rest as hidden test cases)
            cur.execute(
                "SELECT input, expected_output "
                "FROM test_cases WHERE problem_id=? "
                "ORDER BY id ASC LIMIT 2",
                (problem_id,)
            )
            examples = [{"input": r["input"],
                         "output": r["expected_output"]}
                        for r in cur.fetchall()]

            conn.close()
            self.send_json({
                "id":          row["id"],
                "title":       row["title"],
                "description": row["description"],
                "difficulty":  row["difficulty"],
                "time_limit":  row["time_limit"],
                "examples":    examples
            })

        # GET /verdict?id=1 — check queue for verdict
        elif path == "/verdict":
            queue_id = params.get("id", [None])[0]
            if not queue_id:
                self.send_json({"error": "id required"}, 400)
                return

            conn = sqlite3.connect(DB_PATH)
            conn.row_factory = sqlite3.Row
            cur  = conn.cursor()

            cur.execute(
                "SELECT status, verdict FROM queue WHERE id=?",
                (queue_id,)
            )
            row = cur.fetchone()
            conn.close()

            if not row:
                self.send_json({"error": "not found"}, 404)
                return

            self.send_json({
                "status":  row["status"],
                "verdict": row["verdict"]
            })

        else:
            self.send_json({"error": "not found"}, 404)

    # ── POST requests ─────────────────────────────────────
    def do_POST(self):

        # POST /submit — save code, add to queue
        if self.path == "/submit":
            body       = self.read_body()
            problem_id = body.get("problem_id")
            code       = body.get("code", "")

            if not problem_id or not code:
                self.send_json(
                    {"error": "problem_id and code required"}, 400)
                return

            # write code to solution.cpp
            solution_path = os.path.join(BASE_DIR, "solution.cpp")
            with open(solution_path, "w") as f:
                f.write(code)

            # run submit.exe to add to queue
            subprocess.run(
                [os.path.join(BASE_DIR, "submit.exe")],
                str(problem_id)],
                cwd=BASE_DIR,
                capture_output=True
            )

            # get the queue id just inserted
            conn = sqlite3.connect(DB_PATH)
            cur  = conn.cursor()
            cur.execute(
                "SELECT id FROM queue "
                "ORDER BY id DESC LIMIT 1"
            )
            row      = cur.fetchone()
            queue_id = row[0] if row else None
            conn.close()

            self.send_json({
                "queue_id": queue_id,
                "message":  "Submitted successfully"
            })

        else:
            self.send_json({"error": "not found"}, 404)


# ── start server ──────────────────────────────────────────
if __name__ == "__main__":
    PORT   = 8000
    server = HTTPServer(("localhost", PORT), JudgeHandler)
    print(f"Judge server running at http://localhost:8000")
    print("Press Ctrl+C to stop")
    server.serve_forever()
