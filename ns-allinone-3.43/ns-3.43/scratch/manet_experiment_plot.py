import pandas as pd
import matplotlib.pyplot as plt
import sys

if len(sys.argv) < 2:
    print("Please Specify CSV filename")
    print("To run the script run the following command: python3 scratch/manet_experiment_plot.py <CSV filename>")

else:
    # Load the CSV file
    csv_file_string = 'scratch/' + sys.argv[1]
    data = pd.read_csv(csv_file_string)

    # Ensure correct column names (modify if necessary)
    topology_col = 'Topology Size' 
    pdr_col = 'Packet Delivery Ratio (%)'  
    delay_col = 'Average End-to-End Delay (s)'  

    # Compute the average metrics for each topology size
    avg_metrics = data.groupby(topology_col)[[pdr_col, delay_col]].mean()

    # Plot the results
    fig, ax1 = plt.subplots()

    color = 'tab:blue'
    ax1.set_xlabel('Topology Size')
    ax1.set_ylabel('Packet Delivery Ratio (%)', color=color)
    ax1.plot(avg_metrics.index, avg_metrics[pdr_col], marker='o', linestyle='-', color=color, label='PDR')
    ax1.tick_params(axis='y', labelcolor=color)
    ax1.grid(True, linestyle='--', linewidth=0.5)

    ax2 = ax1.twinx()  # Create a second y-axis
    color = 'tab:red'
    ax2.set_ylabel('Average End-to-End Delay (s)', color=color)
    ax2.plot(avg_metrics.index, avg_metrics[delay_col], marker='s', linestyle='--', color=color, label='Delay')
    ax2.tick_params(axis='y', labelcolor=color)

    plt.title('Average Metrics per Topology Size')
    fig.tight_layout()
    plt.show()
