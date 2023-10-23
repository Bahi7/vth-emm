#include <iostream>
#include <unordered_map>
#include <vector>
#include <cereal/archives/binary.hpp>
#include <cereal/types/map.hpp>
#include <cereal/types/vector.hpp>
#include <cereal/types/string.hpp>
#include <iostream>
#include <winsock2.h>
#include <cmath>
#include <map>
#include <algorithm>
#include <cstdlib>
#include <ctime>
#include <functional>
#include <optional>

using namespace std;
 // This line will allow you to use everything from the standard library without prefixing it with 
struct DataPacket {
    map<int, int> stRAM;
    map<int, string> stash_data;
    vector<int> stash;

    DataPacket() = default;

    // Constructor to initialize the data
    DataPacket( const map<int, int>& stRAM_,
               const map<int, string>& stash_data_,
                const vector<int>& stash_)
        : stRAM(stRAM_), stash_data(stash_data_), stash(stash_) {}

    template<class Archive>
    void serialize(Archive & archive) {
        archive(stRAM, stash_data, stash);
    }

    void print() const {
        std::cout << "---- DataPacket ----" << std::endl;
        std::cout << "stRAM:" << std::endl;
        for(const auto& [key, value] : stRAM) {
            std::cout << key << " -> " << value << std::endl;
        }

        std::cout << "stash_data:" << std::endl;
        for(const auto& [key, value] : stash_data) {
            std::cout << key << " -> " << value << std::endl;
        }

        std::cout << "stash:" << std::endl;
        for(const auto& value : stash) {
            std::cout << value << " ";
        }
        std::cout << std::endl;
        std::cout << "--------------------" << std::endl;
    }
};

class LDX {
private:
    unordered_map<string, int> DXst;
    vector<string> RAM;
    map<int, int> stRAM;
    string K;
    WSADATA wsaData;
    string server_Addr;
    int port;
    SOCKET clientSocket;

    map<int, string> stash_data;
    vector<int> stash;
    ostringstream os;
    char buffer[1024];
    int bytesReceived; 

public:
    LDX(const string& serverAddr, int p) : server_Addr(serverAddr), port(p), clientSocket(INVALID_SOCKET) {
        if (!initialize()) {
            cerr << "Initialization failed in constructor." << endl;
        }

        if (!connectToServer()) {
            cerr << "Connection failed in constructor." << endl;
        }
    }

    ~LDX() {
        closeConnection();
        WSACleanup();
    }

    bool initialize() {
        if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
            cerr << "Failed to initialize Winsock." << endl;
            return false;
        }

        clientSocket = socket(AF_INET, SOCK_STREAM, 0);
        if (clientSocket == INVALID_SOCKET) {
            cerr << "Failed to create socket." << endl;
            return false;
        }
        return true;
    }

    bool connectToServer() {
        
        sockaddr_in serverAddr;
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_port = htons(port);
        serverAddr.sin_addr.s_addr = inet_addr(server_Addr.c_str());

        if (connect(clientSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
            cerr << "Failed to connect." << std::endl;
            return false;
        }
        return true;
    }

    void closeConnection() {
        if (clientSocket != INVALID_SOCKET) {
            closesocket(clientSocket);
            clientSocket = INVALID_SOCKET;
        }
    }

    bool setupCS() {
        // Serialize the vector
        {
            cereal::BinaryOutputArchive archive(os);
            archive(RAM);
        }
        string serializedVec = os.str();
        cerr << "Sending RAM." << std::endl;
        cout << "Size of RAM serialized data: " << serializedVec.size() << " bytes" << std::endl;
        send(clientSocket, serializedVec.c_str(), serializedVec.size(), 0);

        cerr << "Receiving PosMap..." << endl;
        bytesReceived = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);

        istringstream is(string(buffer, bytesReceived));
        {
            cereal::BinaryInputArchive archive(is);
            archive(stRAM);
        }
        
        cerr << "Checking bytesReceived PosMap." << endl;
        // Print received map (just for testing)
        for (const auto& [key, value] : stRAM) {
            cout << key << " : " << value << endl;
        }
       
        return true;
    }

    void Setup(const int k, const unordered_map<string, string> DX) {
        int addr = 0;

        for (const auto& pair : DX) {
            RAM.push_back(pair.second);
            DXst[pair.first] = addr;
            addr++;
        }

        if (!setupCS()) cout << "Setup Error" << endl;
    }

    void AccessCS(const string label, const int addr, string& doc_id) {

        cerr << "Creating DataPacket." << endl;
        DataPacket query(stRAM, stash_data, stash);

        
        os.str("");        // Clear the contents
        os.clear();  
        {
            cereal::BinaryOutputArchive archive(os);
            archive(query);
        }
        string serializedquery = os.str();
        cerr << "sending DataPacket." << std::endl;
        cout << "Size of query serialized data: " << serializedquery.size() << " bytes" << endl;
        send(clientSocket, serializedquery.c_str(), serializedquery.size(), 0);

        os.str("");        // Clear the contents
        os.clear();  
        {
            cereal::BinaryOutputArchive archive(os);
            archive(addr);
        }
        string serializedData = os.str();
        cerr << "sending Query." << std::endl;
        std::cout << "Size of query serialized data: " << serializedData.size() << " bytes" << std::endl;
        send(clientSocket, serializedData.c_str(), serializedData.size(), 0);

        memset(buffer, 0, sizeof(buffer));
        bytesReceived = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
        istringstream is(string(buffer, bytesReceived));
        map<int, string> response;
        {
            cereal::BinaryInputArchive archive(is);
            archive(response);
        } 
        cerr << "receiving responses :  " << response.size() <<endl; 
        doc_id = getValueContainingSubstring(response, label);
    }

    void Get(const string label, string& doc_id) {
        int noOfLeafs = pow(2, static_cast<int>(ceil(log2(RAM.size())) - 1));
        int addr = DXst[label];
        int x = stRAM[addr + 1];
        stRAM[addr+1] = rand() % (noOfLeafs - 1);
        AccessCS(label, x, doc_id);
    }

  std::string getValueContainingSubstring(const map<int, std::string>& m, const std::string& substring) {
    for (const auto& [key, value] : m) {
        size_t pos = value.find(substring);
        if (pos != std::string::npos) {
            std::string modifiedValue = value;
            modifiedValue.erase(pos, substring.length());
            return modifiedValue;
        }
    }
    return ""; // Return an empty string if no matching value is found
}

};


