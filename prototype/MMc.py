import simpy.rt
import pandas as pd
import numpy  as np
import scipy.stats as stats
import json
import os
import seaborn as sns
import matplotlib.pyplot as plt
from collections import Counter

def get_probabilities_df(df):
    weights = df.groupby('Queue Lengths')['delta_time'].sum()
    probabilities = weights / weights.sum()
   
    return probabilities
    
#.................................................................
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
#.................................................................
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
#.................................................................


# initialization module
NUMBER_SERVERS = 1
TOKEN_ARRIVAL_MEAN  = 1
SERVER_SERVICE_MEAN  = 100
SIM_TIME  = 50_000
arrivals, departures = [],[]
in_queue, in_system  = [],[]
tme_in_queue, len_in_queue = [],[]
batch_sizes = []


def token_arrival(env, number_servers, batch_dist):

    while True:
        #get the batch That arrived
        batch_size = int(batch_dist.rvs())


        #I store the batch sizes for later use
        batch_sizes.append(batch_size)

        ## exponential distribution for arrivals
        next_token_time = stats.expon.rvs(scale = 1/TOKEN_ARRIVAL_MEAN)

        # Wait for the token
        yield env.timeout(next_token_time)


        length  = len(servers_pool.queue)
        for _ in range(batch_size):
            time_of_arrival = env.now
            arrivals.append(time_of_arrival)
            env.process(processing(env, number_servers, time_of_arrival))
           
#..................................................................
def processing(env, number_servers, time_of_arrival):
    with servers_pool.request() as req: # Generate a request event

        queue_in = time_of_arrival
        length  = len(servers_pool.queue)
        tme_in_queue.append(queue_in)
        len_in_queue.append(length)
        
        yield req  # Wait for access
        queue_out = env.now
        length    = len(servers_pool.queue)
        tme_in_queue.append(queue_out)
        len_in_queue.append(length)
       

        # service distribution 
        r_Markovian = stats.expon.rvs(scale =1/SERVER_SERVICE_MEAN)
        
        yield env.timeout(r_Markovian) # Do something


        # Resource released automatically

        time_of_departure = env.now
        departures.append(time_of_departure)
        time_in_system = time_of_departure - time_of_arrival
        in_system.append(time_in_system)
        time_in_queue = queue_out - queue_in
        in_queue.append(time_in_queue)

#.................................................................
def avg_line(df_length):
   ## finds the time average number of customers in the waiting line
   ## use the next row to figure out how long the queue was   
   df_length['delta_time'] = df_length['time'].shift(-1) - df_length['time']
   ## drop the last row because it would have an infinite delta time 

   df_length = df_length.dropna()
   avg =np.average(df_length['Queue Lengths'],weights=df_length['delta_time'])
   return avg, df_length
#..................................................................

env = simpy.Environment()
## defining resources
servers_pool =simpy.Resource(env, capacity = NUMBER_SERVERS)
## selecting a random seed for the probability distributions
RANDOM_SEEDS = [1234, 5678, 9012, 3456, 7890]
np.random.seed(seed= RANDOM_SEEDS[1])

p=0

## defining the token arrival process
dist = int(input("Choose your distribution:\n1-Zipf:  \n2-Uniform:  \n3-Geometric: \n4-Real Dist:\n"))
if dist == 1:
    s = float(input("Choose the s >= 3 paremeter:"))
    batch_dist = stats.zipf(s)
elif dist == 2:
    a = float(input("Choose the lower bound a:"))
    b = float(input("Choose the upper bound b:"))
    batch_dist = stats.uniform(a, b)
elif dist == 3:
    p = 0.2#float(input("Choose the probability p:"))
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
avg_length, df_length  = avg_line(df_length)

#utilization = server_utilization(df_length)
df3 = pd.DataFrame(arrivals,   columns = ['arrivals'])
df4 = pd.DataFrame(departures, columns = ['departures'])
df_chart = pd.concat([df3, df4], axis = 1)
avg_delay_inqueue = np.mean(in_queue)
avg_delay_insyst  = np.mean(in_system)




#Calculate gamma
#I have th check if ditribution is discrete or continuous to use the correct method
n = max(batch_sizes)
try:
    list_gamma_k = 1 - batch_dist.pdf([i for i in range(1,  n + 1)])/(1 - batch_dist.cdf([i for i in range(n)]))
    
