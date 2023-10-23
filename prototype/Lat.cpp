#include <iostream>
#include <unordered_map>
#include <queue>
#include <vector>
#include <pthread.h>
#include <time.h>
#include "LDX.cpp"
#include <cereal/archives/binary.hpp>
#include <cereal/types/map.hpp>
#include <cereal/types/vector.hpp>
#include <cereal/types/string.hpp>


using namespace std; // This line will allow you to use everything from the standard library without prefixing it with std::

class LAT {
private:
    unordered_map<string, string> DX;
    unordered_map<string, int> DXst;
    queue<tuple<int, string, bool>> Q;
    LDX ldx;

    // Stub function to simulate the ΣDX.Setup
    void SigmaDX_Setup(int k, const unordered_map<string, string>& DX) {
      
        ldx.Setup(k, DX);
    }

public:
    
    void sleep_milliseconds(int milliseconds) {
    struct timespec ts;
    ts.tv_sec = milliseconds / 1000;
    ts.tv_nsec = (milliseconds % 1000) * 1000000;
    nanosleep(&ts, NULL);
}

    LAT(int k, const unordered_map<string, vector<string>>& MM) : ldx("127.0.0.1", 12345) {
        // Initialize DX and DXst
        for (const auto& pair : MM) {
            const string& label = pair.first;
            const vector<string>& values = pair.second;

            for (int i = 0; i < values.size(); i++) {
                DX[label + to_string(i)] = label + to_string(i)+values[i];

            }

            DXst[label] = values.size();
        }

        // Set DX[α] to 0
        DX["α"] = "Dummy"; // Assuming θ is defined somewhere in your code

        // Run ΣDX.Setup
        SigmaDX_Setup(k, DX);
    }

    vector<string> GetC_P(const string& label) {
        vector<string> result;
        int id = 0;
        int n = DXst[label];
        for (int i = 0; i < n - 1; i++) {
            // The pseudocode says Q.enqueue, but doesn't specify what to enqueue.
            // As a placeholder, enqueueing label + index, but this might be wrong without more details.
            Q.emplace(id, label + to_string(i), false);
        }
        Q.emplace(id, label + to_string(n-1), true);
        while(SQPD_P_S());
    }
    public:
    // Assuming D is some class or function that allows us to sample
    int sampleFromD() {
        // Placeholder, replace with your sampling code
        return rand() % 100; // Random int between 0 and 99
    }

    bool SQPD_P_S() {
        // 1. P samples t $← D and wait t time steps
        int t = sampleFromD();
        int id;
        string label_star;
        bool flag ;
        
       sleep_milliseconds(t);

        // 2. parse EMM as EDX and st as (DXst,stDX, Q)
        //string EDX = EMM; // Placeholder, replace with parsing logic if needed

        // 3. if Q.peek ̸= ⊥
        if (!Q.empty()) {
            // (a) P computes e ← Q.dequeue
            tuple<int, string, bool> e = Q.front();
            Q.pop();

            // (b) P parses e as (id, ℓ⋆, flag)
        
            id = get<0>(e);
            label_star = get<1>(e);
            flag = get<2>(e);
            
            // 4. Otherwise P sets ℓ⋆ := α
        } else {
            label_star = "α";
        }

        // 5. compute (r, ⊥) ← ΣDX.GetP_S((st, ℓ⋆), EDX);
        pair<string, nullptr_t> result_pair = GetP_S(label_star);
        string r = result_pair.first;

        // 6. P sends to Cid the value r
        sendToCid(id, r);

        // 7. if flag = true, P sends ⊥ to Cid
        if (flag) {
            sendToCid(id, "⊥"); // Using the "⊥" symbol as a placeholder
            return false;
        }
        return true;
    }

    // Placeholder function, you need to replace this with your message sending mechanism
    void sendToCid(const int& id, const string& message) {
        // Implement your communication logic here
        cout << "Sending to " << id << ": " << message << endl;
    }

    pair<string, nullptr_t> GetP_S(const string& label_star) {
        string retrievedValue;
        ldx.Get(label_star, retrievedValue);
        return {retrievedValue, nullptr};
    }

    // ... other methods ...
};


