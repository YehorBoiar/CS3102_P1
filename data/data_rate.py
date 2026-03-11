import pandas as pd

# 1. Load the data and filter for successful probes
df = pd.read_csv('var_data.csv')
df = df[df['status'] == 'SUCCESS']

# 2. Separate into small (even) and large (odd) packet groups
df_small = df[df['sequence_number'] % 2 == 0]
df_large = df[df['sequence_number'] % 2 != 0]

# Extract the sizes directly from the dataframe
size_small = df_small['bytes_received'].iloc[0]
size_large = df_large['bytes_received'].iloc[0]

# 3. Get median RTTs (in seconds) to account for outliers
median_rtt_small = df_small['rtt_ms'].median() / 1000.0
median_rtt_large = df_large['rtt_ms'].median() / 1000.0

# 4. Apply lecture formulas
# Calculate difference in bits
b = (size_large - size_small) * 8

# Calculate one-way delay difference
T_d_path = (median_rtt_large - median_rtt_small) / 2

# Assume local transmission delay (e.g., local 1 Gbps link)
T_x = b / 1e9

# Calculate bottleneck delay
T_p = T_d_path - T_x

# Calculate final data rate in bps, then convert to Mbps
r_bps = b / T_p
r_mbps = r_bps / 1e6

print(f"Median RTT Small: {median_rtt_small * 1000:.2f} ms")
print(f"Median RTT Large: {median_rtt_large * 1000:.2f} ms")
print(f"Estimated Data Rate: {r_mbps:.2f} Mbps")