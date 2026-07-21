import http.server
import json
import sqlite3
import subprocess
import os
import urllib.parse
import hashlib
import secrets
from http.server import HTTPServer, BaseHTTPRequestHandler

# path to your Code Judge folder
BASE_DIR     = r"C:\Users\HP\.vscode\Code Judge"
DB_PATH      = os.path.join(BASE_DIR, "judge.db")
FRONTEND_DIR = os.path.join(BASE_DIR, "frontend")

def hash_password(password):
    return hashlib.sha256(password.encode()).hexdigest()

def generate_token():
    return secrets.token_hex(32)

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

    # ── helper: serve HTML file ───────────────────────────
    def serve_file(self, filename):
        filepath = os.path.join(FRONTEND_DIR, filename)
        try:
            with open(filepath, 'rb') as f:
                content = f.read()
            self.send_response(200)
            self.send_header("Content-Type", "text/html")
            self.send_header("Content-Length", len(content))
            self.end_headers()
            self.wfile.write(content)
        except FileNotFoundError:
            self.send_json({"error": "file not found"}, 404)

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

        # ── serve frontend HTML files ─────────────────────
        if path == '/' or path == '/index.html':
            self.serve_file('index.html')
            return
        elif path == '/problem.html':
            self.serve_file('problem.html')
            return
        elif path == '/login.html':
            self.serve_file('login.html')
            return
        elif path == '/register.html':
            self.serve_file('register.html')
            return
        elif path == '/profile.html':
            self.serve_file('profile.html')
            return

        # GET /problems — return all problems
        elif path == "/problems":
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

        # GET /profile?username=x — return user stats
        elif path == "/profile":
            username = params.get("username", [None])[0]
            if not username:
                self.send_json({"error": "username required"}, 400)
                return

            conn = sqlite3.connect(DB_PATH)
            conn.row_factory = sqlite3.Row
            cur  = conn.cursor()

            cur.execute(
                "SELECT id, username, created_at FROM users "
                "WHERE username=?", (username,))
            user = cur.fetchone()

            if not user:
                self.send_json({"error": "User not found"}, 404)
                conn.close()
                return

            cur.execute("""
                SELECT
                    COUNT(s.id) as total,
                    SUM(CASE WHEN s.verdict='Accepted'
                        THEN 1 ELSE 0 END) as accepted
                FROM submissions s
                WHERE s.user_id=?
            """, (user['id'],))
            stats = cur.fetchone()

            total    = stats['total']    or 0
            accepted = stats['accepted'] or 0
            rate     = round(accepted * 100.0 / total, 1) \
                       if total > 0 else 0

            cur.execute("""
                SELECT s.id, p.title, s.verdict, s.timestamp
                FROM submissions s
                JOIN problems p ON s.problem_id = p.id
                WHERE s.user_id=?
                ORDER BY s.timestamp DESC
                LIMIT 10
            """, (user['id'],))

            submissions = [{
                "id":        row['id'],
                "problem":   row['title'],
                "verdict":   row['verdict'],
                "timestamp": row['timestamp']
            } for row in cur.fetchall()]

            conn.close()
            self.send_json({
                "username":    user['username'],
                "joined":      user['created_at'],
                "total":       total,
                "accepted":    accepted,
                "rate":        rate,
                "submissions": submissions
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
            user_id    = body.get("user_id", 0)

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
                [os.path.join(BASE_DIR, "submit.exe"),
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

            # store user_id in queue row
            if queue_id and user_id:
                cur.execute(
                    "UPDATE queue SET user_id=? WHERE id=?",
                    (user_id, queue_id))
                conn.commit()

            conn.close()

            self.send_json({
                "queue_id": queue_id,
                "message":  "Submitted successfully"
            })

        # POST /register
        elif self.path == '/register':
            body     = self.read_body()
            username = body.get('username', '').strip()
            password = body.get('password', '').strip()

            if not username or not password:
                self.send_json(
                    {"error": "Username and password required"}, 400)
                return

            if len(username) < 3:
                self.send_json(
                    {"error": "Username must be at least 3 characters"},
                    400)
                return

            if len(password) < 6:
                self.send_json(
                    {"error": "Password must be at least 6 characters"},
                    400)
                return

            conn = sqlite3.connect(DB_PATH)
            cur  = conn.cursor()

            cur.execute(
                "SELECT id FROM users WHERE username=?", (username,))
            if cur.fetchone():
                conn.close()
                self.send_json(
                    {"error": "Username already taken"}, 400)
                return

            hashed = hash_password(password)
            cur.execute(
                "INSERT INTO users (username, password) VALUES (?,?)",
                (username, hashed))
            conn.commit()
            user_id = cur.lastrowid

            token = generate_token()
            cur.execute(
                "INSERT INTO sessions (user_id, token) VALUES (?,?)",
                (user_id, token))
            conn.commit()
            conn.close()

            self.send_json({
                "message":  "Registration successful",
                "token":    token,
                "username": username,
                "user_id":  user_id
            })

        # POST /login
        elif self.path == '/login':
            body     = self.read_body()
            username = body.get('username', '').strip()
            password = body.get('password', '').strip()

            if not username or not password:
                self.send_json(
                    {"error": "Username and password required"}, 400)
                return

            conn = sqlite3.connect(DB_PATH)
            conn.row_factory = sqlite3.Row
            cur  = conn.cursor()

            cur.execute(
                "SELECT id, password FROM users WHERE username=?",
                (username,))
            user = cur.fetchone()

            if not user or \
               user['password'] != hash_password(password):
                conn.close()
                self.send_json(
                    {"error": "Invalid username or password"}, 401)
                return

            token = generate_token()
            cur.execute(
                "INSERT INTO sessions (user_id, token) VALUES (?,?)",
                (user['id'], token))
            conn.commit()
            conn.close()

            self.send_json({
                "message":  "Login successful",
                "token":    token,
                "username": username,
                "user_id":  user['id']
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
