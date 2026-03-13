import pandas as pd

# 1. Load the data and filter for successful probes
df = pd.read_csv('data/fullpe_data.csv')
df = df[df['status'] == 'SUCCESS']


loss_rate = (len(lost_packets) / total_sent) * 100

df_small = df[df['sequence_number'] % 2 == 0]
df_large = df[df['sequence_number'] % 2 != 0]

size_small = df_small['bytes_received'].iloc[0]
size_large = df_large['bytes_received'].iloc[0]

b = (size_large - size_small) * 8


# dataframe for rtt correction. Subtract one way TO THE SLURPE. 
# This is very minor, but good to have
df_correcection = pd.read_csv('nofullpe_data.csv')
df_correcection = df_correcection[df_correcection['status'] == 'SUCCESS']
df_correcection_small = df_correcection[df_correcection['sequence_number'] % 2 == 0]
df_correcection_large = df_correcection[df_correcection['sequence_number'] % 2 != 0]

rtt_small_corrected = df_small['rtt_ms'] - df_correcection_small['rtt_ms'] / 2
rtt_large_corrected = df_large['rtt_ms'] - df_correcection_large['rtt_ms'] / 2

# get median rtt from corrected 
median_rtt_small = rtt_small_corrected.median() / 1000.0
median_rtt_large = rtt_large_corrected.median() / 1000.0


# Calculate one-way delay difference
T_d_path = (median_rtt_large - median_rtt_small)

# Assume local transmission delay (e.g., local 1 Gbps link)
T_x = b / 1e9

# Calculate bottleneck delay
T_p = T_d_path - T_x

# Calculate final data rate in bps, then convert to Mbps
r_bps = b / T_p
r_mbps = r_bps / 1e6

print(f"Loss Rate: {loss_rate}")
print(f"Median RTT Small: {median_rtt_small * 1000:.2f} ms")
print(f"Median RTT Large: {median_rtt_large * 1000:.2f} ms")
print(f"Estimated Data Rate: {r_mbps:.2f} Mbps")
