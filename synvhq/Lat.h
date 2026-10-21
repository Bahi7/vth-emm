#pragma once
#include "LDX.h"
#include <filesystem>

// Include for std::mutex



using namespace std; // This line will allow you to use everything from the standard library without prefixing it with std::



class LAT {
private:

    mutex dataMutex;
  
    map<string, string> DX;
    unordered_map<string, size_t> DXst;

    //This queue incorporate Also the arrival time needed to calculate latency in the Evaluations
    queue<tuple<int, string, bool, std::chrono::time_point<std::chrono::high_resolution_clock>>> Q;
    
    bool response_flag;
    LDX ldx;
    const size_t keyLength = 16; // For a 128-bit key
    unsigned char key[16];

    int queryCounter;
    // Stub function to simulate the ?DX.Setup
    void SigmaDX_Setup(const map<string, string>& DX) {
     
        ldx.LDX::Setup(key,  DX);
    }

public:



    LAT(const string k, const unordered_map<string, vector<string>>* MM) : ldx("127.0.0.1", 3333) {
        
        
        unsigned char salt[8];

       
        RAND_bytes(salt, sizeof(salt));

        deriveKeyFromPassword(k, salt, key, keyLength);

        for (const auto& pair : *MM) {
            const string& label = pair.first;
            const vector<string>& values = pair.second;

            for (int i = 0; i < values.size(); i++) {

                DX[label + to_string(i)] = "x-x" + values[i];
            }
            DXst[label] = values.size();    
        }
    
        DX["alpha"] = "x-xDummy";
        DXst["alpha"] = 1;

      
        SigmaDX_Setup(DX);

        queryCounter = 1000;
    }
   
    //baseline2 version
    //LAT(const string k, const unordered_map<string, vector<string>>* MM) : ldx("127.0.0.1", 3333) {
    //    

    //    unsigned char salt[8];
    //   
    //    RAND_bytes(salt, sizeof(salt));

    //    deriveKeyFromPassword(k, salt, key, keyLength);

    //    for (const auto& pair : *MM) {
    //        const string& label = pair.first;
    //        const vector<string>& values = pair.second;
    //        string pack;

    //        for (int i = 0; i < values.size(); i++) {

    //            pack += values[i];
    //        }
    //        DX[label] = "x-x" + pack;
    //        DXst[label] = values.size();
    //    }
   
    // 
    //    SigmaDX_Setup(DX);
    //    queryCounter = 0;
    //}

    void GetC_P(const int& id, const string& label) {

        int response_length = DXst[label];
        ++queryCounter;
      
        for (int i = 0; i <= response_length - 1; i++) {
      
            {
                std::lock_guard<std::mutex> lock(dataMutex);
                //remember to change this id of client by response_length for the Evaluation purposes
                Q.emplace(response_length, label + to_string(i), i == (response_length - 1), std::chrono::high_resolution_clock::now());
               
            }
          
        }

    }
    //This Get is for Baseline2 Variation Evaluation
    void GetC_P_BaseLine2(const int& id, const string& label) {

        int response_length = DXst[label];
        ++queryCounter;
        std::lock_guard<std::mutex> lock(dataMutex);
        //remember to change this id of client by response_length for the Evaluation purposes
        Q.emplace(response_length, label, true, std::chrono::high_resolution_clock::now());

    }

    //This Get is for Baseline2 Variation Evaluation
    void GetC_P_BaseLine1(const int& id, const string& label, const int maxResponseLength) {

        int response_length = DXst[label];
        ++queryCounter;

        for (int i = 0; i <= maxResponseLength - 1; ++i) {

            {
                std::lock_guard<std::mutex> lock(dataMutex);
                //remember to change this id of client by response_length for the Evaluation purposes
                if(i <= response_length - 1) Q.emplace(response_length, label + to_string(i), i == (maxResponseLength - 1), std::chrono::high_resolution_clock::now());
                else Q.emplace(response_length, "alpha", i == (maxResponseLength - 1), std::chrono::high_resolution_clock::now());


            }
        }

    }

public:

