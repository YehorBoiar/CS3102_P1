import pandas as pd
import matplotlib.pyplot as plt
import seaborn as sns
import numpy as np

SMOL_DATA = 'data/data.csv'

def load_data(data_filename):
    df = pd.read_csv(data_filename) 
    
    # make all LOST rows to be NAN
    df['sequence_number'] = pd.to_numeric(df['sequence_number'], errors='coerce')
    
    # drop nan rows
    df = df.dropna(subset=['sequence_number'])
    df['sequence_number'] = df['sequence_number'].astype(int)

    # reindex to get a dataset with clean consequitive indexation
    full_range = range(0, 1201)
    df = df.set_index('sequence_number').reindex(full_range).reset_index()

    return df

def get_stats(df):
    total_sent = len(df)
    lost_packets = df[df['rtt_ms'].isna()]
    valid_packets = df[df['rtt_ms'].notna()]
    
    loss_rate = (len(lost_packets) / total_sent) * 100
    
    # Precision (absolute difference between consecutive RTTs)
    df['precision'] = df['rtt_ms'].diff().abs()

    # Application Throughput Calculation
    # 74 bytes = 32b payload + 8b UDP + 20b IP + 14b Ethernet
    # NOTE: To reference!!!
    valid_count = len(valid_packets)
    duration_secs = df['sequence_number'].max() if total_sent > 0 else 1
    throughput_bps = (valid_count * 74 * 8) / duration_secs

    stats = {
        'total_sent': total_sent,
        'total_lost': len(lost_packets),
        'loss_rate': loss_rate,
        'avg_precision': df['precision'].mean(),
        'avg_rtt': valid_packets['rtt_ms'].mean(),
        'median_rtt': valid_packets['rtt_ms'].median(),
        'p95_rtt': valid_packets['rtt_ms'].quantile(0.95),
        'max_rtt': valid_packets['rtt_ms'].max(),
        'min_rtt': valid_packets['rtt_ms'].min(),
        'throughput_bps': throughput_bps
    }

    return stats

def plot_scatter_loss(df, stats):
    valid = df[df['rtt_ms'].notna()]
    lost = df[df['rtt_ms'].isna()]

    plt.figure(figsize=(12, 6))
    
    # Valid RTTs
    plt.scatter(valid['sequence_number'], valid['rtt_ms'], 
                s=10, c='blue', alpha=0.5, label='Valid RTT')
    
    # Lost Packets (Red X)
    plt.scatter(lost['sequence_number'], [0] * len(lost), 
                s=30, c='red', marker='x', label='Packet Loss')

    plt.title(f"RTT Trace (Loss: {stats['loss_rate']:.2f}%, Precision: {stats['avg_precision']:.2f}ms)")
    plt.xlabel('Sequence Number')
    plt.ylabel('RTT (ms)')
    plt.legend()
    plt.grid(True, alpha=0.3)
    plt.savefig('1_scatter_loss.png')
    plt.show()

def plot_binned_boxplots(df, bin_size=60):
    df = df.copy()
    df = df[df['rtt_ms'].notna()]
    
    df['time_bin'] = (df['sequence_number'] // bin_size).astype(int)

    plt.figure(figsize=(12, 6))
    sns.boxplot(x='time_bin', y='rtt_ms', data=df,
                color='lightblue', showfliers=False)

    plt.title(f'RTT Stability over Time ({bin_size}s Bins)')
    plt.xlabel(f'Time Bin ({bin_size}s)')
    plt.ylabel('RTT (ms)')
    plt.grid(True, alpha=0.3)
    plt.savefig('2_boxplots_stability.png')

def plot_cdf(df):
    valid_rtt = df['rtt_ms'].dropna().sort_values()
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

# --- MAIN EXECUTION ---
if __name__ == "__main__":
    df = load_data(SMOL_DATA)

    stats = get_stats(df)

    print("="*40)
    print(f"Network Trace Analysis Report")
    print("="*40)
    print(f"Total Sent:     {stats['total_sent']}")
    print(f"Total Lost:     {stats['total_lost']} ({stats['loss_rate']:.2f}%)")

    print("-" * 20)
    print(f"RTT Min/Max:    {stats['min_rtt']:.2f} ms / {stats['max_rtt']:.2f} ms")
    print(f"Avg RTT:        {stats['avg_rtt']:.2f} ms")
    print(f"Median RTT:     {stats['median_rtt']:.2f} ms")
    print(f"95th Pctl RTT:  {stats['p95_rtt']:.2f} ms")
    print(f"Avg Precision:  {stats['avg_precision']:.2f} ms")
    print(f"Throughput:     {stats['throughput_bps']:.2f} bps")
    print("="*40)


    # # 3. Generate Plots
    plot_scatter_loss(df, stats)
    plot_binned_boxplots(df)
    plot_cdf(df)