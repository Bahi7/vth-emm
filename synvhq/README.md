
# VTH Implementation

This repository contains the implementation for a VTH (Volume Time Hiding) system. It is a part of a submission for ACM CCS (Association for Computing Machinery's Conference on Computer and Communications Security). The current version is for evaluation purposes only, with the full scheme to be shared later.

## Contents
- `Server.cpp` - The server-side implementation.
- `Client.cpp` - The client-side implementation.
- `Includes.h` - Common includes and definitions for the project.
- `PathORam.h` - Header for Path ORAM-related functions.
- `Lat.h` - Header for the VTH Scheme.
- `LDX.h` - Header for LDX (Leakage Free Dictionary) related functions.

## Prerequisites
- Microsoft Visual Studio with C++ support.
- The following C++ libraries:
  - Asio standalone (not Boost.Asio) for networking.
  - Cereal for serialization.
  - Boost (Lockfree queue).
  - nlohmann/json for JSON parsing.

## Setup and Execution

### Installing Dependencies
Before running the project, ensure you have the above dependencies installed. You can add them to your Visual Studio project using a package manager like vcpkg or directly include them in your project.

### Running the Project
1. **Open the Visual Studio Solution**:
   - Open the provided `.sln` file in Visual Studio.
2. **Start the Server**:
   - Build and run `Server.cpp`.
3. **Run the Client**:
   - Build and run `Client.cpp`.
   - Specify the path of a JSON file or hardcode your own multimap as needed.

### Client Operations
Once running, the client performs the following operations:
- **Data Statistics at Setup**: Prints the statistics of the data during the setup phase.
- **Key Selection and Response**: Randomly selects keys, shows their response length, and outputs the response.
- **Completion Signal**: Uses 'UPT' to signify the completion of a response for a specific keyword.
- **Latency**: The Client outputs the average processing latency 1 /Mu after each 5000 sub tokens processed.
- **End Of Program**: Stops sending queries or issuing queries after 10000 queries.

### Output
The program generates two output files:
- `QueueSize.csv`: Contains data regarding queue sizes.
- `LatencyData.csv`: Contains latency-related data.
These files are used by two separate Python scripts (not included in this repository) to retrieve the necessary statistical results.

## How the Code Works

### VTH Scheme Implementation
This code is an implementation of a Volume Time Hiding (VTH) scheme. It is designed to process a set number of queries (100 by default) before restarting the queue. This restart is essential for beginning the recording of latency and queue size, which are critical for benchmarking purposes. The VTH scheme is hardcoded to function in this manner, facilitating a consistent and controlled evaluation environment.

### Running the VTH Scheme
To run the VTH scheme:
- **Start the Server**: Run `Server.cpp`, which initiates the server-side of the VTH scheme.
- **Run the Client**: Execute `Client.cpp`. The client-side is pre-configured to process 100 queries before it begins recording performance metrics and stops after 10000 queries.

### Evaluating Baseline 1
- **Modify `measureLatencyForKey` Function**:
  - In `Client.cpp`, change the `GetC_P_BaseLine1` method by uncommenting it and commenting out the `GetC_P` call.
  - You also need to specify the maximum response length for Baseline 1.
- **Adjust `lat.h`**:
  - Uncomment the continue in `SQPD_P_S()` while loop. This loop is responsible for sending the sub tokens and dummy queries if the queue is empty sending dummy queries.
- **Change `sendToCid` Method**:
  - Modify alpha to alphaX to account for the dummy queries in Baseline 1.

### Evaluating Baseline 2
- **Update `Lat` Constructor in `Lat.h`**:
  - Comment out the original constructor `LAT` and uncomment the one specific to Baseline 2.
- **Modify `measureLatencyForKey` Function in `Client.cpp`**:
  - Use the `GetC_P_BaseLine2` method instead of the standard `GetC_P`.
- **Adjust in `lat.h`**:
  - As with Baseline 1, uncomment the `continue` statement in the `SQPD` function. Baseline 2 does not require changes to the `sendToCid` method, as it doesn't use dummy queries.

## Note for Evaluators
This code is provided for evaluation purposes in the context of an ACM CCS submission. It represents a part of the larger scheme, which will be shared in its entirety at a later stage. As such, this implementation is not the final version and may undergo significant changes before the complete scheme is released.
