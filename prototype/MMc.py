import simpy
import pandas as pd
import numpy  as np
import scipy.stats as stats
import json
import os
import seaborn as sns
import matplotlib.pyplot as plt
from collections import Counter

def get_probabilities(lst):
    """
    Calculates the probabilities of each unique element in a list.
    
    Parameters:
    lst (list): A list of elements.
    
    Returns:
    dict: A dictionary with elements as keys and their probabilities as values.
    """
    counter = Counter(lst)
    total_count = len(lst)
    return {key: value / total_count for key, value in counter.items()}

# Get the Real distibution 
def get_dist(directory_path = "C:\\Users\\Enuma\\OneDrive\\Desktop\\100MB\\MM"):
    # Initialize an empty dictionary to store all data
    all_data = {}

    # Loop over each file in the directory
    for filename in os.listdir(directory_path):
        # Check if the file is a JSON file
        if filename.endswith(".json"):
            # Construct the full file path
            filepath = os.path.join(directory_path, filename)
            # Open the file
            with open(filepath, 'r') as file:
                # Load JSON data from file
                data = json.load(file)
                for key in data.keys():
                    if key in all_data.keys():
                        all_data[key] += data[key]
                    else:
                        all_data[key]=data[key]


    # Initializing an empty list to store the count of values
    response_lengths = []

    # Looping through each key-value pair in the dictionary
    for key, value in all_data.items():
        response_lengths.append(len(value))

    print(f"Number of Pairs: {sum(response_lengths)}")
    print(f"Max response length: {max(response_lengths)}")
    print(f"Number of Labels: {len(response_lengths)}")
    print(f"Number of distinct Labels (Size wise): {len(set(response_lengths))}")
    return get_probabilities(response_lengths)



# initialization module
NUMBER_SERVERS = 1
TOKEN_ARRIVAL_MEAN  = 2
SERVER_SERVICE_MEAN  = 800
SIM_TIME  = 10_000
arrivals, departures = [],[]
in_queue, in_system  = [],[]
tme_in_queue, len_in_queue = [],[]
batch_sizes = []


def token_arrival(env, number_servers, batch_dist):
    # IDs for tokens
    next_token_id = 0
    while True:
        batch_size = int(batch_dist.rvs())
        batch_sizes.append(batch_size)
        ## exponential distribution for arrivals
        next_token = stats.expon.rvs(scale = 1/TOKEN_ARRIVAL_MEAN)
        # Wait for the token
        yield env.timeout(next_token)
        time_of_arrival = env.now
        for _ in range(batch_size):
            arrivals.append(time_of_arrival)
            next_token_id += 1
            #print('%3d arrives at %.2f' % (next_token_id, env.now)) 
            env.process(processing(env, number_servers,next_token_id,time_of_arrival))
           
#..................................................................
def processing(env, number_servers, token_number, time_of_arrival):
    with servers_pool.request() as req:
        #print('%3d enters the queue at %.2f' % (token_number, env.now))
        queue_in = time_of_arrival
        length   = len(servers_pool.queue)
        tme_in_queue.append(queue_in)
        len_in_queue.append(length)
        yield req
        #print('%3d leaves the queue at %.2f' % (token_number, env.now))
        queue_out = env.now
        length    = len(servers_pool.queue)
        tme_in_queue.append(queue_out)
        len_in_queue.append(length)
        # normal distribution for the weighing process
        r_Markovian = stats.expon.rvs(scale =1/SERVER_SERVICE_MEAN)
        yield env.timeout(r_Markovian)
        #print('%3d permanece %.2f' % (token_number,r_Markovian))
        time_of_departure = env.now
        departures.append(time_of_departure)
        time_in_system = time_of_departure - time_of_arrival
        in_system.append(time_in_system)
        time_in_queue = queue_out - queue_in
        in_queue.append(time_in_queue)


def avg_line(df_length):
   ## finds the time average number of customers in the waiting line
   ## use the next row to figure out how long the queue was   
   df_length['delta_time'] = df_length['time'].shift(-1) - df_length['time']
   ## drop the last row because it would have an infinite delta time 
   df_length = df_length[0:-1]
   avg =np.average(df_length['Queue Lengths'],weights=df_length['delta_time'])
   return avg
