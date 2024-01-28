#pragma once
#include "Lat.h"
#include <nlohmann/json.hpp>

namespace fs = std::filesystem;
using json = nlohmann::json;
double lambda = 32;

std::string input_directory = "MM\\MM";

double calculateExpectedValueSquared(const std::unordered_map<std::string, std::vector<std::string>>& myMap) {
    double sumOfSquares = 0.0;
    int numberOfKeys = 0;

    for (const auto& pair : myMap) {
        int length = pair.second.size();  // Length of the vector for this key
        sumOfSquares += length * length;  // Add the square of the length
        numberOfKeys++;
    }

    if (numberOfKeys == 0) return 0;  // Avoid division by zero

    return sumOfSquares / numberOfKeys;  // Calculate the average of the squares
}

double exponentialWait(double lambda) {
    std::random_device rd;
    std::mt19937_64 gen(rd());
    std::exponential_distribution<double> d(lambda);

    return d(gen);
}


namespace fs = std::filesystem;
using json = nlohmann::json;

// Function to reverse the map
std::unordered_map<std::string, std::vector<std::string>> reverseMap(
    const json& originalMap) {

    std::unordered_map<std::string, std::vector<std::string>> reversedMap;
    for (auto& [key, value] : originalMap.items()) {
        for (const auto& str : value) {
            reversedMap[str].push_back(key);
        }
    }
    return reversedMap;
}

// Function to reverse the map back
std::unordered_map<std::string, std::vector<std::string>> reverseMapBack(
    const std::unordered_map<std::string, std::vector<std::string>>& reversedMap) {

    std::unordered_map<std::string, std::vector<std::string>> originalMap;
    for (const auto& pair : reversedMap) {
        for (const auto& str : pair.second) {
            originalMap[str].push_back(pair.first);
        }
    }
    return originalMap;
}
static std::unordered_map<std::string, std::vector<std::string>> readAndMergeJSONFiles(const std::string& directory, size_t size_of_data) {
    std::cout << "Reading MM..." << std::endl;

    std::unordered_map<std::string, std::vector<std::string>> dataMap;
    size_t maxValues = 0;  // To track the maximum number of values for a single key
    size_t totalPairs = 0; // To track the total number of key-value pairs

    for (const auto& entry : fs::directory_iterator(directory)) {
        if (totalPairs >= size_of_data) {
            break;  // Stop if we have reached the size limit
        }

        if (entry.path().extension() == ".json") {
            std::ifstream file(entry.path());
            if (!file.is_open()) {
                std::cerr << "Failed to open " << entry.path() << std::endl;
                continue;
            }

            json j;
            file >> j;
            file.close();
            auto revMap = reverseMap(j);
           
            // Update the unordered_map
            for (const auto& pair : revMap) {
                
                for (const auto& item : pair.second) {
                   
                    dataMap[pair.first].push_back(item);
                    totalPairs++;
                    maxValues = std::max(maxValues, dataMap[pair.first].size());
                    if (totalPairs >= size_of_data) {
               
                        return reverseMapBack(dataMap);  // Return the map if size limit is reached
                    }
                }
            }
        }
    }

    return reverseMapBack(dataMap);
}

void printStatistics(const std::unordered_map<std::string, std::vector<std::string>>* dataMap) {
    if (dataMap->empty()) {
        std::cout << "The map is empty." << std::endl;
        return;
    }

    size_t maxValues = 0;
    size_t totalPairs = 0;
    double sumOfSquares = 0.0;

    for (const auto& pair : *dataMap) {
        maxValues = std::max(maxValues, pair.second.size());
        totalPairs += pair.second.size();
        sumOfSquares += std::pow(pair.second.size(), 2);
    }

    double expectedValueSquared = sumOfSquares / dataMap->size();

    std::cout << "Maximum values for a key: " << maxValues << std::endl;
    std::cout << "Number of pairs: " << totalPairs << std::endl;
    std::cout << "Number of keys: " << dataMap->size() << std::endl;
    std::cout << "E[X^2] = " << expectedValueSquared << std::endl;
}
std::string getRandomKey(const std::vector<std::string>& vec) {
    if (vec.empty()) {
        throw std::runtime_error("Vector is empty");
    }

    // Random number generator
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, vec.size() - 1);

    // Select a random index
    int index = dis(gen);

    return vec[index];
}
void startProxy(LAT& lat) {
    std::cout << "Proxy starting !" << std::endl;
    lat.LAT::SQPD_P_S();

}
void measureLatencyForKey(LAT& lat) {
    double waitTime;
    std::string randomKey;
    std::chrono::time_point<std::chrono::high_resolution_clock>start, end;
    vector<string> keysVec = lat.getKeys();
    size_t thread_id_hash = std::hash<std::thread::id>()(std::this_thread::get_id());
    int stop = 10000;
    while (stop--) {

        waitTime = exponentialWait(lambda);
        preciseSleep(waitTime); // Assuming waitTime is in seconds
        randomKey = getRandomKey(keysVec);

     
        std::cout << "Client chosen keyword -----> " << randomKey << std::endl;
        
 
        lat.GetC_P(thread_id_hash, randomKey);
        
        //Baseline First appraoch
        //lat.GetC_P_BaseLine1(thread_id_hash, randomKey, 25);
        
        //Baseline second approach
        //lat.GetC_P_BaseLine2(thread_id_hash, randomKey);
         
    }

}
bool spawnThreadsAndMeasure(const int numThreads, LAT& lat) {
    std::vector<std::thread> threads;

    threads.push_back(std::thread(startProxy, std::ref(lat)));

    for (int i = 0; i < numThreads; ++i) {

        threads.push_back(std::thread(measureLatencyForKey, std::ref(lat)));
    }


    for (auto& thread : threads) {
        thread.join();
    }
    return true;
}

// Function to fill a multimap with keys where the key's length follows a geometric distribution
std::unordered_map<std::string, std::vector<std::string>> createMapWithGeometricDistribution(int numKeys, double successProbability) {
    std::unordered_map<std::string, std::vector<std::string>> myMap;
    std::random_device rd;  // Non-deterministic random number generator
    std::mt19937 gen(rd()); // Seed the generator
    std::geometric_distribution<> d(successProbability); // Geometric distribution

    size_t nbrofpairs = 0;
    size_t maxlength = 0;

    for (int i = 0; i < numKeys; ++i) {
        // Generate a key name
        std::string key = "Key_" + std::to_string(i);

        // Generate a random size for the vector following the geometric distribution
        int vecSize = d(gen);

        // Create a vector of the determined size with dummy content
        std::vector<std::string> vec(vecSize, "dummy_value_1234567890123456789012345678901234567890123456789065324667866tutgf uyr65 765 541");
        if (maxlength < vecSize) {
            maxlength = vecSize;
        }
        nbrofpairs += vecSize;
        // Insert the key-value pair into the map
        myMap[key] = vec;
    }
    cout << "maxRep " << maxlength << "\n";
    cout << "number of pairs  " << nbrofpairs << "\n";

    return myMap;
 }

int main() {
    
    auto MM = std::make_unique<std::unordered_map<std::string, std::vector<std::string>>>(

        readAndMergeJSONFiles(input_directory, pow(2, 12))
    
    );

    std::cout << "Reading MM done" << std::endl;
    printStatistics(MM.get());
    LAT lat("your_password_here", MM.get());
    
    MM.reset();
   
    spawnThreadsAndMeasure(1, lat);


    return 0;
}
