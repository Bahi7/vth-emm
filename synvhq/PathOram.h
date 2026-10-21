#pragma once 
#include "includes.h"
#include <intrin.h>

using namespace std;
class PathOram {

private:

    size_t L, treeSize, noOfLeafs, pathLength, sizeOfBlock, Z = 4;
    std::unique_ptr<unsigned char* []> data;
    size_t* newPath, * oldPath;
    bool* isAltered;
    char* dummy;

    unsigned int clz(unsigned int value) {
        unsigned long leading_zero = 0;

        if (_BitScanReverse(&leading_zero, value)) {
            return 31 - leading_zero;
        }
        else {
            // Handle the case where value is 0
            return 32;
        }
    }

    int readLeaf(const size_t& branch) const {
        return noOfLeafs + branch - 1;
    }

    int getParent(const size_t& node) const {
        return (node - 1) >> 1;
    }

    void pathNodes(size_t branch, size_t*& path) const {


        for (size_t i = 0; i <= L; i++) {

            path[i] = branch;
       
            branch = getParent(branch);

        }
    }
    //int readLeaf(const int& branch) const {
    //    return 2 * branch;
    //}

    //int getParent(const int& node, int& h, int& i) const {
    //    int sign = (h % 2 == 0) ? 1 : -1;
    //    h >>= 1;  // Halve h.

    //    // Correcting the operator precedence and ensuring proper calculation of 2^i.
    //    return node + sign * (1 << i++);
    //}

    //void pathNodes(int branch, int*& path) const {
    //    int h = branch >> 1;
    //    int x = 0;  // Starting from 0 to properly calculate 2^i.
    //    for (int i = 0; i <= L; ++i) {
    //        path[i] = branch;
    //        branch = getParent(branch, h, x);
    //    }
    //}

    void readBucket(const size_t& block, unsigned char*& output) const noexcept {

        if (data[block] != nullptr && output != nullptr) {
            // Copy the data from the 'data' array to the 'output' buffer
            std::memcpy(output, data[block], sizeOfBlock);
        }
        
    }

    void writeBucket(const size_t& block, const unsigned char* newData) noexcept {

        if (data[block] != nullptr && newData != nullptr) {
            // Copy the data from 'newData' to the 'data' array
            std::memcpy(data[block], newData, sizeOfBlock);

         
        }
    }