    void SQPD_P_S() {
        //Evaluatio files
        
        ofstream QSize_file("QueueSize.csv", ios_base::app);
        QSize_file << "QueueSize,DeltaTime" << endl;
        ofstream latency_file("latency_data.csv", ios_base::app);
        latency_file << "ResponseLength,Latency" << endl;
        size_t counter = 0;
        double latency = 0;

        double waitTime;
        int id;
        string label_star;
        bool flag;
        vector<string> result;
        pair<string, nullptr_t> result_pair;
        
        //Warmup phase
        if (queryCounter == 1e2) {
            while (!Q.empty()) {
                std::lock_guard<std::mutex> lock(dataMutex);
                Q.pop();
            }
        }

        std::chrono::time_point<std::chrono::high_resolution_clock> start, end, arrival_time, startQueue;
        tuple<int, string, bool, std::chrono::time_point<std::chrono::high_resolution_clock>> e;

        //  you can either specify a number of queries or run it indifinetly  while(true)
        while (queryCounter <= (1e4 + 1e2))
        {
            if (!Q.empty()) {
              
                {
                    std::lock_guard<std::mutex> lock(dataMutex);
                    e = Q.front();
                    Q.pop();
                }

                id = get<0>(e);
                label_star = get<1>(e);
                flag = get<2>(e);

                //the arrrival time is only for evaluations not part of the real scheme
                arrival_time = get<3>(e);
              
               
            }
            
            else {

                label_star = "alpha";
                id = 1;
                flag = true;

                //Continue for baselines because they don't send dummy queries
                //continue;
                
            
            }
         
            // Write the QueueSize and latency to the CSV

            {
                std::lock_guard<std::mutex> lock(dataMutex);
                if (queryCounter > 1e2) {
                    QSize_file << Q.size() << "," << std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - startQueue).count() << "\n";
                }
            }
             startQueue = std::chrono::high_resolution_clock::now();

            start = std::chrono::high_resolution_clock::now();
            GetP_S(label_star, result_pair);
            end = std::chrono::high_resolution_clock::now();
            latency += std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
            ++counter;

            //Warmup phase Done
            if (queryCounter > 1e2) {

                //change nullptr by the latency_file to record latency
                sendToCid(id, label_star, result_pair.first, result, &latency_file, flag, arrival_time);

                if (counter % 5000 == 0) {
                    QSize_file.flush();
                    latency_file.flush();
                    cout << "query:# " << queryCounter << "--" << latency / counter << "us\nstash size: ";
                    ldx.getClientStash_size();
                }
                
            }
            
        }

        QSize_file.close();
        latency_file.close();
        cout << "we done\n";

    }

    
    void sendToCid(const int& id, const string& label_star, const string& value,  vector<string>& result, std::ofstream* file, const bool& flag, std::chrono::time_point < std::chrono::high_resolution_clock> arrial_time) {
        
        if (value == "") {
            cout <<"subtoken: "<< label_star << "\nSending to: " << id << "\nresponse: " << "!Empty! " << "\n";
            this_thread::sleep_for(std::chrono::milliseconds(3600 * 1000));
        }
       cout << "subtoken: " << label_star <<"\nSending to: " << id << "\nresponse: " << value << "\n";
        if (flag) {

            //Remove this condition when using the BaseLine Variations for calculating latency
            if (label_star != "alpha") {
                std::chrono::time_point<std::chrono::high_resolution_clock> end;
                end = std::chrono::high_resolution_clock::now();
                *file << id << "," << std::chrono::duration_cast<std::chrono::microseconds>(end - arrial_time).count() << "\n";
            }
            cout << "Sending to " << id << ": " << "UPT\n"; 
        }
    }

    void  GetP_S(const string& label_star, pair<string, nullptr_t>& result_pair) {      
        ldx.LDX::Get(label_star, result_pair.first);
    }
    void set_responseFlag() {
        response_flag = true;
    }

    std::vector<string> getKeys() {
        std::vector<string> keys;
        keys.reserve(DXst.size());
        for (const auto& pair : DXst) {
            if (pair.first != "alpha") keys.push_back(pair.first);
        }
        return keys;
    }
    
};


