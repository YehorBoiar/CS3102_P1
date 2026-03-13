import pandas as pd
import matplotlib.pyplot as plt
import seaborn as sns
import numpy as np

FULLPE_DATA = 'data/fullpe_data.csv'
NOFULLPE_DATA = 'data/nofullpe_data.csv'

def load_and_correct_data(fullpe_path, baseline_path):
    # Load both datasets
    df_f = pd.read_csv(fullpe_path)
    df_b = pd.read_csv(baseline_path)
    
    # Standardize sequence numbers
    for d in [df_f, df_b]:
        d['sequence_number'] = pd.to_numeric(d['sequence_number'], errors='coerce')
        d.dropna(subset=['sequence_number'], inplace=True)
        d['sequence_number'] = d['sequence_number'].astype(int)

    # Reindex to ensure alignment
    full_range = range(0, 1201)
    df_f = df_f.set_index('sequence_number').reindex(full_range).reset_index()
    df_b = df_b.set_index('sequence_number').reindex(full_range).reset_index()

    # Create the "Corrected" RTT column
    # We subtract the full baseline RTT from the fullpe RTT
    df_f['rtt_corrected'] = df_f['rtt_ms'] - df_b['rtt_ms']
    
    # Categorize packet sizes
    df_f['packet_type'] = np.where(df_f['sequence_number'] % 2 == 0, 'Small', 'Large')
    
    return df_f

def plot_scatter_loss(df):
    plt.figure(figsize=(12, 6))
    
    # Note: Using rtt_corrected now
    small_valid = df[(df['rtt_corrected'].notna()) & (df['packet_type'] == 'Small')]
    large_valid = df[(df['rtt_corrected'].notna()) & (df['packet_type'] == 'Large')]
    lost = df[df['rtt_ms'].isna()]

    plt.scatter(small_valid['sequence_number'], small_valid['rtt_corrected'], 
                s=10, c='blue', alpha=0.5, label='Corrected RTT (Small)')
    plt.scatter(large_valid['sequence_number'], large_valid['rtt_corrected'], 
                s=10, c='green', alpha=0.5, label='Corrected RTT (Large)')
    
    # Mark loss at 0 on the Y-axis
    plt.scatter(lost['sequence_number'], [0] * len(lost), 
                s=30, c='red', marker='x', label='Packet Loss')

    plt.title('Corrected RTT (Emulation Delay Only) over Time')
    plt.xlabel('Sequence Number')
    plt.ylabel('Corrected RTT (ms)')
    plt.legend()
    plt.grid(True, alpha=0.3)
    plt.savefig('1_scatter_loss_corrected.png')
    plt.show()

def plot_binned_boxplots(df, bin_size=60):
    df_plot = df[df['rtt_corrected'].notna()].copy()
    df_plot['time_bin'] = (df_plot['sequence_number'] // bin_size).astype(int)

    plt.figure(figsize=(12, 6))
    sns.boxplot(x='time_bin', y='rtt_corrected', hue='packet_type', data=df_plot,
                palette={'Small': 'lightblue', 'Large': 'lightgreen'}, showfliers=False)

    plt.title(f'Corrected RTT Stability ({bin_size}s Bins)')
    plt.xlabel('Time Bin')
    plt.ylabel('Corrected RTT (ms)')
    plt.legend(title='Packet Size')
    plt.grid(True, alpha=0.3)
    plt.savefig('2_boxplots_corrected.png')
    plt.show()

def plot_cdf(df):
    plt.figure(figsize=(8, 6))

    for p_type, color in [('Small', 'blue'), ('Large', 'green')]:
        valid_rtt = df[df['packet_type'] == p_type]['rtt_corrected'].dropna().sort_values()
        yvals = np.arange(len(valid_rtt)) / float(len(valid_rtt) - 1)

        plt.plot(valid_rtt, yvals, marker='.', linestyle='none', markersize=2, 
                 color=color, label=f'{p_type} Packets')
        
        median_val = valid_rtt.median()
        plt.axvline(median_val, color=color, linestyle='--', alpha=0.7, 
                    label=f'Med {p_type}: {median_val:.2f}ms')
    
    plt.title('CDF of Corrected Round Trip Time')
    plt.xlabel('Corrected RTT (ms)')
    plt.ylabel('Probability')
    plt.grid(True)
    plt.legend()
    plt.savefig('3_cdf_corrected.png')
    plt.show()

if __name__ == "__main__":
    # Load using both files to perform the subtraction
    df = load_and_correct_data(FULLPE_DATA, NOFULLPE_DATA)

    plot_scatter_loss(df)
    plot_binned_boxplots(df)
    plot_cdf(df)