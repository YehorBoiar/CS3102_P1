import pandas as pd
import matplotlib.pyplot as plt
import seaborn as sns
import numpy as np

def load_data():
    df = pd.read_csv('data/data.csv', 
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
    # get loss rate
    total_sent = len(df)
    lost_packets = df[df['time'].isna()]
    loss_rate = (len(lost_packets) / total_sent) * 100
    
    # get precision
    df['precision'] = df['time'].diff().abs()

    stats = {
        'total_sent': total_sent,
        'total_lost': len(lost_packets),
        'loss_rate': loss_rate,
        'avg_precision': df['precision'].mean()
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
    """
    Plots the Cumulative Distribution Function.
    Good for showing that delay is not Gaussian.
    """
    valid_rtt = df['time'].dropna().sort_values()
    yvals = np.arange(len(valid_rtt)) / float(len(valid_rtt) - 1)

    plt.figure(figsize=(8, 6))
    plt.plot(valid_rtt, yvals, marker='.', linestyle='none', markersize=2)
    
    # Add median line
    median_rtt = valid_rtt.median()
    plt.axvline(median_rtt, color='r', linestyle='--', label=f'Median: {median_rtt}ms')
    
    plt.title('CDF of Round Trip Time')
    plt.xlabel('RTT (ms)')
    plt.ylabel('Probability (0.0 - 1.0)')
    plt.grid(True)
    plt.legend()
    plt.savefig('3_cdf_plot.png')
    plt.show()

# --- MAIN EXECUTION ---
if __name__ == "__main__":
    # 1. Load
    df = load_data()
    stats = get_stats(df)

    # 2. Print Text Report
    print("="*40)
    print(f"Network Trace Analysis")
    print("="*40)
    print(f"Total Sent:     {stats['total_sent']}")
    print(f"Total Lost:     {stats['total_lost']} ({stats['loss_rate']:.2f}%)")
    print(f"Average precision: {stats['avg_precision']:.3f} ms")
    print("="*40)

    # # 3. Generate Plots
    plot_scatter_loss(df, stats)
    plot_binned_boxplots(df)
    plot_cdf(df)