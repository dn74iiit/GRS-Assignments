#!/bin/bash

# =========================================================
# MT25074 - Part D: Scaling Analysis & Gnuplot
# =========================================================

OUTPUT_FILE="MT25074_Part_D_CSV.csv"
PIN_CORE="2"

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
    echo "Error: Executables not found."
    exit 1
fi

echo "Step 2: Starting Scaling Analysis (Pinned to Core $PIN_CORE)"
echo "-----------------------------------------------------"

# 2. WRITE CSV HEADER
echo "Program,Task,Count,Value" | tee $OUTPUT_FILE

# 3. STAT COLLECTION FUNCTION
measure_stats() {
    prog_cmd=$1
    prog_name=$2

    for task in "cpu" "mem" "io"; do
        for count in {2..8}; do

            echo -n "Running $prog_name [$task] Count: $count ... "

            # A. Start Workload
            taskset -c $PIN_CORE $prog_cmd $task $count > /dev/null 2>&1 &
            WORKLOAD_PID=$!

            # B. Monitor Loop
            total_metric=0
            samples=0
            sleep 0.2

            while kill -0 $WORKLOAD_PID 2> /dev/null; do

                # Identify PIDs
                if [[ "$prog_name" == "Process_A" ]]; then
                    PIDS=$(pgrep -f "program_a1" | tr '\n' ',' | sed 's/,$//')
                else
                    PIDS=$(pgrep -f "program_a2" | tr '\n' ',' | sed 's/,$//')
                fi

                if [ -z "$PIDS" ]; then break; fi

                # Capture Metrics
                current_val=0.0
                if [[ "$task" == "cpu" ]]; then
                    current_val=$(top -b -n 1 -p "$PIDS" 2>/dev/null | awk 'NR>7 {sum+=$9} END {print sum+0}')
                elif [[ "$task" == "mem" ]]; then
                    current_val=$(top -b -n 1 -p "$PIDS" 2>/dev/null | awk 'NR>7 {sum+=$10} END {print sum+0}')
                elif [[ "$task" == "io" ]]; then
                    # --- FIX IS HERE: Use '1 2' and 'tail -1' ---
                    current_val=$(iostat -d -k 1 2 | grep -E 'sd|vd|nvme' | tail -1 | awk '{sum+=$4} END {print sum}')
                fi

                total_metric=$(echo "$total_metric + $current_val" | bc)
                samples=$((samples + 1))
                sleep 1
            done


            # Computing Average
            if [ $samples -gt 0 ]; then
                avg_val=$(echo "scale=2; $total_metric / $samples" | bc)
            else
                avg_val="0"
            fi

            echo "Avg: $avg_val"
            echo "$prog_name,$task,$count,$avg_val" >> $OUTPUT_FILE
        done
        echo ""
    done
}

# 4. RUN EXPERIMENTS
measure_stats "./program_a1" "Process_A"
measure_stats "./program_a2" "Thread_B"

echo "Part D Data Collection Complete!"
echo "-----------------------------------------------------"

# =========================================================
# STEP 5: GENERATE PLOTS (GNUPLOT)
# =========================================================
echo "Step 3: Generating Plots using Gnuplot..."

# Prepare Data
grep "Process_A,cpu" $OUTPUT_FILE > proc_cpu.dat
grep "Thread_B,cpu"  $OUTPUT_FILE > thread_cpu.dat
grep "Process_A,mem" $OUTPUT_FILE > proc_mem.dat
grep "Thread_B,mem"  $OUTPUT_FILE > thread_mem.dat
grep "Process_A,io"  $OUTPUT_FILE > proc_io.dat
grep "Thread_B,io"   $OUTPUT_FILE > thread_io.dat

# Run Gnuplot
gnuplot << EOF
set terminal pngcairo size 1800,600 enhanced font 'Verdana,10'
set output 'MT25074_Part_D_Plot.png'
set datafile separator ","
set multiplot layout 1,3 title "Part D: Scaling Analysis (Generated via Bash/Gnuplot)" font ",14"

# -- Graph 1: CPU --
set title "CPU Usage"
set xlabel "Worker Count"
set ylabel "CPU %"
set grid
set key bottom right
plot 'proc_cpu.dat' using 3:4 with linespoints title "Processes" lc rgb "blue" lw 2 pt 7, \
     'thread_cpu.dat' using 3:4 with linespoints title "Threads" lc rgb "orange" lw 2 pt 7

# -- Graph 2: Memory --
set title "Memory Usage"
set xlabel "Worker Count"
set ylabel "RAM %"
set grid
set key top left
plot 'proc_mem.dat' using 3:4 with linespoints title "Processes" lc rgb "blue" lw 2 pt 7, \
     'thread_mem.dat' using 3:4 with linespoints title "Threads" lc rgb "orange" lw 2 pt 7

# -- Graph 3: IO --
set title "Disk I/O Speed"
set xlabel "Worker Count"
set ylabel "Throughput (KB/s)"
set grid
set key top left
plot 'proc_io.dat' using 3:4 with linespoints title "Processes" lc rgb "blue" lw 2 pt 7, \
     'thread_io.dat' using 3:4 with linespoints title "Threads" lc rgb "orange" lw 2 pt 7

unset multiplot
EOF

# Cleanup
rm proc_cpu.dat thread_cpu.dat proc_mem.dat thread_mem.dat proc_io.dat thread_io.dat

echo "Success! Generated: MT25074_Part_D_Plot.png"
echo "-----------------------------------------------------"