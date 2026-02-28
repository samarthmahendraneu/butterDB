# 🧈 ButterDB — Phase 2: Concurrent Persistent B-Tree

### 🎯 Goal
Build a serious database engine from scratch.  
**ButterDB** is now a **multi-threaded, persistent Key-Value Store** built on a B-Tree structure. It features a custom paging engine, buffer pool, write-ahead logging (WAL) for durability, and fine-grained locking for concurrency.

---

### 🧠 Features

- **Storage Engine:** Fixed-size **4KB Paged Architecture** (custom `pager.c`).
- **Data Structure:** Disk-resident **B-Tree**.
- **Durability (ACID):** **Write-Ahead Logging (WAL)** ensures no data loss on power failure.
- **Concurrency:**
    - **Latch Crabbing** (Lock Coupling) allowing multiple threads to traverse the tree in parallel.
    - **Thread-Safe WAL** with atomic appends.
    - **Multi-threaded Server** handling concurrent TCP connections.

---

### ⚙️ Architecture

<img width="1358" height="1182" alt="image" src="https://github.com/user-attachments/assets/1fdd81b8-7e5d-4eb2-b46d-74d46c0fcede" />

https://excalidraw.com/#json=sf7-nVIwhUrWbGRNW11mp,aXHw-GSD1zc3wF-r0HB-kw


#### 1. The Pager (`pager.c`)
- Abstraction over the OS file system.
- Reads/Writes 4KB blocks (`page_id * 4096`).

#### 2. The Buffer Pool (`btree.c`)
- Caches frequently accessed pages in RAM.
- **Pinning:** Protects pages from eviction while in use.
- **Eviction:** Uses a "Clock" or "First-Unpinned" policy to flush dirty pages to disk when memory is full.

#### 3. Concurrency Control
- **Fine-Grained Locking:** Uses `pthread_mutex` per Page.
- **Protocol:** "Latch Crabbing" — Lock Parent → Lock Child → Unlock Parent.
- **Deadlock Free:** Top-down locking order guarantees no circular waits.

#### 4. Durability & Recovery (`wal.c`)
- **WAL Protocol:** Log records are written to `butterdb.wal` *before* dirty pages touch the disk.
- **Crash Recovery:** On startup, re-plays the log to reconstruct lost in-memory state.

---

### 🧪 Usage

```bash
make
./butterdb
# Output:
# Recovering from WAL... (Iterating)
# ButterDB Phase 1 (B-tree) running on port 9090...
# Waiting for client connections (Multi-threaded)...
```

**Client connection (via telnet or nc):**

```bash
nc localhost 9090
PUT user:1 {"name": "Alice"}
OK
GET user:1
{"name": "Alice"}
```

### 📂 File Structure

```bash
butterdb/
├── dbserver.c     # Multi-threaded TCP server
├── btree.c        # B-Tree implementation (Buffer Pool + Concurrency)
├── pager.c        # Fixed-Page Storage Engine
├── wal.c          # Write-Ahead Log (Durability)
├── btree.h        # Data structures
├── wal.h          # WAL interface
├── Makefile       # Build script
└── README.md
```
