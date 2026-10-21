#pragma once
#include "PathOram.h"

using namespace std;
using asio::ip::tcp;
// This line will allow you to use everything from the standard library without prefixing it with 

class LDX
{
private:

    unordered_map<string, int> DXst;
    vector<string> RAM;
    unordered_map<int, size_t> stRAM;
    unordered_map<string, string> client_stash;
    //Security parameter
    unsigned char* K;
    size_t max_length = 0;
    PathOram oramInstance;
    std:: unique_ptr<char*[]> response_stash;
    std::unique_ptr<unsigned char* []> encrypted_response_stash;
    mt19937 generator; // Mersenne Twister generator
    uniform_int_distribution<uint64_t> distribution; // Uniform distribution
    asio::io_context io_context;
    tcp::socket socket;
    unsigned char* dataBlock;
    size_t response_stash_size, blockSize, n;
    ThreadPool* pool;
  


public:

    LDX(const std::string& serverAddr, const uint16_t& port)
        : io_context(), socket(io_context), generator(std::random_device{}()),
        distribution(0, std::numeric_limits<uint64_t>::max()) {

        // Resolve the server address and port
        tcp::resolver resolver(io_context);
        auto endpoints = resolver.resolve(serverAddr, std::to_string(port));

        // Try to connect to one of the resolved endpoints
        try {
            asio::connect(socket, endpoints);
            std::cout << "Connected to " << serverAddr << ":" << port << std::endl;
        }
        catch (std::exception& e) {
            std::cerr << "Failed to connect: " << e.what() << std::endl;
        }
    }

    void getClientStash_size() {
        cout << client_stash.size() <<"\n";
    }
    void serializeData(const std::unique_ptr<unsigned char* []>& data, const size_t& dataSize, const size_t& blockSize) {
        std::ofstream os("serialized_data.bin", std::ios::binary);
        cereal::BinaryOutputArchive archive(os);

        // Serialize each string in the array
        for (size_t i = 0; i < dataSize; ++i) {
           
            // Now serialize the string data
            for (size_t j = 0; j < blockSize; ++j) {
                archive(data[i][j]);
            }
        }
    }

    bool setupCS() {

        // Server has responded to a ping request				
        std::cout << "Server Accepted Connection\n";
      
        vector<size_t> attributes;
        oramInstance = PathOram(RAM, stRAM, max_length, client_stash, K);
        RAM.clear(); 
        std::vector<string>().swap(RAM);

        attributes.push_back(oramInstance.getL());
        attributes.push_back(oramInstance.getNoOfLeafs());
        attributes.push_back(oramInstance.getsizeOfBlock());
        attributes.push_back(oramInstance.getTreeSize());
        attributes.push_back(oramInstance.getZ());

        n =  oramInstance.response_size(); 
        blockSize = oramInstance.getsizeOfBlock();
        response_stash_size = n * blockSize;

        // Allocate the contiguous block
        dataBlock = new unsigned char[response_stash_size];

        // Create an array of pointers for easy access
        response_stash = std::make_unique<char* []>(n);
        encrypted_response_stash = std::make_unique<unsigned char* []>(n);

        // Point each element in 'data' to the correct location in 'dataBlock'
        for (size_t i = 0; i < n; ++i) {
            encrypted_response_stash[i] = &dataBlock[i * blockSize];
            response_stash[i] = new char[blockSize + 1 - EVP_MAX_IV_LENGTH];
            response_stash[i][blockSize - EVP_MAX_IV_LENGTH] = '\0';
        }

        cout << "Request_Size: " << response_stash_size / 1024 << " KB\n";

        cout << "Block size: " << oramInstance.getsizeOfBlock() << " B\n";
       
        serializeData(oramInstance.getTree(), oramInstance.getTreeSize() * oramInstance.getZ(), oramInstance.getsizeOfBlock());
   
        oramInstance.deleteTree();
        std::cout << "Press Enter to continue...";
        std::cin.get();  // Waits for the user to press Enter

        // Continue with the rest of your code
        std::cout << "Continuing with the rest of the program.\n";

        //Send serialized attributes
        asio::write(socket, asio::buffer(attributes));

        // Read the response back from the server
        char ack[4];  // Assuming 'ok' is the expected response
        asio::read(socket, asio::buffer(ack, 3));

        ack[3] = '\0';

        size_t numOfThreads = std::thread::hardware_concurrency() / 2;
        cout << "Half Number Of Threads: " << numOfThreads  << "\n";
        pool = new ThreadPool(numOfThreads);
        if (string(ack) == "ok") return true;
        return false;

    }

    void Setup(unsigned char* k, const map< string, string>& DX) {
        int addr = 0;
        K = k;
        
        for (const auto& pair : DX) {
            if (pair.second.size()  + to_string(addr).size() > max_length) max_length = to_string(addr).size() + pair.second.size();
         
            RAM.push_back(to_string(addr) + pair.second);
            DXst[pair.first] = addr;
            ++addr;
        }

        if (!setupCS()) cout << "Setup Error" << endl;
    }

    void AccessCS(const std::string& label, const size_t leafNode, const int& addr, std::string& doc_id) {
       
        stRAM[addr + 1] = distribution(generator) % oramInstance.getNoOfLeafs();
       
        // Send leafNode
        asio::write(socket, asio::buffer(&leafNode, sizeof(size_t)));
        

        asio::read(socket, asio::buffer(dataBlock, response_stash_size));
      
      
        decryptAll(*pool, encrypted_response_stash, n, blockSize, K,  response_stash);
   
        oramInstance.check(response_stash, client_stash, addr, doc_id);
        oramInstance.reAll(response_stash, stRAM, DXst, leafNode, client_stash);
     
        encryptAll(*pool,response_stash, n, blockSize, K, encrypted_response_stash);
      
        asio::write(socket, asio::buffer(dataBlock, response_stash_size));
        
    }

    void Get(const string& label, string& doc_id)  {
        AccessCS(label , stRAM[DXst[label] + 1], DXst[label] , doc_id);
    }




};