#!/usr/bin/env python3
# MT25074_Part_D_Plots.py
# Part D: Plotting and Visualization
# Generates plots from Part C experimental results

import matplotlib.pyplot as plt
import numpy as np

# System configuration
SYSTEM_CONFIG = "Linux 6.8.0-100, perf tool, Network namespaces (ns1/ns2)"

# Hardcoded data from MT25074_Part_C_Results.csv
# Format: part, field_size, num_threads, cycles, instructions, ipc, cache_misses, cache_references, cache_miss_rate, context_switches

# A1 (Two-Copy) data
A1_field_size = [64, 64, 64, 64, 256, 256, 256, 256, 1024, 1024, 1024, 1024, 4096, 4096, 4096, 4096]
A1_num_threads = [1, 2, 4, 8, 1, 2, 4, 8, 1, 2, 4, 8, 1, 2, 4, 8]
A1_cycles = [39475195, 72915496, 145423266, 239993734, 57339554, 422731044, 196384431, 559373781, 
             481972275, 889358924, 1632860714, 3505906322, 21837116057, 39078594641, 66151152030, 112375457443]
A1_cache_misses = [1106106, 1852938, 4599321, 8261668, 1766876, 14328400, 6127387, 18548918,
                   11534759, 21468263, 38828402, 82573726, 451861353, 794469453, 1203171466, 1470317153]

# A2 (One-Copy) data
A2_field_size = [64, 64, 64, 64, 256, 256, 256, 256, 1024, 1024, 1024, 1024, 4096, 4096, 4096, 4096]
A2_num_threads = [1, 2, 4, 8, 1, 2, 4, 8, 1, 2, 4, 8, 1, 2, 4, 8]
A2_cycles = [42492945, 80720228, 154138040, 287522828, 64854783, 450791965, 201773672, 435136643,
             457718323, 1237521711, 2069915834, 4762915157, 21312543835, 38604787844, 74897780705, 104265049315]
A2_cache_misses = [1109381, 2130726, 4701976, 7935811, 1636074, 15007026, 6345088, 12662830,
                   10792756, 30842059, 51231940, 119332090, 457004495, 803483378, 1390654072, 1381411280]

# A3 (Zero-Copy) data
A3_field_size = [64, 64, 64, 64, 256, 256, 256, 256, 1024, 1024, 1024, 1024, 4096, 4096, 4096, 4096]
A3_num_threads = [1, 2, 4, 8, 1, 2, 4, 8, 1, 2, 4, 8, 1, 2, 4, 8]
A3_cycles = [47243393, 91816484, 164884933, 292013243, 164299763, 435833879, 781801562, 1724926034,
             311139173, 681113648, 1483712583, 2971613313, 20499727968, 37578446744, 75483270897, 109139400671]
A3_cache_misses = [1324059, 2856468, 4826505, 9849433, 5840901, 15492459, 25941890, 63279628,
                   8606617, 19603870, 42365662, 82880079, 658121770, 1090285843, 1883149653, 1770599067]

# Message sizes for plotting
message_sizes = [64, 256, 1024, 4096]
thread_counts = [1, 2, 4, 8]

def extract_by_size(data_dict, field_size_val):
    """Extract data for a specific field size"""
    result = []
    for i, size in enumerate(data_dict['field_size']):
        if size == field_size_val:
            result.append(data_dict['value'][i])
    return result

def extract_by_threads(data_dict, thread_val):
    """Extract data for a specific thread count"""
    result = []
    for i, threads in enumerate(data_dict['num_threads']):
        if threads == thread_val:
            result.append(data_dict['value'][i])
    return result

