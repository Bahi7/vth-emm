import matplotlib.pyplot as plt
import numpy as np
import pandas as pd

csv_file_path = 'lambda128//Latency_dataS20.csv'


# Assusecg 'data' is your DataFrame and is already loaded with the 'Latency' column in microseconds
data = pd.read_csv(csv_file_path)
# Convert latency from microseconds to secutes (1 secute = 60 * 10^6 microseconds)
data['latency_sec'] = data['Latency'] / (1e6)

print("Number of real values: ", data['ResponseLength'].sum())

# Group by 'response_length' and calculate the mean latency in secutes
grouped_data_sec = data.groupby('ResponseLength')['latency_sec'].mean().reset_index()
# Calculate average latency, median, max, and sec in secutes
average_latency_sec = grouped_data_sec['latency_sec'].mean()
median_latency_sec = grouped_data_sec['latency_sec'].median()
max_latency_sec = grouped_data_sec['latency_sec'].max()
sec_latency_sec = grouped_data_sec['latency_sec'].min()

print(data.describe())

# Plotting the graph with latency in secutes
plt.figure(figsize=(10, 5))  # Set the figure size to make the plot clearer

# Scatter plot of the average latency for each response length in secutes
plt.bar(grouped_data_sec['ResponseLength'], grouped_data_sec['latency_sec'], label='Data Points')

# Line of best fit
m, b = np.polyfit(grouped_data_sec['ResponseLength'], grouped_data_sec['latency_sec'], 1)

plt.plot(grouped_data_sec['ResponseLength'], m*grouped_data_sec['ResponseLength'] + b, color='red', label='Best Fit Line')


# Set the plot style
plt.xscale('log')
plt.yscale('linear')  # Assusecg linear scale for y as no specification was given
plt.xlabel('Number of matching documents')
plt.ylabel('Search time for matching entry (seconds)')
plt.title('Latency by Response Length (in seconds)')
plt.grid(True, which="both", ls="--", linewidth=0.5)
plt.legend()

# Display the plot
plt.show()