#.................................................................
def server_utilization(df_length):
   ## finds the server utilization 
   sum_server_free = df_length[df_length['Queue Lengths']==0]  ['delta_time'].sum()
   ## the process begins with the server empty
   first_event =  df_length['time'].iloc[0]
   sum_server_free = sum_server_free + first_event
   utilization = round((1 - sum_server_free / SIM_TIME) * 100, 2)
   return utilization
#..................................................................
"""def not_allowed_perc(df_length):
   ## finds the percentage of time of tokens on queue not allowed to be waiting
   sum_not_allowed = df_length[df_length['len']>=  NOT_ALLOWED_NUMBER]['delta_time'].sum()
   not_allowed = round((sum_not_allowed / SIM_TIME) * 100, 2)
   return not_allowed"""
#.................................................................
## set up the environment
env = simpy.Environment()
## defining resources
servers_pool = simpy.Resource(env, capacity = NUMBER_SERVERS)
## selecting a random seed for the probability distributions
RANDOM_SEEDS = [1234, 5678, 9012, 3456, 7890]
np.random.seed(seed= RANDOM_SEEDS[1])

## defining the token arrival process
dist = 3#int(input("Choose your distribution:\n1-Zipf:  \n2-Uniform:  \n3-Geometric: \n4-Real Dist:\n"))
if dist == 1:
    s = float(input("Choose the s > 1 paremeter:"))
    batch_dist = stats.zipf(s)
elif dist == 2:
    a = 1#float(input("Choose the lower bound a:"))
    b = 5#float(input("Choose the upper bound b:"))
    batch_dist = stats.uniform(a, b)
elif dist == 3:
    p = 0.9#float(input("Choose the probability p:"))
    batch_dist = stats.geom(p)
elif dist == 4:
    #path = input("Paste the Multi-Map files path:")
    dist = get_dist()
    batch_dist = stats.rv_discrete(values=(list(dist.keys()),list(dist.values())))


expected_batch_size = batch_dist.mean()
# Calculate E[X^2]
expected_value_squared = batch_dist.expect(lambda x: x**2)

rho = TOKEN_ARRIVAL_MEAN / SERVER_SERVICE_MEAN  / NUMBER_SERVERS * expected_batch_size
env.process(token_arrival(env, servers_pool, batch_dist))
## run the simulation
env.run(until = SIM_TIME)

df1 = pd.DataFrame(tme_in_queue, columns = ['time'])
df2 = pd.DataFrame(len_in_queue, columns = ['Queue Lengths'])
df_length = pd.concat([df1, df2], axis = 1)
avg_length  = avg_line(df_length)
utilization = server_utilization(df_length)
df3 = pd.DataFrame(arrivals,   columns = ['arrivals'])
df4 = pd.DataFrame(departures, columns = ['departures'])
df_chart = pd.concat([df3, df4], axis = 1)
avg_delay_inqueue = np.mean(in_queue)
avg_delay_insyst  = np.mean(in_system)

print('Expected value of Batch size :%.2f' % (expected_batch_size))
print('Offer load: %.2f' %(rho))
print('The utilization of the server is %.2f' % (utilization))
print('The average delay in queue is %.2f'  % (avg_delay_inqueue))
print('The average delay in system is %.2f' % (avg_delay_insyst))
print('The average number of tokens in queue is %.2f' %  (avg_length))
print('Theoritical Lq : %.2f' %(rho/(1-rho)* (expected_batch_size + expected_value_squared) / 2 / expected_batch_size - rho))
print('Little Lq %.2f' %  (TOKEN_ARRIVAL_MEAN*expected_batch_size*avg_delay_inqueue))

#Calculate gamma
try:
    gamma_k = np.roll(batch_dist.pdf(batch_sizes), 1)/(1 - batch_dist.cdf(batch_sizes))
    x = min(batch_dist.pdf(batch_sizes))
except AttributeError:
    gamma_k = np.roll(batch_dist.pmf(batch_sizes), 1)/(1 - batch_dist.cdf(batch_sizes))
    x = min(batch_dist.pmf(batch_sizes))
#Removing infinite values, because cdf  equals 1 at some point    
gamma_k = gamma_k[np.isfinite(gamma_k)]