# Plot 1: Throughput vs Message Size
# Throughput calculated as: bytes per cycle (field_size / cycles)
# Higher throughput = more bytes transferred per cycle
def plot_throughput_vs_size():
    fig, ax = plt.subplots(figsize=(10, 6))
    
    # Calculate throughput for each size (average across thread counts for each size)
    A1_throughput = []
    A2_throughput = []
    A3_throughput = []
    
    for size in message_sizes:
        # Get all cycles for this size across all threads
        A1_cycles_for_size = [A1_cycles[i] for i in range(len(A1_field_size)) if A1_field_size[i] == size]
        A2_cycles_for_size = [A2_cycles[i] for i in range(len(A2_field_size)) if A2_field_size[i] == size]
        A3_cycles_for_size = [A3_cycles[i] for i in range(len(A3_field_size)) if A3_field_size[i] == size]
        
        # Calculate average throughput (bytes per cycle) - using harmonic mean for better representation
        A1_avg = size / np.mean(A1_cycles_for_size) if A1_cycles_for_size else 0
        A2_avg = size / np.mean(A2_cycles_for_size) if A2_cycles_for_size else 0
        A3_avg = size / np.mean(A3_cycles_for_size) if A3_cycles_for_size else 0
        
        A1_throughput.append(A1_avg * 1e9)  # Scale for readability
        A2_throughput.append(A2_avg * 1e9)
        A3_throughput.append(A3_avg * 1e9)
    
    ax.plot(message_sizes, A1_throughput, marker='o', label='A1 (Two-Copy)', linewidth=2)
    ax.plot(message_sizes, A2_throughput, marker='s', label='A2 (One-Copy)', linewidth=2)
    ax.plot(message_sizes, A3_throughput, marker='^', label='A3 (Zero-Copy)', linewidth=2)
    
    ax.set_xlabel('Message Size (bytes)', fontsize=12)
    ax.set_ylabel('Throughput (Normalized: bytes/cycle × 10⁹)', fontsize=12)
    ax.set_title('Throughput vs Message Size', fontsize=14, fontweight='bold')
    ax.legend(fontsize=10)
    ax.grid(True, alpha=0.3)
    ax.set_xscale('log', base=2)
    ax.set_xticks(message_sizes)
    ax.set_xticklabels([str(s) for s in message_sizes])
    
    # Add system config
    fig.text(0.5, 0.02, f'System: {SYSTEM_CONFIG}', ha='center', fontsize=9, style='italic')
    
    plt.tight_layout()
    plt.savefig('MT25074_Part_D_Plot1_Throughput_vs_Size.png', dpi=300, bbox_inches='tight')
    plt.close()
    print("Plot 1 saved: MT25074_Part_D_Plot1_Throughput_vs_Size.png")

# Plot 2: Latency vs Thread Count
# Latency represented as cycles (proxy metric)
def plot_latency_vs_threads():
    fig, ax = plt.subplots(figsize=(10, 6))
    
    # Use size=1024 as representative for latency analysis
    size_idx = 1024
    
    # Extract cycles for size=1024 across different thread counts
    A1_latency = []
    A2_latency = []
    A3_latency = []
    
    for threads in thread_counts:
        A1_cycles_val = [A1_cycles[i] for i in range(len(A1_field_size)) 
                         if A1_field_size[i] == size_idx and A1_num_threads[i] == threads]
        A2_cycles_val = [A2_cycles[i] for i in range(len(A2_field_size)) 
                         if A2_field_size[i] == size_idx and A2_num_threads[i] == threads]
        A3_cycles_val = [A3_cycles[i] for i in range(len(A3_field_size)) 
                         if A3_field_size[i] == size_idx and A3_num_threads[i] == threads]
        
        A1_latency.append(A1_cycles_val[0] if A1_cycles_val else 0)
        A2_latency.append(A2_cycles_val[0] if A2_cycles_val else 0)
        A3_latency.append(A3_cycles_val[0] if A3_cycles_val else 0)
    
    ax.plot(thread_counts, A1_latency, marker='o', label='A1 (Two-Copy)', linewidth=2)
    ax.plot(thread_counts, A2_latency, marker='s', label='A2 (One-Copy)', linewidth=2)
    ax.plot(thread_counts, A3_latency, marker='^', label='A3 (Zero-Copy)', linewidth=2)
    
    ax.set_xlabel('Number of Threads', fontsize=12)
    ax.set_ylabel('Latency (CPU Cycles)', fontsize=12)
    ax.set_title('Latency vs Thread Count (Message Size: 1024 bytes)', fontsize=14, fontweight='bold')
    ax.legend(fontsize=10)
    ax.grid(True, alpha=0.3)
    ax.set_xticks(thread_counts)
    
    # Add system config
    fig.text(0.5, 0.02, f'System: {SYSTEM_CONFIG}', ha='center', fontsize=9, style='italic')
    
    plt.tight_layout()
    plt.savefig('MT25074_Part_D_Plot2_Latency_vs_Threads.png', dpi=300, bbox_inches='tight')
    plt.close()
    print("Plot 2 saved: MT25074_Part_D_Plot2_Latency_vs_Threads.png")

