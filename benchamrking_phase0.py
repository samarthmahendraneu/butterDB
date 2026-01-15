#!/usr/bin/env python3
import socket
import time
import random
import string
import threading
import psutil
import statistics
from contextlib import closing
import matplotlib.pyplot as plt

# ---------- Config ----------
HOST, PORT = "127.0.0.1", 9090
OPS = ["PUT", "GET", "DEL"]   # workload mix
BASE_OPS = 10000              # per client for baseline
SCALING_CLIENTS = [1, 2, 4, 8, 16, 32]  # for scaling test

# ---------- Utilities ----------
def rand_key(n=8):
    return ''.join(random.choice(string.ascii_lowercase) for _ in range(n))

def rand_val(n=16):
    return ''.join(random.choice(string.ascii_letters) for _ in range(n))

# ---------- Client Worker ----------
def run_client(op_counts, latencies):
    """Single client thread sending operations"""
    with closing(socket.socket(socket.AF_INET, socket.SOCK_STREAM)) as s:
        s.connect((HOST, PORT))
        for _ in range(op_counts):
            op = random.choices(OPS, weights=[0.5, 0.4, 0.1])[0]
            k = rand_key()
            v = rand_val() if op == "PUT" else ""
            msg = f"{op} {k} {v}\n" if v else f"{op} {k}\n"
            start = time.perf_counter()
            s.sendall(msg.encode())
            _ = s.recv(1024)
            latencies.append(time.perf_counter() - start)

# ---------- System Monitor ----------
def monitor_server(proc_name="dbserver", interval=0.5, stop_flag=None, results=None):
    """Continuously samples CPU and memory usage of server process"""
    procs = [p for p in psutil.process_iter(['pid', 'name']) if proc_name in p.info['name']]
    if not procs:
        print("⚠️  Could not find server process; CPU/mem metrics unavailable")
        return
    proc = procs[0]
    while not stop_flag.is_set():
        try:
            results["cpu"].append(proc.cpu_percent(interval=interval))
            results["mem"].append(proc.memory_info().rss / (1024 * 1024))
        except psutil.NoSuchProcess:
            break

# ---------- Single Benchmark Run ----------
def run_benchmark(n_clients, n_ops):
    latencies = []
    stop_flag = threading.Event()
    mon_data = {"cpu": [], "mem": []}

    monitor_thread = threading.Thread(target=monitor_server,
                                      args=("dbserver", 0.5, stop_flag, mon_data))
    monitor_thread.start()

    threads = []
    start = time.time()
    for _ in range(n_clients):
        t = threading.Thread(target=run_client, args=(n_ops, latencies))
        t.start()
        threads.append(t)
    for t in threads:
        t.join()
    total_time = time.time() - start
    stop_flag.set()
    monitor_thread.join(timeout=1)

    total_ops = n_clients * n_ops
    throughput = total_ops / total_time
    avg_latency = sum(latencies) / len(latencies)
    p95_latency = statistics.quantiles(latencies, n=100)[94] if latencies else 0
    avg_cpu = sum(mon_data["cpu"]) / len(mon_data["cpu"]) if mon_data["cpu"] else 0
    avg_mem = sum(mon_data["mem"]) / len(mon_data["mem"]) if mon_data["mem"] else 0

    return {
        "clients": n_clients,
        "ops": total_ops,
        "time": total_time,
        "throughput": throughput,
        "avg_latency_ms": avg_latency * 1000,
        "p95_latency_ms": p95_latency * 1000,
        "cpu": avg_cpu,
        "mem": avg_mem,
    }

# ---------- Main Benchmark Suite ----------
def main():
    print("\n🚀 Phase 0 Benchmark Suite — KV Store (Hashmap Backend)\n")

    # ---------- Test 1: Baseline ----------
    print("🏁 Test 1 — Baseline Run (4 clients × 40k ops each)\n")
    baseline = run_benchmark(4, BASE_OPS)
    print(f"  • Total Ops:     {baseline['ops']:,}")
    print(f"  • Total Time:    {baseline['time']:.3f} s")
    print(f"  • Throughput:    {baseline['throughput']:.1f} ops/s")
    print(f"  • Avg Latency:   {baseline['avg_latency_ms']:.3f} ms")
    print(f"  • p95 Latency:   {baseline['p95_latency_ms']:.3f} ms")
    print(f"  • CPU Usage:     {baseline['cpu']:.2f}%")
    print(f"  • Memory:        {baseline['mem']:.2f} MB")

    # ---------- Test 2: Scaling ----------
    print("\n📈 Test 2 — Scaling with Concurrent Clients\n")
    throughputs, latencies = [], []
    for n in SCALING_CLIENTS:
        res = run_benchmark(n, BASE_OPS // 2)
        throughputs.append(res["throughput"])
        latencies.append(res["avg_latency_ms"])
        print(f"  {n:>2} clients → {res['throughput']:.1f} ops/s, "
              f"{res['avg_latency_ms']:.2f} ms avg, CPU {res['cpu']:.1f}%")

    # ---------- Plot ----------
    plt.figure(figsize=(7,5))
    plt.plot(SCALING_CLIENTS, throughputs, marker='o', label='Throughput (ops/s)')
    plt.xlabel("Concurrent Clients")
    plt.ylabel("Throughput (ops/s)")
    plt.title("KV Store Throughput Scaling")
    plt.grid(True)
    plt.legend()

    plt.figure(figsize=(7,5))
    plt.plot(SCALING_CLIENTS, latencies, marker='s', color='orange', label='Avg Latency (ms)')
    plt.xlabel("Concurrent Clients")
    plt.ylabel("Average Latency (ms)")
    plt.title("KV Store Latency Scaling")
    plt.grid(True)
    plt.legend()

    plt.tight_layout()
    plt.show()

    print("\n✅ Benchmarking complete.\n")

if __name__ == "__main__":
    main()
