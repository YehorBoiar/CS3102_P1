import pandas as pd
import matplotlib.pyplot as plt
import seaborn as sns
import numpy as np

SMOL_DATA = 'data/data.csv'
BIG_DATA = 'data/data_big.csv'

def load_data(data_filename):
    df = pd.read_csv(data_filename, 
                     names=['counter', 'time', 'bytes_received'], 
                     on_bad_lines='skip') 
    
    
    # make all LOST rows to be NAN
    df['counter'] = pd.to_numeric(df['counter'], errors='coerce')
    
    # drop nan rows
    df = df.dropna(subset=['counter'])
    df['counter'] = df['counter'].astype(int)

    # reindex to get a dataset with clean consequitive indexation
    full_range = range(0, 1201)
    df = df.set_index('counter').reindex(full_range).reset_index()

    return df

def get_stats(df):
    total_sent = len(df)
    lost_packets = df[df['time'].isna()]
    valid_packets = df[df['time'].notna()]
    
    loss_rate = (len(lost_packets) / total_sent) * 100
    
    # get precision
    df['precision'] = df['time'].diff().abs()

    stats = {
        'total_sent': total_sent,
        'total_lost': len(lost_packets),
        'loss_rate': loss_rate,
        'avg_precision': df['precision'].mean(),
        'avg_rtt': valid_packets['time'].mean(),
        'max_rtt': valid_packets['time'].max(),
        'min_rtt': valid_packets['time'].min()
    }

    return stats

def plot_scatter_loss(df, stats):
    valid = df[df['time'].notna()]
    lost = df[df['time'].isna()]

    plt.figure(figsize=(12, 6))
    
    # Valid RTTs
    plt.scatter(valid['counter'], valid['time'], 
                s=10, c='blue', alpha=0.5, label='Valid RTT')
    
    # Lost Packets (Red X)
    plt.scatter(lost['counter'], [0] * len(lost), 
                s=30, c='red', marker='x', label='Packet Loss')

    plt.title(f"RTT Trace (Loss: {stats['loss_rate']:.2f}%, Jitter: {stats['avg_precision']:.2f}ms)")
    plt.xlabel('Sequence Number')
    plt.ylabel('RTT (ms)')
    plt.legend()
    plt.grid(True, alpha=0.3)
    plt.savefig('1_scatter_loss.png')
    plt.show()

def plot_binned_boxplots(df, bin_size=60):
    """
    Groups data into bins (default 60s/1min) and shows distribution stability.
    Good for detecting transient instability.
    """
    # Create a bin column (e.g., Minute 1, Minute 2...)
    df['time_bin'] = (df['counter'] // bin_size).astype(int)
    
    plt.figure(figsize=(12, 6))
    sns.boxplot(x='time_bin', y='time', data=df, color='lightblue', showfliers=False)
    
    plt.title(f'RTT Stability over Time ({bin_size}s Bins)')
    plt.xlabel(f'Time Bin ({bin_size}s)')
    plt.ylabel('RTT (ms)')
    plt.grid(True, alpha=0.3)
    plt.savefig('2_boxplots_stability.png')
    plt.show()

def plot_cdf(df):
    valid_rtt = df['time'].dropna().sort_values()
    yvals = np.arange(len(valid_rtt)) / float(len(valid_rtt) - 1)

    plt.figure(figsize=(8, 6))
    plt.plot(valid_rtt, yvals, marker='.', linestyle='none', markersize=2)
    
    # Add median line
    median_rtt = valid_rtt.median()
    plt.axvline(median_rtt, color='r', linestyle='--', label=f'Median: {median_rtt}ms')
    
    plt.title('CDF of Round Trip Time')
    plt.xlabel('RTT (ms)')
    plt.ylabel('Probability')
    plt.grid(True)
    plt.legend()
    plt.savefig('3_cdf_plot.png')
    plt.show()

def estimate_bandwidth(df_small, df_large):
    """
    Calculates Bandwidth
    """
    print("="*40)
    print("Bandwidth Estimation")
    print("="*40)

    # 1. Filter valid packets
    valid_s = df_small.dropna(subset=['time', 'bytes_received'])
    valid_l = df_large.dropna(subset=['time', 'bytes_received'])

    if valid_s.empty or valid_l.empty:
        print("Error: One of the datasets is empty or has 100% packet loss.")
        return

    # 2. Get the Packet Sizes (Mode or Median to be safe)
    # We use mode() because size should be constant in a ping test
    size_s = valid_s['bytes_received'].mode()[0]
    size_l = valid_l['bytes_received'].mode()[0]

    # 3. Get the Median RTTs
    rtt_s = valid_s['time'].median()
    rtt_l = valid_l['time'].median()

    print(f"Small Dataset: {size_s} bytes | Median RTT: {rtt_s:.2f} ms")
    print(f"Large Dataset: {size_l} bytes | Median RTT: {rtt_l:.2f} ms")

    if size_l <= size_s:
        print("\nERROR: 'Large' packet size must be greater than 'Small'.")
        print("Check your file inputs.")
        return

    # --- THE MATH (Slide 39) ---
    
    # 1. Delta Bits (b)
    # The 'bytes_received' in ping usually includes IP header (20) + ICMP header (8).
    # The formula relies on the physical difference in bits on the wire.
    b_bits = (size_l - size_s) * 8

    # 2. Delta Time (Tp in your slide context for difference)
    # We need the One-Way delay difference.
    # Since we have RTT, we divide the difference by 2.
    delta_rtt_ms = (rtt_l - rtt_s)
    
    # Sanity Check: Did large packets actually take longer?
    if delta_rtt_ms <= 0:
        print("\nWARNING: Large packets were faster or equal to small packets.")
        print(f"Delta RTT: {delta_rtt_ms:.2f} ms")
        print("Conclusion: Bottleneck bandwidth is likely higher than measurement precision allows.")
        return

    # Convert ms to seconds
    # Td_path = (median(Td_large) - median(Td_small)) / 2
    delta_oneway_sec = (delta_rtt_ms / 2) / 1000.0

    # 3. Estimate Rate (r = b / Tp)
    bandwidth_bps = b_bits / delta_oneway_sec
    bandwidth_mbps = bandwidth_bps / 1_000_000

    print("-" * 20)
    print(f"Results:")
    print(f"  Delta Bits:          {b_bits} bits")
    print(f"  Delta One-Way Time:  {delta_oneway_sec*1000:.4f} ms")
    print(f"  Estimated Bandwidth: {bandwidth_mbps:.2f} Mbps")
    print("="*40)

# --- MAIN EXECUTION ---
if __name__ == "__main__":
    df = load_data(SMOL_DATA)
    df2 = load_data(BIG_DATA)
    stats = get_stats(df)

    print("="*40)
    print(f"Network Trace Analysis Report")
    print("="*40)
    print(f"Total Sent:     {stats['total_sent']}")
    print(f"Total Lost:     {stats['total_lost']} ({stats['loss_rate']:.2f}%)")

    print("-" * 20)
    print(f"RTT Min/Max:    {stats['min_rtt']:.2f} ms / {stats['max_rtt']:.2f} ms")
    print(f"Avg RTT:        {stats['avg_rtt']:.2f} ms")
    print(f"Avg precision:     {stats['avg_precision']:.2f} ms")
    print("="*40)


    estimate_bandwidth(df, df2)


    # # 3. Generate Plots
    plot_scatter_loss(df, stats)
    plot_binned_boxplots(df)
    plot_cdf(df)