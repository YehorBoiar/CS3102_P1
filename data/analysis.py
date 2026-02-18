import pandas as pd
import matplotlib.pyplot as plt
import numpy as np

data_file = 'data/data.csv'

valid_rows = []
loss_count_explicit = 0

with open(data_file, 'r') as f:
    headers = f.readline().strip().split(',')
    
    for line in f:
        line = line.strip()
        if not line: continue
        
        if line[0].isdigit():
            valid_rows.append(line.split(','))
        else:
            # capture "LOST (Timeout)" line
            loss_count_explicit += 1

# Create DataFrame from valid rows
df = pd.DataFrame(valid_rows, columns=['counter', 'time', 'bytes_received'])

# Convert columns to numeric types
df['counter'] = pd.to_numeric(df['counter'])
df['time'] = pd.to_numeric(df['time']) # RTT in ms
df['bytes_received'] = pd.to_numeric(df['bytes_received'])

# --- ANALYSIS ---

# 1. LOSS CALCULATION
min_counter = df['counter'].min()
max_counter = df['counter'].max()
total_expected_packets = max_counter - min_counter + 1
total_received_packets = len(df)
total_lost_packets = total_expected_packets - total_received_packets

loss_percentage = (total_lost_packets / total_expected_packets) * 100

# 2. DELAY (LATENCY) CHARACTERISATION
min_rtt = df['time'].min()
max_rtt = df['time'].max()
avg_rtt = df['time'].mean()
std_dev_rtt = df['time'].std() # Jitter metric

# 3. END-TO-END DATA RATE (THROUGHPUT)
# Throughput = Total Bits Received / Total Time Elapsed
total_bytes = df['bytes_received'].sum()
total_bits = total_bytes * 8

duration_seconds = total_expected_packets
throughput_bps = total_bits / duration_seconds

# --- OUTPUT RESULTS ---
print("="*40)
print(f"Network Path Analysis (FullPE)")
print("="*40)
print(f"Total Packets Sent (Est): {total_expected_packets}")
print(f"Total Packets Received:   {total_received_packets}")
print(f"Packets Lost:             {total_lost_packets}")
print(f"Loss Rate:                {loss_percentage:.2f}%")
print("-" * 40)
print(f"Min Delay (RTT):          {min_rtt:.3f} ms")
print(f"Max Delay (RTT):          {max_rtt:.3f} ms")
print(f"Avg Delay (RTT):          {avg_rtt:.3f} ms")
print(f"Jitter (Std Dev):         {std_dev_rtt:.3f} ms")
print("-" * 40)
print(f"Total Data Received:      {total_bytes / 1024:.2f} KB")
print(f"Est. Duration:            {duration_seconds:.2f} s")
print(f"End-to-End Data Rate:     {throughput_bps:.2f} bps")
print("="*40)

# # --- PLOTTING ---
# plt.figure(figsize=(12, 6))

# # Plot RTT
# plt.plot(df['counter'], df['time'], label='RTT', marker='o', markersize=2, linestyle='-', linewidth=0.5, alpha=0.7)

# # Highlight Average
# plt.axhline(y=avg_rtt, color='r', linestyle='--', label=f'Avg RTT ({avg_rtt:.2f}ms)')

# plt.title('Round Trip Time (RTT) over 20 Minute Trace')
# plt.xlabel('Packet Sequence Number')
# plt.ylabel('RTT (ms)')
# plt.grid(True, which='both', linestyle='--', linewidth=0.5)
# plt.legend()

# # Save the plot for the report
# plt.savefig('rtt_analysis.png')
# print("Plot saved to 'rtt_analysis.png'")
# plt.show()