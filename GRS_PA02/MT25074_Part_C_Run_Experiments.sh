#!/bin/bash
# MT25074_Part_C_Run_Experiments.sh
# Automated experiments - produces individual CSV files + aggregated CSV

ROLL_NUM="MT25074"
PART_B_SCRIPT="./MT25074_Part_B_Run_Single_Experiment.sh"

SIZES=(64 256 1024 4096)
THREADS=(1 2 4 8)
AGGREGATED_CSV="${ROLL_NUM}_Part_C_Results.csv"

# Clean previous results (keep only .c, .h, .sh, Makefile, README)
echo "Cleaning previous results..."
rm -f ${ROLL_NUM}_Part_A?_size*_threads*.csv 2>/dev/null || true
rm -f "$AGGREGATED_CSV" 2>/dev/null || true

# Clean and build
make clean 2>/dev/null || true
sudo ip netns del ns1 2>/dev/null || true
sudo ip netns del ns2 2>/dev/null || true

echo "Building all implementations..."
make all || exit 1

echo "Setting up namespaces..."
sudo bash ${ROLL_NUM}_Part_A_Namespaces.sh || exit 1

# Aggregated CSV header
echo "part,field_size,num_threads,cycles,instructions,ipc,cache_misses,cache_references,cache_miss_rate,context_switches" > "$AGGREGATED_CSV"

TOTAL=$((3 * 4 * 4))
CURRENT=0

echo "Running experiments..."

for PART in A1 A2 A3; do
    for SIZE in "${SIZES[@]}"; do
        for NUM in "${THREADS[@]}"; do
            CURRENT=$((CURRENT + 1))
            echo "[$CURRENT/$TOTAL] $PART size=$SIZE threads=$NUM"
            
            # Run experiment and get output filename
            OUTPUT_FILE=$(sudo bash "$PART_B_SCRIPT" "$PART" "$SIZE" "$NUM")
            
            # Append to aggregated CSV (skip header, take data line)
            tail -1 "$OUTPUT_FILE" >> "$AGGREGATED_CSV"
            
            sleep 0.2
        done
    done
done

# Cleanup
make clean 2>/dev/null || true
sudo ip netns del ns1 2>/dev/null || true
sudo ip netns del ns2 2>/dev/null || true

echo ""
echo "=========================================="
echo "Experiments complete!"
echo "=========================================="
echo ""
echo "Individual result files (48 files):"
ls -1 ${ROLL_NUM}_Part_A?_size*_threads*.csv | head -10
echo "... ($(ls ${ROLL_NUM}_Part_A?_size*_threads*.csv 2>/dev/null | wc -l) total files)"
echo ""
echo "Aggregated results: $AGGREGATED_CSV"
echo ""
cat "$AGGREGATED_CSV"