# Plot 3: Cache Misses vs Message Size
def plot_cache_misses_vs_size():
    fig, ax = plt.subplots(figsize=(10, 6))
    
    # Calculate average cache misses for each size (across all thread counts)
    A1_cache_avg = []
    A2_cache_avg = []
    A3_cache_avg = []
    
    for size in message_sizes:
        A1_misses = [A1_cache_misses[i] for i in range(len(A1_field_size)) if A1_field_size[i] == size]
        A2_misses = [A2_cache_misses[i] for i in range(len(A2_field_size)) if A2_field_size[i] == size]
        A3_misses = [A3_cache_misses[i] for i in range(len(A3_field_size)) if A3_field_size[i] == size]
        
        A1_cache_avg.append(np.mean(A1_misses))
        A2_cache_avg.append(np.mean(A2_misses))
        A3_cache_avg.append(np.mean(A3_misses))
    
    ax.plot(message_sizes, A1_cache_avg, marker='o', label='A1 (Two-Copy)', linewidth=2)
    ax.plot(message_sizes, A2_cache_avg, marker='s', label='A2 (One-Copy)', linewidth=2)
    ax.plot(message_sizes, A3_cache_avg, marker='^', label='A3 (Zero-Copy)', linewidth=2)
    
    ax.set_xlabel('Message Size (bytes)', fontsize=12)
    ax.set_ylabel('Cache Misses (Average)', fontsize=12)
    ax.set_title('Cache Misses vs Message Size', fontsize=14, fontweight='bold')
    ax.legend(fontsize=10)
    ax.grid(True, alpha=0.3)
    ax.set_xscale('log', base=2)
    ax.set_xticks(message_sizes)
    ax.set_xticklabels([str(s) for s in message_sizes])
    ax.set_yscale('log')
    
    # Add system config
    fig.text(0.5, 0.02, f'System: {SYSTEM_CONFIG}', ha='center', fontsize=9, style='italic')
    
    plt.tight_layout()
    plt.savefig('MT25074_Part_D_Plot3_CacheMisses_vs_Size.png', dpi=300, bbox_inches='tight')
    plt.close()
    print("Plot 3 saved: MT25074_Part_D_Plot3_CacheMisses_vs_Size.png")

# Plot 4: CPU Cycles per Byte Transferred
def plot_cycles_per_byte():
    fig, ax = plt.subplots(figsize=(10, 6))
    
    # Calculate cycles per byte for each size (average across thread counts)
    A1_cpb = []
    A2_cpb = []
    A3_cpb = []
    
    for size in message_sizes:
        A1_cycles_for_size = [A1_cycles[i] for i in range(len(A1_field_size)) if A1_field_size[i] == size]
        A2_cycles_for_size = [A2_cycles[i] for i in range(len(A2_field_size)) if A2_field_size[i] == size]
        A3_cycles_for_size = [A3_cycles[i] for i in range(len(A3_field_size)) if A3_field_size[i] == size]
        
        A1_cpb.append(np.mean([c / size for c in A1_cycles_for_size]))
        A2_cpb.append(np.mean([c / size for c in A2_cycles_for_size]))
        A3_cpb.append(np.mean([c / size for c in A3_cycles_for_size]))
    
    ax.plot(message_sizes, A1_cpb, marker='o', label='A1 (Two-Copy)', linewidth=2)
    ax.plot(message_sizes, A2_cpb, marker='s', label='A2 (One-Copy)', linewidth=2)
    ax.plot(message_sizes, A3_cpb, marker='^', label='A3 (Zero-Copy)', linewidth=2)
    
    ax.set_xlabel('Message Size (bytes)', fontsize=12)
    ax.set_ylabel('CPU Cycles per Byte', fontsize=12)
    ax.set_title('CPU Cycles per Byte Transferred', fontsize=14, fontweight='bold')
    ax.legend(fontsize=10)
    ax.grid(True, alpha=0.3)
    ax.set_xscale('log', base=2)
    ax.set_xticks(message_sizes)
    ax.set_xticklabels([str(s) for s in message_sizes])
    
    # Add system config
    fig.text(0.5, 0.02, f'System: {SYSTEM_CONFIG}', ha='center', fontsize=9, style='italic')
    
    plt.tight_layout()
    plt.savefig('MT25074_Part_D_Plot4_CyclesPerByte.png', dpi=300, bbox_inches='tight')
    plt.close()
    print("Plot 4 saved: MT25074_Part_D_Plot4_CyclesPerByte.png")

if __name__ == "__main__":
    print("Generating Part D plots...")
    print("=" * 50)
    
    plot_throughput_vs_size()
    plot_latency_vs_threads()
    plot_cache_misses_vs_size()
    plot_cycles_per_byte()
    
    print("=" * 50)
    print("All plots generated successfully!")