    static void generateRandomString(size_t maxLength, char*& result) {
        static const char alphanum[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";

        // Ensure that maxLength is at least 2 for the start and end '*'
        if (maxLength < 2) {
            maxLength = 2;
        }

        // Allocate memory for the result string
        result = new char[maxLength + 1]; // +1 for the null terminator

        result[0] = '*'; // Start with *

        for (size_t i = 1; i < maxLength - 1; ++i) {
            result[i] = alphanum[rand() % (sizeof(alphanum) - 1)]; // Append a random character
        }

        result[maxLength - 1] = '*'; // End with *
        result[maxLength] = '\0';
        // Null-terminate the string
    }

    void splitBySeparator(const char* input, const char* delimiter, char* output[]) const noexcept {
        // Find the first occurrence of the delimiter
        const char* delimiterPos = strstr(input, delimiter);

        if (delimiterPos != nullptr) {
            size_t pos = delimiterPos - input;
            size_t delimiterLen = strlen(delimiter);

            // Copy the first part
            size_t firstPartLength = std::min(pos, sizeOfBlock - EVP_MAX_IV_LENGTH);
            strncpy(output[0], input, firstPartLength);
            output[0][firstPartLength] = '\0'; // Null-terminate

            // Copy the second part
            const char* secondPartStart = input + pos + delimiterLen;
            size_t secondPartLength = std::min(strlen(secondPartStart), sizeOfBlock - EVP_MAX_IV_LENGTH);
            strncpy(output[1], secondPartStart, secondPartLength);
            output[1][secondPartLength] = '\0'; // Null-terminate
        }
        else {
            // Delimiter not found, copy the whole string to output[0]
            strncpy(output[0], input, sizeOfBlock - EVP_MAX_IV_LENGTH);
            output[0][sizeOfBlock - EVP_MAX_IV_LENGTH] = '\0'; // Null-terminate
            output[1][0] = '\0'; // Make output[1] empty
        }
    }



public:

  
    void init_arrays()
    {
        isAltered = new bool[Z * pathLength];
        oldPath = new size_t[pathLength];
        newPath = new size_t[pathLength];
        generateRandomString(sizeOfBlock - EVP_MAX_IV_LENGTH, dummy);
    }
    PathOram() : L(0), treeSize(0), noOfLeafs(0), sizeOfBlock(0), Z(4), data(), newPath(), oldPath(), isAltered(), dummy() {}

    PathOram(const vector<string>& RAM, unordered_map<int, size_t>& posMap, const size_t& maxLength, unordered_map<string, string>& stash, const unsigned char* key) {

        std::cout << "Starting setup\n";

        size_t N = RAM.size();
      
        L = (((N > 1) ? 31 - clz(N - 1) : 0) + 1);
        noOfLeafs = pow(2, L);
        treeSize = (1 << (L + 1)) - 1;
        pathLength = L + 1;
        sizeOfBlock = maxLength + EVP_MAX_IV_LENGTH;
        init_arrays();



        std::cout << "Number of leafs: " << noOfLeafs << endl;

        std::cout << "Height: " << L << endl;

        std::cout << "size of Tree: " << treeSize << endl;

        data = std::make_unique<unsigned char* []>(treeSize * Z);
       

        

        char* result[2] = { new char[sizeOfBlock - EVP_MAX_IV_LENGTH + 1], new char[sizeOfBlock - EVP_MAX_IV_LENGTH + 1] }; // Assuming sizeOfBlock is defined
        int numCharsToAdd;
        bool insertion;
        size_t leaf;
        random_device rd;
        mt19937 generator(rd());
        char* baseChar = new char[sizeOfBlock + 1 - EVP_MAX_IV_LENGTH]; // Allocate memory for baseChar
        unsigned char* ciphertext = new unsigned char[sizeOfBlock];

        // Define the distribution for the desired range
        uniform_int_distribution<uint64_t> distribution(0, std::numeric_limits<uint64_t>::max());

        for (size_t i = 1; i <= N; i++) {
            leaf = distribution(generator) % noOfLeafs;
            insertion = false;
            size_t stringLength = RAM[i - 1].length();
            numCharsToAdd = sizeOfBlock - stringLength - EVP_MAX_IV_LENGTH; // Calculate chars to add
            numCharsToAdd = std::max(numCharsToAdd, 0);

            if (numCharsToAdd > 0) {
                // Copy the string from RAM
                std::memcpy(baseChar, RAM[i - 1].c_str(), std::min(stringLength, sizeOfBlock - EVP_MAX_IV_LENGTH));

                // Append '/'
                size_t currentPosition = std::min(stringLength, sizeOfBlock - EVP_MAX_IV_LENGTH);
                if (currentPosition < sizeOfBlock - EVP_MAX_IV_LENGTH) {
                    baseChar[currentPosition++] = '/';
                }

                // Fill with '#' if there is space left
                if (numCharsToAdd > 1) { // Subtract 1 for the '/' character
                    std::memset(baseChar + currentPosition, '#', numCharsToAdd - 1); // Fill with '#'
                }
            }
            else {
                // If no chars to add, just copy the string from RAM
                std::memcpy(baseChar, RAM[i - 1].c_str(), sizeOfBlock - EVP_MAX_IV_LENGTH);
            }

            posMap[i] = leaf;
            leaf = readLeaf(leaf);

            while (!insertion) {
                size_t count = Z - 1;
                while (count > 0) {
                    
                    if (data[leaf * Z + count] == nullptr) {
                        data[leaf * Z + count] = new unsigned char[sizeOfBlock];
                        encrypt(baseChar, sizeOfBlock, key, ciphertext);
                       
                        writeBucket(leaf * Z + count, ciphertext);
                        insertion = true;
                        break;
                    }
                    --count;
                }
                if (leaf == 0 && !insertion) {
                    
                    splitBySeparator(baseChar, "x-x", result);
                    
                    stash.emplace(result[0], result[1]);
                    break;
                }
                leaf = getParent(leaf);
            }

        }
        
        for (size_t i = 0; i < treeSize * Z; ++i) 
        {
          
            if (data[i] == nullptr) {
                encrypt(dummy, sizeOfBlock, key, ciphertext);
                data[i] = new unsigned char[sizeOfBlock];
                writeBucket(i, ciphertext);
            }
            
        }

        delete[] baseChar; // Free memory allocated for baseChar
        delete[] ciphertext;
        delete[] result[0];
        delete[] result[1];
        std::cout << "Setup Done !\nclient_stash number of elements #: " << stash.size() << "\n";


    }
   
    void showData()
    {
        cout << "printing: " << treeSize * Z << "\n";
        for (size_t i = 0; i < treeSize * Z; ++i) {
            cout << i << "---> " << data[i] << endl;
        }
    }
    //Getters
    char* getDummy() const {
        return dummy;
    }
    size_t getL() const {
        return L;
    }

    size_t getTreeSize() const {
        return treeSize;
    }

    size_t getNoOfLeafs() const {
        return noOfLeafs;
    }

    size_t getZ() const {
        return Z;
    }

    size_t getsizeOfBlock() const {
        return sizeOfBlock;
    }

    size_t response_size() const {
        return Z * (L + 1);
    }
    const std::unique_ptr<unsigned char* []>& getTree() const {
        return data;
    }
    //Setters
    void setL(const size_t& newL) {
        L = newL;
        // Additional logic to handle changes in L
    }

    void setTreeSize(const size_t& newTreeSize) {
        treeSize = newTreeSize;
        // Additional logic to handle changes in treeSize
    }

    void setNoOfLeafs(const size_t& newNoOfLeafs) {
        noOfLeafs = newNoOfLeafs;
        // Additional logic to handle changes in noOfLeafs
    }

    void setZ(const size_t& newZ) {
        Z = newZ;
        // Additional logic to handle changes in Z
    }

    void setSizeOfBlock(const size_t& newSizeOfBlock) {
        sizeOfBlock = newSizeOfBlock;

    }

    void setPathLength(const size_t& newPathLength) {
        pathLength = newPathLength;

    }

    void setTree(std::unique_ptr<unsigned char* []>& newData) {
        data = std::move(newData); // Transfer ownership from newData to data
        // After this, newData is empty (nullptr), and data owns the array
        newData.release();
    }

    void deleteTree() {
        for(size_t i = 0 ; i < treeSize * Z;++i)
        {
            delete[] data[i];
        }
        data.release();  // Automatically deletes the managed array and sets data to nullptr
    }



    size_t findSubstring(const char* str, const char* substr) {
        const char* result = strstr(str, substr);
        if (result == nullptr) {
            return std::string::npos;
        }
        return result - str;
    }

 
    // The main function to be implemented.
    void check(std::unique_ptr<char* []>& response, std::unordered_map<std::string, std::string>& client_stash, const int& addr, string& doc_id) noexcept {
    
        bool foundIt = false;
        char* currentString;
        string substring = to_string(addr);

        
        // Checking in client_stash
        if (!client_stash.empty()) {
           
            for (const auto& pair : client_stash) {
                if (pair.first.find(substring) != std::string::npos) {
                    doc_id = string(pair.second);
                    foundIt = true;
                   
                    break;
                }
            }
        }

        // Searching in the response
        for (size_t i = 0; i < response_size() && (!foundIt or !client_stash.empty()); ++i) {
            if (response[i][0] == '*' && response[i][sizeOfBlock - 1 - EVP_MAX_IV_LENGTH] == '*') {
                if (!client_stash.empty()) {
                    
                    auto it = client_stash.begin();

                    const std::string& newData = it->first + "x-x" + it->second;
                   
                    strncpy(response[i], newData.c_str(), sizeOfBlock - EVP_MAX_IV_LENGTH);

                    client_stash.erase(it);
                }
                else if (foundIt) break;
                
            }

           
            currentString = strstr(response[i], (substring+"x-x").c_str());
            

            // First, check if substring is present
            if (currentString != nullptr) {
                
                size_t startPos = substring.size() + strlen("x-x");

                // Move the pointer forward to skip the substring and "x-x"
                char* modifiedString = currentString + startPos;

                // Find the first occurrence of "/#"
                char* endPos = strstr(modifiedString, "/");
                if (endPos != nullptr) {
                    // Calculate length from modifiedString to endPos
                    size_t length = endPos - modifiedString;

                    // Copy the modified string into doc_id
                    doc_id = std::string(modifiedString, length);
                }
                else {
                    // If "/#" is not found, use the entire string from modifiedString
                    doc_id = std::string(modifiedString);
                    
                }
                foundIt = true;
            
            }
            else {
                // substring not found, skip to the next
           
                continue;
            }
        }

    }

    void findCommonLevelFromBottom(const size_t* oldPath, const size_t* newPath, size_t& level) const noexcept {

        for (size_t i = 0; i < L + 1; i++) {
   
            if (oldPath[i] == newPath[i]) {
                level = i; // Return the level where paths are the same
                return;
            }
        }
       
        level = -1; // No common level found
    }

    void reAll(std::unique_ptr<char* []>& response, const std::unordered_map<int, size_t>& PosMap, const std::unordered_map<std::string, int>& DXst, const size_t& oldleafNode,
        std::unordered_map<std::string, std::string>& client_stash) noexcept {
        
        std::fill_n(isAltered, Z * pathLength, false);
        char* output[2] = { new char[sizeOfBlock + 1 - EVP_MAX_IV_LENGTH], new char[sizeOfBlock + 1 - EVP_MAX_IV_LENGTH] };
        size_t commonLevel;
        size_t n = response_size();
        size_t key;
        pathNodes(readLeaf(oldleafNode), oldPath);
        
        for (size_t i = 0; i < n; ++i) {
            if (isAltered[i]) continue;
     
            if (response[i][0] != '*' || response[i][sizeOfBlock - 1 - EVP_MAX_IV_LENGTH] != '*') {
                splitBySeparator(response[i], "x-x", output);
               
                pathNodes(readLeaf(PosMap.at(std::stoi(output[0]) + 1)), newPath);
                findCommonLevelFromBottom(oldPath, newPath, commonLevel);
                key = Z * commonLevel; 
             
            
                while (key < n && isAltered[key]) {
                    ++key;
                }
                if (key < n) {
                    std::swap(response[key], response[i]);
                    isAltered[key] = true;
                    --i;
                }
                else {
                    // Need to copy strings into client_stash
                 
                    client_stash.emplace(output[0], output[1]);
                    response[i][0] = '*';
                    response[i][sizeOfBlock - 1 - EVP_MAX_IV_LENGTH] = '*';
                }
            }
        }

        delete[] output[0]; // Free the allocated memory
        delete[] output[1]; // Free the allocated memory
    }

    void reWrite(const std::unique_ptr<unsigned char* []>& request) noexcept{
      
        size_t pathNodeIndex;

        // Traverse each element in the stash
        for (size_t i = 0; i < response_size(); i++) {
            pathNodeIndex = i / Z; // Determine the path node index
            if (pathNodeIndex < pathLength) {
                writeBucket(Z * oldPath[pathNodeIndex] + (i % Z), request[i]);
            }
        }
    }

    void access(const size_t& leafNode, std::unique_ptr<unsigned char* []>& response) noexcept  {
  
        pathNodes(readLeaf(leafNode), oldPath);
        for (size_t i = 0; i < L + 1; ++i) {
            for (size_t j = 0; j < Z; ++j) {
                readBucket(oldPath[i] * Z + j, response[Z * i + j]);
            }
        }
    }

};

