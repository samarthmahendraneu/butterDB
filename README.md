# 🧩 Phase 0 — In-Memory Single-Threaded Server

### 🎯 Goal
Build the first prototype of **ButterDB** — a minimal, single-threaded key-value database server.  
It listens on a TCP port, accepts simple text commands (`PUT`, `GET`, `DEL`), and stores all data in memory.  
Focus: establish the **client–server protocol** and **network communication** before persistence.

---

### 🧠 Overview
- **Server:** Single-threaded TCP server handling one client at a time.
- **Protocol:** Plain-text commands over sockets.
- **Storage:** Fixed-size in-memory key-value table (`KVStore`).
- **Client:** Reads from stdin → sends commands → prints responses.

---

### ⚙️ Architecture

#### 🖥️ Server (`dbserver.c`)
- Creates socket → `bind()` → `listen()` → `accept()`.
- Reads and parses commands like:

PUT key value
GET key
DEL key
EXIT

- Executes the corresponding KV functions and returns results to the client.

#### 💾 Storage (`kvstore.c` / `kvstore.h`)
- `kv_put()` — insert or update key-value pairs
- `kv_get()` — fetch value for a key
- `kv_del()` — delete an entry
- Data lives entirely in RAM (non-persistent).

#### 🧑‍💻 Client (`dbclient.c`)
- Connects to the server via TCP.
- Reads commands from stdin, sends them, and prints responses.

---

### 🧪 Usage

```bash
make
./dbserver  # Start the ButterDB server


nc localhost 9090   # Connect as client
PUT name samarth
OK
GET name
samarth
DEL name
DELETED
EXIT

Connection closed by foreign host.
```

```bash
butterdb/
├── dbserver.c     # TCP server and command handler
├── dbclient.c     # Simple command-line client
├── kvstore.c      # In-memory key-value store logic
├── kvstore.h      # Struct definitions and function prototypes
├── Makefile       # Build script
└── README.md

```