# MT25074
# PA02: Analysis of Network I/O Primitives using "perf" Tool
# Roll Number: MT25074 | Name: Nindra Dhanush

This repository contains the implementation and experiments for ** PA02**: comparing two-copy, one-copy, and zero-copy socket communication with multithreaded client–server programs and `perf` profiling.

---

## Overview

- **A1 (Two-Copy):** Baseline TCP using `send()`/`recv()`.
- **A2 (One-Copy):** Optimized path using `sendmsg()` with pre-registered buffers.
- **A3 (Zero-Copy):** Zero-copy path using `sendmsg()` with `MSG_ZEROCOPY`.

Server and client run in **separate network namespaces** (ns1, ns2) over a veth pair. Experiments use 4 message sizes (64, 256, 1024, 4096 bytes) and 4 thread counts (1, 2, 4, 8).

---

## Directory Layout

| File / Pattern | Description |
|----------------|-------------|
| `Makefile` | Builds all Part A server/client binaries. |
| `MT25074_Part_A_Namespaces.sh` | Creates ns1, ns2 and veth pair (10.0.0.1 / 10.0.0.2). |
| `MT25074_Part_A1_Server.c`, `MT25074_Part_A1_Client.c` | A1 two-copy implementation. |
| `MT25074_Part_A2_Server.c`, `MT25074_Part_A2_Client.c` | A2 one-copy implementation. |
| `MT25074_Part_A3_Server.c`, `MT25074_Part_A3_Client.c` | A3 zero-copy implementation. |
| `MT25074_Part_B_Run_Single_Experiment.sh` | Runs one experiment (A1/A2/A3, size, threads) with `perf stat`, writes one CSV. |
| `MT25074_Part_C_Run_Experiments.sh` | Runs all 48 experiments, produces per-run CSVs + `MT25074_Part_C_Results.csv`. |
| `MT25074_Part_C_Results.csv` | Aggregated results (cycles, instructions, IPC, cache misses, context switches). |
| `MT25074_Part_D_Plots.py` | Matplotlib script (hardcoded data) to generate the four Part D plots (PNG). |
| `MT25074_Part_A*_size*_threads*.csv` | Individual experiment CSVs from Part C. |

---

## Prerequisites

- **Linux** (namespaces and veth used; VM may not be suitable).
- **gcc** with pthread support.
- **perf** (e.g. `linux-tools-6.8.0-100` or your kernel’s `perf`).
- **Python 3** with **matplotlib** and **numpy** (for Part D plots).
- **sudo** for namespace and `perf` usage.

---

## Build

```bash
make clean   # optional
make all     # builds MT25074_Part_A1_Server, MT25074_Part_A1_Client, and same for A2, A3
```

Binaries: `MT25074_Part_A1_Server`, `MT25074_Part_A1_Client`, and similarly for A2 and A3.

---

## Usage

### 1. Setup network namespaces (once per session)

```bash
sudo bash MT25074_Part_A_Namespaces.sh
```

This creates `ns1`, `ns2`, a veth pair, and assigns 10.0.0.1 (ns1) and 10.0.0.2 (ns2).

### 2. Run a single experiment (Part B)

```bash
chmod +x MT25074_Part_B_Run_Single_Experiment.sh
sudo bash MT25074_Part_B_Run_Single_Experiment.sh <A1|A2|A3> <field_size> <num_threads>
```

Example:  
`sudo bash MT25074_Part_B_Run_Single_Experiment.sh A1 1024 4`  
Output: `MT25074_Part_A1_size1024_threads4.csv` (and prints that path).

### 3. Run all experiments (Part C)

```bash
chmod +x MT25074_Part_C_Run_Experiments.sh
sudo bash MT25074_Part_C_Run_Experiments.sh
```

- Cleans old CSVs, runs `make all`, sets up namespaces.
- Runs 48 experiments (3 parts × 4 sizes × 4 thread counts).
- Produces 48 individual CSVs and one aggregated `MT25074_Part_C_Results.csv`.
- Cleans namespaces and binaries at the end.

No manual steps are required after starting the script.

### 4. Generate Part D plots

```bash
python3 MT25074_Part_D_Plots.py
# or: python MT25074_Part_D_Plots.py
```

Plots are saved as PNGs in the current directory:

- `MT25074_Part_D_Plot1_Throughput_vs_Size.png`
- `MT25074_Part_D_Plot2_Latency_vs_Threads.png`
- `MT25074_Part_D_Plot3_CacheMisses_vs_Size.png`
- `MT25074_Part_D_Plot4_CyclesPerByte.png`

Data is hardcoded in the script from `MT25074_Part_C_Results.csv` (no CSV read at run time).

---

## CSV Format (Part B / Part C)

Each row (after header) has:

`part,field_size,num_threads,cycles,instructions,ipc,cache_misses,cache_references,cache_miss_rate,context_switches`

- **part:** A1, A2, or A3  
- **field_size:** 64, 256, 1024, or 4096  
- **num_threads:** 1, 2, 4, or 8  
- **cycles, instructions:** from `perf stat`  
- **ipc:** instructions per cycle  
- **cache_misses, cache_references, cache_miss_rate:** from `perf stat`  
- **context_switches:** from `perf stat`  



---

