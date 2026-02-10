#!/bin/bash
# MT25074_Part_B_Run_Single_Experiment.sh
# Runs single experiment, outputs CSV file with encoded parameters in filename

ROLL_NUM="MT25074"

if [ $# -ne 3 ]; then
    echo "Usage: $0 <A1|A2|A3> <field_size> <num_threads>" >&2
    exit 1
fi

PART=$1
FIELD_SIZE=$2
NUM_CLIENTS=$3

SERVER_BIN="./${ROLL_NUM}_Part_${PART}_Server"
CLIENT_BIN="./${ROLL_NUM}_Part_${PART}_Client"

# Output filename with encoded parameters
OUTPUT_FILE="${ROLL_NUM}_Part_${PART}_size${FIELD_SIZE}_threads${NUM_CLIENTS}.csv"

# Temp file for perf
PERF_TMP=$(mktemp /tmp/perf_XXXXXX)

# Run server with perf
sudo ip netns exec ns1 /usr/lib/linux-tools-6.8.0-100/perf stat \
    -e cycles,cache-misses,instructions,cache-references,context-switches \
    -o "$PERF_TMP" \
    "$SERVER_BIN" "$FIELD_SIZE" "$NUM_CLIENTS" > /dev/null 2>&1 &
SERVER_PID=$!

sleep 0.5

# Check if server started
if ! kill -0 $SERVER_PID 2>/dev/null; then
    echo "ERROR: Server failed to start" >&2
    rm -f "$PERF_TMP"
    exit 1
fi

# Launch clients
for ((i=1; i<=NUM_CLIENTS; i++)); do
    sudo ip netns exec ns2 "$CLIENT_BIN" "$FIELD_SIZE" > /dev/null 2>&1 &
done

# Wait for server to finish
wait $SERVER_PID 2>/dev/null

# Parse perf output - MUST exclude "time elapsed" line which also contains "cycles"
CYCLES=$(grep "cycles" "$PERF_TMP" | grep -v "time elapsed" | head -1 | awk '{print $1}' | tr -d ',')
INSTRUCTIONS=$(grep "instructions" "$PERF_TMP" | head -1 | awk '{print $1}' | tr -d ',')
CACHE_MISSES=$(grep "cache-misses" "$PERF_TMP" | head -1 | awk '{print $1}' | tr -d ',')
CACHE_REFS=$(grep "cache-references" "$PERF_TMP" | head -1 | awk '{print $1}' | tr -d ',')
CONTEXT_SWITCHES=$(grep "context-switches" "$PERF_TMP" | head -1 | awk '{print $1}' | tr -d ',')

# Default to 0 if empty
CYCLES=${CYCLES:-0}
INSTRUCTIONS=${INSTRUCTIONS:-0}
CACHE_MISSES=${CACHE_MISSES:-0}
CACHE_REFS=${CACHE_REFS:-0}
CONTEXT_SWITCHES=${CONTEXT_SWITCHES:-0}

# Calculate derived metrics
if [[ "$CYCLES" != "0" && -n "$CYCLES" ]]; then
    IPC=$(echo "scale=4; $INSTRUCTIONS / $CYCLES" | bc 2>/dev/null || echo "0")
else
    IPC="0"
fi

if [[ "$CACHE_REFS" != "0" && -n "$CACHE_REFS" ]]; then
    MISS_RATE=$(echo "scale=4; $CACHE_MISSES / $CACHE_REFS * 100" | bc 2>/dev/null || echo "0")
else
    MISS_RATE="0"
fi

# Write CSV
echo "part,field_size,num_threads,cycles,instructions,ipc,cache_misses,cache_references,cache_miss_rate,context_switches" > "$OUTPUT_FILE"
echo "$PART,$FIELD_SIZE,$NUM_CLIENTS,$CYCLES,$INSTRUCTIONS,$IPC,$CACHE_MISSES,$CACHE_REFS,$MISS_RATE,$CONTEXT_SWITCHES" >> "$OUTPUT_FILE"

rm -f "$PERF_TMP"
echo "$OUTPUT_FILE"
