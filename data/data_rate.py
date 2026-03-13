import pandas as pd

# 1. Load the data and filter for successful probes
df = pd.read_csv('data/fullpe_data.csv')
lost_packets = df[df['rtt_ms'].isna()]
loss_rate = (len(lost_packets) / len(df)) * 100

df = df[df['status'] == 'SUCCESS']


df_small = df[df['sequence_number'] % 2 == 0]
df_large = df[df['sequence_number'] % 2 != 0]

size_small = df_small['bytes_received'].iloc[0]
size_large = df_large['bytes_received'].iloc[0]

bytes = (size_large - size_small) * 8

df_correcection = pd.read_csv('data/nofullpe_data.csv')
df_correcection = df_correcection[df_correcection['status'] == 'SUCCESS']
df_correcection_small = df_correcection[df_correcection['sequence_number'] % 2 == 0]
df_correcection_large = df_correcection[df_correcection['sequence_number'] % 2 != 0]

med_small = df_small['rtt_ms'].median()
med_large = df_large['rtt_ms'].median()

med_corr_small = df_correcection_small['rtt_ms'].median()
med_corr_large = df_correcection_large['rtt_ms'].median()

# 2. Apply your asymmetric correction
rtt_small_corrected = (med_small - (med_corr_small / 2)) / 1000.0
rtt_large_corrected = (med_large - (med_corr_large / 2)) / 1000.0

# Calculate one-way delay difference
T_d_path = rtt_large_corrected - rtt_small_corrected

# Assume local transmission delay (e.g., local 1 Gbps link)
T_x = bytes / 1e9

# Calculate bottleneck delay
T_p = T_d_path - T_x

# Calculate final data rate in bps, then convert to Mbps
r_bps = bytes / T_p
r_mbps = r_bps / 1e6

print(f"Loss Rate: {loss_rate}")
print(f"Median RTT Small: {rtt_small_corrected * 1000:.2f} ms")
print(f"Median RTT Large: {rtt_large_corrected * 1000:.2f} ms")
print(f"Estimated Data Rate: {r_mbps:.2f} Mbps")
