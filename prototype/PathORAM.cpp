#include "utils.h"


using namespace std; // This line will allow you to use everything from the standard library without prefixing it with std::
class PathOram {

private:

    int L;
    vector<string> data;
    string dummy;
    int Z = 4;


    int readLeaf(const int& branch) {
        return static_cast<int>(pow(2, L)) + branch - 1;
    }

    int getParent(const int& node) {
        return (node - 1) / 2;
    }

    vector<int> pathNodes(int branch) {
        vector<int> path;
        for (int i = 0; i <= L; i++) {
            path.push_back(branch);
            //branch = getParent(branch);
            --branch;
            branch /= 2;
        }
        //reverse(path.begin(), path.end());
        return path;
    }



    string readBucket(const int& block) {

        return data[block];
    }

    void writeBucket(const int& block, const string& new_data) {

        data[block] = new_data;
    }

    static void generateRandomString(size_t maxLength, string& result) {
        static const char alphanum[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";

        // Ensure that maxLength is at least 2 for the start and end '*'
        if (maxLength < 2) {
            result = string(maxLength, '*');
            return;
        }

       
        result += '*'; // Start with *

        for (size_t i = 0; i < maxLength - 2; ++i) {
            result += alphanum[rand() % (sizeof(alphanum) - 1)]; // Append a random character
        }

        result += '*'; // End with *

        
    }



public:

    // Size function
   size_t size() const {
        size_t size = 0;
        // Add the size of primitive types
        size += sizeof(L)  + sizeof(Z);
        // Add the size of each string in the vector
        for (const std::string& str : data) {
            // Add the size of the string content plus the size needed for length (usually small overhead)
            size += str.size();
        }

        return size;
    }
   int getNoLeafs() {
       return static_cast<int>(pow(2, L));
    }
   vector<string> getTree() {
       return data;
   }
    
    PathOram() : L(0), Z(4), data() {
        // Initialize with default values or logic
    }

   
    template<class Archive>
    void serialize(Archive& archive)
    {
        archive(L, data, Z);
    }
    void deletTree() {
        data.clear();
    }

    void showData() {
        int count = 0;
       
        for (const string& vale : data) {
            cout << count++ << "---<" << vale << endl;
              

        }
            

    }

    static void splitBySeparator(const std::string& input, const std::string& separator, std::vector<std::string>& result) {
    
        // Escape special characters in the separator for regex
        std::string escapedSeparator = std::regex_replace(separator, std::regex("([.^$|()\\[\\]{}*+?\\\\])"), "\\$1");
        std::regex pattern("(.*?)" + escapedSeparator + "(.*)");  // Pattern to split around the separator

        std::smatch matches;
        std::string currentInput = input;
        result.clear();

        while (std::regex_match(currentInput, matches, pattern)) {
            result.push_back(matches[1].str()); // Add the first part before the separator
            currentInput = matches[2].str();    // Continue with the rest of the string
        }

        if (!currentInput.empty()) {
            result.push_back(currentInput); // Add the remaining part of the string
        }

    }

    void check(vector<string>& response_stash, unordered_map<string, string>& client_stash, const string & substring, string& doc_id) {
     
        std::regex pattern("/#*");
        size_t originalSize = response_stash.size();
        size_t pos;
        string modifiedValue;
        vector<string>::iterator it = response_stash.begin();
        bool foundIt;
        try {
            foundIt = true;
            
            modifiedValue = client_stash.at(substring);
            pos = modifiedValue.find(substring);
            if (pos != std::string::npos) {
                modifiedValue.erase(pos, substring.length());
                doc_id = std::regex_replace(modifiedValue, pattern, ""); 
            }
           
          
        }
        catch (const std::out_of_range& e) {
            // Handle the case where the key is not found
            foundIt = false;
          
        }

        while (it != response_stash.end()) {
            
            
            if (it->front() == '*' && it->back() == '*') {
              
                //cout << "Removing item from response_stash: " << *it << endl;
                it = response_stash.erase(it); // Erase the item

                if (!client_stash.empty()) {
                    //cout << "Retrieving from client_stash: " << client_stash.back() << endl;

                   // Retrieve an iterator to the first element in the unordered_map
                    auto ict = client_stash.begin();

                    // Assuming response_stash is a vector that stores the same type as the values in client_stash
                    response_stash.push_back(ict->second);
                    //cout << "Successfully moved item from client_stash to response_stash. " << ict->first << endl;
                    // Remove the key-value pair from client_stash
                    client_stash.erase(ict);
                    
                }
                else {
                    //cout << "client_stash is empty, breaking out of loop." << endl;
                    if(foundIt) break; // Break out of the loop if client_stash is empty
                }
            }
            else {
                if (!foundIt) {

                    pos = it->find(substring);
                    if (pos != std::string::npos) {
                        modifiedValue = *it;
                        modifiedValue.erase(pos, substring.length());
                        doc_id = std::regex_replace(modifiedValue, pattern, "");
                        foundIt = true;
                      
                    }
                }
                ++it; // Only increment if no erasure occurred
            }
            //cout << "Continuing loop." << endl;
        }

        // Fill the response_stash with dummy items if its size decreased
        while (response_stash.size() < originalSize) {
            //cout << "Adding dummy item to response_stash." << endl;
            response_stash.push_back(dummy);
        }
        //cout << "Function complete." << endl;
    }


    static void findCommonLevelFromBottom(const vector<int>& oldPath, const vector<int>& newPath, int& level) {
        if (oldPath.size() != newPath.size()) {
            level = -1; // Paths are not the same length, return -1 or handle as needed
            return;
        }

        for (size_t i = 0; i < oldPath.size(); i++) {
            if (oldPath[i] == newPath[i]) {
                //cout << "shoe me had level " << oldPath[i] << endl;
                level = i; // Return the level where paths are the same
                return;
            }
        }

        level =  -1; // No common level found
    }
    void reAll(vector< string>& stash,const unordered_map<int, int>& PosMap, const unordered_map< string, int>& DXst, const int& oldleafNode,unordered_map<string, string>& client_stash) {
        
        //std::chrono::system_clock::time_point timeThen = std::chrono::system_clock::now();;
        //cout << "Starting reAll function" << endl;
        //auto timeThen = std::chrono::system_clock::now();

        vector<int> oldPath = pathNodes(readLeaf(oldleafNode));
        std::vector<bool> isAltered(stash.size(), false);
     
        
  
        vector<string> output;
        vector<int> newPath;
        int key;
        int commonLevel;
        int index;
        int count = 0;

        for (string& item : stash) {
            if (isAltered[count]) continue;
            count++;
            //cout << item  <<"  Still here @" << endl;
            if (item.front() != '*' or item.back() != '*') {
                splitBySeparator(item, "xU-Px", output);
                //cout << output[0] << endl;

                auto it = DXst.find(output[0]);
                if (it != DXst.end()) {
                    index = it->second;
                    auto leafOfItem = PosMap.find(index + 1);
                    newPath = pathNodes(readLeaf(leafOfItem->second));
                    findCommonLevelFromBottom(oldPath, newPath, commonLevel);
                    //cout << commonLevel << " " << endl;
                    key = Z * commonLevel;
                    //cout << "first " << key << endl;

                    while (!isAltered[key] && key < Z*(L+1)) {
                        ++key; // Decrement key if it already exists
                    }
                    if (key < Z * (L + 1)) {
                        //cout << "inserting stash " << key << endl;

                        std::swap(stash[key], item);
                        isAltered[key] = true;
                    }

                    else {

                        //cout << "inserting client stash " << key << endl;

                        client_stash.emplace(output[0] + "xU-Px", item); // Place in client_stash if not placed
                    }

                }
            }

            
        }
      

       // replace_if(newStash.begin(), newStash.end(), [](const auto& s) { return s.empty(); }, dummy);

   
        //std::chrono::system_clock::time_point timeNow = std::chrono::system_clock::now();
        //std::cout << "RRE Done Ping: " << std::chrono::duration_cast<std::chrono::microseconds>(timeNow - timeThen).count() << " micros\n";
    }



    void reWrite(const int &oldleafNode, const vector<string>& stash) {



        vector<int> path = pathNodes(readLeaf(oldleafNode));

       
        int pathNodeIndex;
        /*/int block;
        for(const auto& vale : path) {
            std::cout <<  "----"<<vale<<"----" << std::endl;
        }
        */
        // Traverse each element in the stash
        for (int i = 0; i < stash.size(); i++) {
            pathNodeIndex = i / Z; // Determine the path node index
            if (pathNodeIndex < path.size()) {
               // int block = Z * path[pathNodeIndex] + (i % Z); // Calculate block number
                writeBucket(Z * path[pathNodeIndex] + (i % Z), stash[i]);
                if (stash[i] == "") {
                    cout << "What !!!" << endl;
                }
            }
        }
    }

    void access(const int& leafNode, vector<string>& stash) {

        //cout << "Access " << readLeaf(leafNode) << endl;
        //int counter = 0;

        vector<int> path = pathNodes(readLeaf(leafNode));
        for (const int node : path) {
            for (int i = 0; i < Z; i++) {
                if (readBucket(node * Z + i) == "") {
                    //cout << "What !!!" <<   " this node " << node << endl;
                }
                stash.push_back(readBucket(node * Z + i));
                //cout << readBucket(node * Z + i) << endl;
            }
        }
        //cout << counter << " response length"<< endl;
    }

    PathOram(const vector<string>& RAM, unordered_map<int, int>& posMap, const size_t& maxLength, unordered_map<string,string>& stash) {

        cout << "Starting setup" << endl;

   

        vector<int> tree;

        cout << "The length of the longest string is: " << maxLength << std::endl;

        generateRandomString(maxLength, dummy);
       // cout << dummy << endl;;

        size_t N = RAM.size();

        cout << "RAM size:" << N << endl;

        
        L = static_cast<int>(ceil(log2(N)));
        size_t noOfLeafs = pow(2, L);


        cout << "number of leafs: " << noOfLeafs << endl;

        cout << "height: " << L << endl;

        size_t treeSize = pow(2, L + 1) - 1;
      
        data.resize(Z*treeSize, dummy);

       
        int numCharsToAdd;
        string baseString;
        bool insertion;
        vector<string> result;
        int leaf =-1;

        for (int i = 1; i <= N; i++) {

            leaf++; //rand() % noOfLeafs;
         
            insertion = false;
      
            posMap[i] = leaf;
            // read leaf here converts the number of the leaf to the actual position of the leaf in the vector for instance in a tree of 4 leafs leaf 0 is the node 3 in the vector
            leaf = readLeaf(leaf);
 
            while (!insertion) {
                int count = Z - 1;
                while (count > 0) {
                    // check if this block is a dummy , because dummies are of this form *...* to replace it
                   
                    if (data[leaf * Z + count].front() == '*' && data[leaf * Z + count].back() == '*') {
                        //this area is to pad the values so that they are of the same length
                        numCharsToAdd = maxLength - RAM[i - 1].size();  // Calculate how many characters to add
                        numCharsToAdd = std::max(numCharsToAdd, 0);
                        if (numCharsToAdd > 0) {
                            // Create a string of '0's and append to baseString
                            baseString = RAM[i - 1] + '/' + std::string(numCharsToAdd - 1, '#');
                        }
                        else {
                            baseString = RAM[i - 1];
                        }
                        //write backet is just an assignment data[leaf * Z + count] = basestring
                        writeBucket(leaf * Z + count, baseString);
                        //if the element is inserted we break to the next one
                        insertion = true;
                        break;
                    }
                    //if  the last block is non empty that is it has non dummy element we move to the next block of the same node count--
                    count--;
                }
                //if we re at the root and already all blocks are non empty we put in the stash
                if (leaf == 0 and !insertion) {
                   
                   
                    numCharsToAdd = maxLength - RAM[i - 1].size();  // Calculate how many characters to add
                    numCharsToAdd = std::max(numCharsToAdd, 0);
                    if (numCharsToAdd > 0) {
                        // Create a string of '#'s and append to baseString
                        baseString = RAM[i - 1] + '/' + std::string(numCharsToAdd - 1, '#');
                    }
                    else {
                        baseString = RAM[i- 1];
                    }
                
                    splitBySeparator(RAM[i - 1], "xU-Px", result);
                    stash.emplace(result[0] + "xU-Px", baseString);
                    break;
                }
                // the parent of a leaf node is ( node - 1) // 2
                leaf = getParent(leaf);
            
            }
        }
        //showData();
        size_t size = 0;
        for (const auto [key, value] : stash) {
            // Add the size of the string content plus the size needed for length (usually small overhead)
            size += key.size() + value.size();
        }
        cout << "Setup Done !\nclient_stash size #: " << stash.size() << " \nclient_stash size in Bytes: " << static_cast<double>(size) / 1024 /1024 << " MB\n";
      
    }

};
