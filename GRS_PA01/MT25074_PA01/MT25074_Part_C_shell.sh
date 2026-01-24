#!/bin/bash

# =========================================================
# MT25074 - Part C: Verified Execution & Gnuplot
# =========================================================

OUTPUT_FILE="MT25074_Part_C_CSV.csv"
PIN_CORE="2"
TEST_COUNT="2"

# --- DEPENDENCY CHECK ---
check_tool() {
    if ! command -v $1 &> /dev/null; then
        echo "Error: Required tool '$1' is not installed."
        echo "Install it using: sudo apt install $2 -y"
        exit 1
    fi
}

echo "Checking dependencies..."
check_tool "make" "build-essential"
check_tool "gnuplot" "gnuplot"
check_tool "iostat" "sysstat"
check_tool "bc" "bc"
echo "All dependencies found."
echo "-----------------------------------------------------"

# 1. CLEANUP & COMPILE
rm -f $OUTPUT_FILE
make clean > /dev/null 2>&1
make > /dev/null 2>&1

if [ ! -f "./program_a1" ] || [ ! -f "./program_a2" ]; then
    echo "Error: Executables not found. Compilation failed."
    exit 1
fi

echo "Step 1: Measuring Resource Usage (Pinned to Core $PIN_CORE)"
echo "-----------------------------------------------------"

# 2. HEADER (Updated to use Commas)
echo "Program+Function,CPU,Mem,IO" | tee $OUTPUT_FILE

measure_stats() {
    prog_cmd=$1
    task=$2
    label=$3
    prog_name=$(basename "$prog_cmd")

    # Run Program
    taskset -c $PIN_CORE $prog_cmd $task $TEST_COUNT > /dev/null 2>&1 &
    MAIN_PID=$!

    total_cpu=0; total_mem=0; total_io=0; samples=0

    # Monitor Loop
    while [ -e /proc/$MAIN_PID ]; do
        state=$(awk '{print $3}' /proc/$MAIN_PID/stat 2>/dev/null)
        if [ "$state" == "Z" ] || [ -z "$state" ]; then break; fi

        PIDS=$(pgrep -d',' -f "$prog_name")
        if [ -z "$PIDS" ]; then sleep 0.1; continue; fi

        # Capture Stats
        stats=$(top -b -n 1 -p "$PIDS" | grep -E "^ *[0-9]+" | awk '{cpu+=$9; mem+=$10} END {print cpu+0, mem+0}')
        io_now=$(iostat -d -k 1 2 | grep -E "sd[a-z]|vd[a-z]|nvme" | tail -1 | awk '{print $4}')

        cpu_now=$(echo "$stats" | awk '{print $1}')
        mem_now=$(echo "$stats" | awk '{print $2}')

        # Sanitize empty values
        if [ -z "$cpu_now" ]; then cpu_now=0; fi
        if [ -z "$mem_now" ]; then mem_now=0; fi
        if [ -z "$io_now" ]; then io_now=0; fi

        # Sum using bc
        total_cpu=$(echo "$total_cpu + $cpu_now" | bc)
        total_mem=$(echo "$total_mem + $mem_now" | bc)
        total_io=$(echo "$total_io + $io_now" | bc)
        samples=$((samples + 1))

        sleep 1
    done

    # Calculate Average
    if [ $samples -eq 0 ]; then samples=1; fi
    avg_cpu=$(echo "scale=2; $total_cpu / $samples" | bc)
    avg_mem=$(echo "scale=2; $total_mem / $samples" | bc)
    avg_io=$(echo "scale=2; $total_io / $samples" | bc)

    # OUTPUT (Updated to use Commas)
    echo "$label+$task,$avg_cpu,$avg_mem,$avg_io" | tee -a $OUTPUT_FILE
}

# 3. RUN EXPERIMENTS
measure_stats "./program_a1" "cpu" "Process_A"
measure_stats "./program_a1" "mem" "Process_A"
measure_stats "./program_a1" "io"  "Process_A"

measure_stats "./program_a2" "cpu" "Thread_B"
measure_stats "./program_a2" "mem" "Thread_B"
measure_stats "./program_a2" "io"  "Thread_B"

echo "-----------------------------------------------------"
echo "Data Collection Complete!"

# =========================================================
# STEP 4: GENERATE PLOTS (GNUPLOT)
# =========================================================
echo "Step 2: Generating Plots using Gnuplot..."

if ! command -v gnuplot &> /dev/null; then
    echo "Error: Gnuplot not found. Install: sudo apt install gnuplot"
    exit 1
fi

# A. Prepare Data for Bar Charts
# Updated awk to use comma delimiter (-F,)
p_cpu=$(grep "Process_A+cpu" $OUTPUT_FILE | awk -F, '{print $2}')
t_cpu=$(grep "Thread_B+cpu" $OUTPUT_FILE | awk -F, '{print $2}')
echo "CPU $p_cpu $t_cpu" > plot_cpu.dat

p_mem=$(grep "Process_A+mem" $OUTPUT_FILE | awk -F, '{print $3}')
t_mem=$(grep "Thread_B+mem" $OUTPUT_FILE | awk -F, '{print $3}')
echo "Mem $p_mem $t_mem" > plot_mem.dat

p_io=$(grep "Process_A+io" $OUTPUT_FILE | awk -F, '{print $4}')
t_io=$(grep "Thread_B+io" $OUTPUT_FILE | awk -F, '{print $4}')
echo "IO $p_io $t_io" > plot_io.dat

# B. Run Gnuplot Script
gnuplot << EOF
set terminal pngcairo size 1800,600 enhanced font 'Verdana,10'
set output 'MT25074_Part_C_Plot.png'
set multiplot layout 1,3 title "Part C: Resource Usage (Generated via Bash/Gnuplot)" font ",14"

# Common Settings
set style data histograms
set style histogram cluster gap 1
set style fill solid border -1
set boxwidth 0.9
set grid y

# -- Graph 1: CPU --
set title "CPU Usage (Higher is Better)"
set ylabel "CPU %"
set yrange [0:110]
plot 'plot_cpu.dat' using 2:xtic(1) title "Processes" lc rgb "blue", \
     ''             using 3 title "Threads" lc rgb "orange"

# -- Graph 2: Memory --
set title "Memory Usage (Target: 256MB)"
set ylabel "RAM %"
set yrange [0:]
plot 'plot_mem.dat' using 2:xtic(1) title "Processes" lc rgb "blue", \
     ''             using 3 title "Threads" lc rgb "orange"

# -- Graph 3: IO --
set title "Disk Throughput"
set ylabel "Speed (KB/s)"
unset yrange
set autoscale y
plot 'plot_io.dat' using 2:xtic(1) title "Processes" lc rgb "blue", \
     ''             using 3 title "Threads" lc rgb "orange"

unset multiplot
EOF

# Cleanup Temp Files
rm plot_cpu.dat plot_mem.dat plot_io.dat

echo "Success! Generated: MT25074_Part_C_Plot.png"
echo "-----------------------------------------------------"
