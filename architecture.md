# ButterDB Technical Deep-Dive

This document details the internal architecture of ButterDB. Use this to explain **what you built**, **how it works**, and **why you made specific design decisions** during technical interviews.

## 1. Storage Engine: Fixed-Page Architecture
**Component**: `pager.c`, `pager.h`

Instead of letting the OS manage file structure (like writing structs directly to a file), you implemented a **Page-Based Storage Engine**.

### How it works
- **Page Size**: 4096 bytes (4KB).
- **Reasoning**: This matches the standard Virtual Memory page size of most CPUs and OSs. When we read 4KB from disk, valid systems often read a 4KB block anyway. Aligning writes prevents "Write Amplification" (where modifying 1 byte causes the OS to read-modify-write an entire block).
- **Addressing**: Nodes identify each other by `Page ID` (an integer index), not byte offsets. memory arithmetic is `PageID * 4096`.

### Buffer Pool (Cache)
- **Role**: Reduces Disk I/O.
- **Implementation**: A simple array of `Page` structs.
- **Pinning**: When a thread needs a page, it increments `ref_count` (Pinning). This tells the eviction algorithm "do not swap this page out, I am using it."
- **Eviction Policy**: Linear scan for `ref_count == 0`. (Interview Tip: Real DBs use LRU or Clock-Sweep; you implemented a simpler "First Unpinned" policy for Phase 2).

---

## 2. Concurrency Control: Latch Crabbing
**Component**: `btree.c`

To allow multiple operations to run safely in parallel (future-proofing the engine), you integrated **Per-Node Locking**.

### The Technique: "Latch Crabbing" (or Coupling)
- **Goal**: To modify a leaf node without blocking the availability of the Root for other readers.
- **How you implemented it**:
  1.  **Lock Parent**: Acquire lock on the current node.
  2.  **Safety Check**: Is this node "safe"? (e.g., for insert, not full).
  3.  **Lock Child & Release Parent**: If safe, release the parent lock immediately after locking the child.
- **Effect**: Threads "crab" down the tree `(Lock A -> Lock B -> Unlock A)`.
- **Trade-off**: This implementation is "pessimistic" (locks are exclusive). Optimistic approaches involves Reader-Writer locks (Shared for read, Exclusive for write).

---

## 3. Durability: Write-Ahead Logging (WAL)
**Component**: `wal.c`

To ensure no data is lost during a power failure/crash (Durability in ACID), you implemented the "WAL Protocol".

### The Golden Rule
> "Log records must be written to stable storage **before** the corresponding data page is written to disk."

### How you implemented it
1.  **Logical Logging**: When `btree_insert` happens, you write an entry `(LSN, INSERT, Key, Value)` to the append-only `wal.log` file.
2.  **Log Sequence Number (LSN)**: Every log entry gets a unique ID (currently its file offset).
3.  **Dirty Pages**: In memory, the modified Page is marked `dirty` and tagged with `page_lsn = LSN`.
4.  **Flush Enforcement**: In `get_page`, before evicting a dirty page to disk, the system checks:
    - *Is the WAL flushed up to `page_lsn`?*
    - If no, call `fsync` on the WAL.
    - Only then write the Page data.
- **Interview Win**: This guarantees that if the system crashes, either the page is already safe, or the redo information is safe in the log.

---

## 4. Crash Recovery: Redo History
**Component**: `btree.c` (`recover_from_log`)

When the database opens, it assumes the previous run might have crashed.

### The Algorithm
1.  **Scan WAL**: Open `butterdb.wal` and read from the beginning.
2.  **Replay**: For every `INSERT` record found, re-apply the operation to the B-Tree.
3.  **Idempotency**: Your implementation handles duplicates ("update in place") so re-running the same log twice doesn't start corrupting data.
- **Why it works**: Even if the B-Tree file (`btree.dat`) is empty (because pages never flushed), the WAL contains the full history of operations. Replaying it reconstructs the state.

---

## Summary for Interviews
*"I built a persistent Key-Value store in C from scratch. I moved away from simple `fwrite` structs to a **Page-Based Architecture** managing 4KB pages with a customized **Buffer Pool**. To handle data safety, I implemented **Write-Ahead Logging** with **Crash Recovery**, ensuring ACID durability, and added **Fine-Grained Locking** (Latch Crabbing) to prepare the system for high-concurrency workloads."*
