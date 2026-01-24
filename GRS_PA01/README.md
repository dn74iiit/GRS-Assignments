# Process vs. Thread Performance Analysis
**Roll No:** MT25074
**Name:** Dhanush
**Course:** Graduate Systems (CSE638) - PA01

## Overview
This assignment explores the performance differences between Unix Processes (`fork`) and POSIX Threads (`pthread`). I implemented a C program that spawns workers to perform CPU-intensive, Memory-intensive, and I/O-intensive tasks. The project measures resource usage and scaling behavior when pinned to a specific CPU core.

## Directory Structure
* **Source Code:**
    * `MT25074_Part_A_Program_A.c`: Process manager using `fork()`.
    * `MT25074_Part_A_Program_B.c`: Thread manager using `pthread_create()`.
    * `MT25074_Part_B_Program.c`: Contains the worker logic (`cpu`, `mem`, `io`).
    * `MT25074_Part_B_Program.h`: Header file for worker functions.
* **Automation Scripts:**
    * `MT25074_Part_C_shell.sh`: Runs the base resource measurement (Part C) and generates bar charts using Gnuplot.
    * `MT25074_Part_D_shell.sh`: Runs the scaling analysis (Part D) and generates line charts using Gnuplot.
* **Data & Plots:**
    * `MT25074_Part_C_CSV.csv` / `MT25074_Part_D_CSV.csv`: Raw measurement data.
    * `MT25074_Part_C_Plot.png` / `MT25074_Part_D_Plot.png`: Generated graphs.

## How to Compile & Run (Step-by-Step)

### 1. Compile the Code
Run the `make` command to build both executables (`program_a1` and `program_a2`).
```bash
make clean && make
```
or 
```bash
make
```


### 2. Run Part C (Resource Usage)
This script runs the programs with 2 workers, monitors system stats (CPU, Mem, Disk IO), and automatically generates the Bar Charts (MT25074_Part_C_Plot.png) and csv file.
```bash
chmod +x MT25074_Part_C_shell.sh
./MT25074_Part_C_shell.sh
```

### 3. Run Part D (Scaling Analysis)
This script runs the programs with worker counts scaling from 2 to 8. It measures performance trends and automatically generates the Line Charts (MT25074_Part_D_Plot.png) and the csv file.
```bash
chmod +x MT25074_Part_D_shell.sh
./MT25074_Part_D_shell.sh
```

## Implementation Details

* Pinning: All experiments are pinned to a single core (Core 0/2) using taskset to ensure fair comparison and force context switching.
* CPU Task: Uses Trigonometric functions (sin/cos) to stress the FPU.
* Memory Task: Allocates a 256MB array per worker and performs linear writes to force cache misses.
* IO Task: Writes 4KB blocks to the disk in /tmp/ and uses fsync to force physical disk writes.

## Script Implementation Details

The project uses two Bash scripts to automate data collection and visualization. I prioritized a **"Zero-Python"** approach, using standard Linux tools (`awk`, `grep`, `bc`) to ensure the solution is lightweight and portable on any Unix system.

### Common Logic (Both Scripts)
1.  **Dependency Checking:** A `check_tool()` function runs at startup to verify that `make`, `gnuplot`, `sysstat`, and `bc` are installed. If missing, it halts execution and provides the install command.
2.  **Core Pinning:** All executables are launched using `taskset -c 0` (or `2`) to force execution on a single core. This eliminates scheduler noise from multi-core balancing.
3.  **PID Tracking:** The scripts launch the C programs in the background (`&`) and immediately capture the parent PID (`$!`). A `while` loop monitors `/proc/$MAIN_PID/stat` to keep the script running exactly as long as the C program is active.
4.  **Floating Point Math:** Since Bash does not support floating-point arithmetic natively, I piped all calculations (sums, averages) to `bc`.

### Specifics: `MT25074_Part_C_shell.sh` 
* **Sampling:** Runs a measurement loop every 1 second.
* **Metrics:**
  * **CPU/Mem:** Parsed from `top -b -n 1 -p $PIDS`.
  * **Disk I/O:** Parsed from `iostat -d -k 1 2`. I specifically use `1 2` and `tail -1` to capture the *current* throughput rather than the system uptime average and store in the file `MT25074_Part_C_CSV.csv`.
* **Visualization:** Uses `awk` to format the CSV data into Gnuplot-ready temp files, then executes an embedded Gnuplot script to generate grouped bar charts (`MT25074_Part_C_Plot.png`).

### Specifics: `MT25074_Part_D_shell.sh` 
* **Outer Loop:** Iterates from `worker_count = 2` to `8`.
* **Inner Loop:** Runs the experiment for `Process` then `Thread` models.
* **Data Aggregation:** Calculates the average CPU%, Memory%, and I/O Throughput over the duration of the run and appends it to `MT25074_Part_D_CSV.csv`.
* **Visualization:** Generates a 3-panel line chart (`MT25074_Part_D_Plot.png`) comparing the scaling trends of Processes vs. Threads.

## Dependencies and Automation
The Bash scripts include an Automatic Dependency Check. When you run them, they will verify if the following tools are installed:
* make / gcc: For compilation.
* gnuplot: For generating the plots.
* sysstat: For the iostat command.
* bc: For floating-point calculations in Bash.
If a tool is missing, the script will pause and provide the exact sudo apt install command needed to fix it.

## AI Usage Declaration

I used AI tools to assist in knowing  the Makefile syntax, debugging the Bash scripts for top/iostat parsing, and creating the Gnuplot templates. All C logic for the worker functions and the analysis of the results are my own work.


