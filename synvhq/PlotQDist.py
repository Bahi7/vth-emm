import pandas as pd
import matplotlib.pyplot as plt
import numpy as np

# Load the data
data = pd.read_csv('lambda128\\QueuesizeS12.csv')  # Adjust the path to your CSV file

print(data.describe(), "\n")
# Shift the DeltaTime column backward and replace the first element
data['DeltaTime'] = data['DeltaTime'].shift(-1)

# Drop the last row as it will now have NaN in DeltaTime
data.dropna(subset=['DeltaTime'], inplace=True)

# Calculate weighted statistics
queue_sizes = data['QueueSize']
delta_time = data['DeltaTime']

# Weighted mean and std
weighted_mean = np.average(queue_sizes, weights=delta_time)
weighted_std = np.sqrt(np.average((queue_sizes - weighted_mean)**2, weights=delta_time))

def weighted_median_percentile(data, weights, percentile=50):
    sorted_indices = np.argsort(data)
    sorted_data = data.iloc[sorted_indices]
    sorted_weights = weights.iloc[sorted_indices]
    cumsum_weights = np.cumsum(sorted_weights)
    cutoff = cumsum_weights.iloc[-1] * percentile / 100
    return sorted_data[cumsum_weights >= cutoff].iloc[0]


# Calculate weighted median and 75th percentile
weighted_median = weighted_median_percentile(queue_sizes, delta_time, 50)
weighted_75_percentile = weighted_median_percentile(queue_sizes, delta_time, 75)
weighted_25_percentile = weighted_median_percentile(queue_sizes, delta_time, 25)

# Min and Max
min_size = queue_sizes.min()
max_size = queue_sizes.max()

# Output the statistics
print(f"Weighted Mean: {weighted_mean}")
print(f"Weighted Std: {weighted_std}")
print(f"Weighted Median: {weighted_median}")
print(f"Weighted 75th Percentile: {weighted_75_percentile}")
print(f"Weighted 25th Percentile: {weighted_25_percentile}")
print(f"Min Size: {min_size}")
print(f"Max Size: {max_size}")