gamma_val = 1 - min(gamma_k)
print("---> gamma 1 - min(c_k+1/bar{c_k+1}): ", gamma_val, "\n---> gamma if we naively picked 1 - min(c_k): ", 1 - x,"\n---> lambda / mu: " ,TOKEN_ARRIVAL_MEAN/ SERVER_SERVICE_MEAN)
#Define the function for P(X > K)
def p_n(n, lambda_val, mu_val, alpha, rho_val):
    return n.apply(lambda x: 1 - rho_val if x == 0 else (1 - rho_val) * (lambda_val / mu_val) * (alpha + (lambda_val / mu_val))**(x - 1))


def prob_func(k, lambda_val, mu_val, gamma_val, rho_val):
    return (lambda_val / mu_val ) * ((lambda_val / mu_val  + gamma_val) ** (k-1)) / (1 - (lambda_val / mu_val + gamma_val) * (1 - rho_val))


# Extract the 'len' column to a list, which represents the queue lengths at each time step
queue_lengths = df_length['Queue Lengths'].tolist()
def px_n(n, lambda_val, mu_val, alpha, rho_val):
    if n == 0:
        return 1-rho_val
    return (1 - rho_val) * (lambda_val/mu_val) * (alpha + (lambda_val/mu_val))**(n-1)

# Assuming data_dict is your dictionary
Queue_dist = get_probabilities(queue_lengths)
for i in Queue_dist:
    print(i, px_n(i, TOKEN_ARRIVAL_MEAN, SERVER_SERVICE_MEAN, p, rho))


# Convert the dictionary to a DataFrame and sort by the outcome
df = pd.DataFrame(list(Queue_dist.items()), columns=['Queue_length', 'PMF']).sort_values('Queue_length')
#geometric PMF
df['P[X=n]'] = p_n(df['Queue_length'], TOKEN_ARRIVAL_MEAN, SERVER_SERVICE_MEAN, p, rho)
print(1- rho)
print(df.head(20))
print(df['P[X=n]'].sum(), df['PMF'].sum())

df['P[X>K]'] = prob_func(df['Queue_length'], TOKEN_ARRIVAL_MEAN, SERVER_SERVICE_MEAN, gamma_val, rho)
# Calculate the CDF
df['CDF'] = df['PMF'].cumsum()

# Calculate the complement of the CDF
df['CCDF'] = 1 - df['CDF']
print("-------Start Plotting-------")
# Define the number of bins
num_bins = 100
for key, value in Queue_dist.items():
    if value > 1:
        print(f"The value for {key} is larger than 1.")
        break

# Create bins for 'Queue_length'
df['Queue_length_bin'] = pd.cut(df['Queue_length'], bins=num_bins, labels=False)

# Group by the bins and calculate the means for 'PMF' and 'CCDF'
df_binned = df.groupby('Queue_length_bin').agg({'PMF': 'mean', 'CCDF': 'mean', 'P[X>K]': 'mean', 'P[X=n]': 'mean'}).reset_index()


# Create the first plot
fig, ax = plt.subplots()
ax.bar(df_binned['Queue_length_bin'], df_binned['CCDF'], color='blue', alpha=0.5, label='CCDF')
ax.bar(df_binned['Queue_length_bin'], df_binned['P[X>K]'], color='red', alpha=0.5, label='P[X>K]')
ax.set_xlabel('Queue_length')
ax.set_title('Theoretical P[X>K] vs Empirical P[X>K] (CCDF)')
ax.legend()



# Create the first plot
fig, ax = plt.subplots()
#ax.bar(df_binned['Queue_length_bin'], df_binned['PMF'], color='blue', alpha=0.5, label='PMF')
ax.bar(df_binned['Queue_length_bin'], df_binned['P[X=n]'], color='red', alpha=0.5, label='P[X=n]')
ax.set_xlabel('Queue_length')
ax.set_title('Theoretical P[X>K] vs Empirical P[X>K] (CCDF)')
ax.legend()


# Create the second plot
fig, ax = plt.subplots()
ax.bar(df_binned['Queue_length_bin'],df_binned['PMF'], label='PMF')
ax.set_xlabel('Queue_length')
ax.set_title('PMF for each Queue size')

plt.show()