except AttributeError:
    list_gamma_k = 1-batch_dist.pmf([i for i in range(1, n + 1)])/(1 - batch_dist.cdf([i for i in range(n)]))


# I choose gamma as C_bar_1
gamma_val = x = (1 - batch_dist.cdf([i for i in range(n)]))[1] #max(list_gamma_k)

print( 'gamma: %.2f' % (gamma_val))
print('Expected value of Batch size :%.2f' % (expected_batch_size))

print('Offer load: %.2f' %(rho))
print('Wq The average delay in queue is %.2f'  % (avg_delay_inqueue))
print('W The average delay in system is %.2f' % (avg_delay_insyst))

print('Lq The average number of tokens in queue is %.2f' %  (avg_length))
print('Theoritical Lq : %.2f' %(rho/(1-rho)* (expected_batch_size + expected_value_squared) / 2 / expected_batch_size - rho))
print('Little Law Lq %.2f' %  (TOKEN_ARRIVAL_MEAN*expected_batch_size*avg_delay_inqueue))

#This boolean varibale is used to plot or not the upper bound function if it converges
converge = True
if gamma_val + rho/expected_batch_size > 1:
    print("\n-------The Upper bound won't Converge !!!")
    print("\n-------We'll not plot the Upper bound")
    converge = False



#This is the theoritical Pn of geometric Distribution
def p_n(n, lambda_val, mu_val,  rho_val, alpha=0):
    if dist == 3:
        return n.apply(lambda x: 1 - rho_val if x == 0 else (1 - rho_val) * (lambda_val / mu_val) * (1-alpha + (lambda_val / mu_val))**(x - 1))
    else:
        return n.apply(lambda x: 1 - rho_val if x == 0 else (1 - rho_val) * rho_val**x)


#Define the function for P(X > K)
def prob_func(k, lambda_val, mu_val, gamma_val, rho_val):

    r = (lambda_val / mu_val  + gamma_val)

    pk = lambda x : (lambda_val / mu_val ) * r ** (x-1) / (1 - r) * (1 - rho_val)

    return k.apply(lambda x: rho_val if x == 0 else pk(x))




Queue_dist = get_probabilities_df(df_length)

# Convert the dictionary to a DataFrame and sort by outcome
df = pd.DataFrame(list(Queue_dist.items()), columns=['Queue_length', 'PMF']).sort_values('Queue_length')

#geometric Pn
if dist = 3:
    df['P[X=n]'] = p_n(df['Queue_length'], TOKEN_ARRIVAL_MEAN, SERVER_SERVICE_MEAN,  rho, p)

df['P[X>K]'] = prob_func(df['Queue_length'], TOKEN_ARRIVAL_MEAN, SERVER_SERVICE_MEAN, gamma_val, rho)
# Calculate the CDF
df['CDF'] = df['PMF'].cumsum()

# Calculate the complement of the CDF
df['CCDF'] = 1 - df['CDF']

print("-------Start Plotting-------")


# Determine the index that corresponds to 25% of the data
index_25 = int(len(df) * 0.25)

# Create the plot using only the first 25% of the data
fig, ax = plt.subplots()
ax.bar(df['Queue_length'][:index_25], df['CCDF'][:index_25], color='blue', alpha=0.5, label='CCDF')
if converge:
    ax.bar(df['Queue_length'][:index_25], df['P[X>K]'][:index_25], color='red', alpha=0.5, label='P[X>K]')
ax.set_xlabel('Queue_length')
ax.set_title('Theoretical P[X>K] vs Empirical P[X>K] (CCDF)')
ax.set_xscale('log')  # Set log scale on x-axis
ax.legend()





# Create the first plot
fig, ax = plt.subplots()
ax.bar(df['Queue_length'][:index_25], df['PMF'][:index_25], color='blue', alpha=0.5, label='PMF')
#geometric Pn
if dist == 3:
    ax.bar(df['Queue_length'][:index_25], df['P[X=n]'][:index_25], color='red', alpha=0.5, label='P[X=n]')
ax.set_xlabel('Queue_length')
ax.set_title('Emprical PMF')
ax.set_xscale('log')  # Set log scale on x-axis  
ax.legend()



plt.show()



