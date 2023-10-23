#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cmath>
#include <map>
#include <algorithm>
#include <cstdlib>
#include <ctime>
#include <functional>
#include <optional>



using namespace std; // This line will allow you to use everything from the standard library without prefixing it with std::
class PathOram {

private:

    int N, L, noOfLeafs, treeSize;
    vector<int> tree;
    map<int, int> posMap;
    map<int, string> data;
    int Z = 4;
    
    
    int readLeaf(const int& branch) {
        return static_cast<int>(pow(2, L)) + branch - 1;
    }

    int getParent(const int& node) {
        return (node - 1) / 2;
    }

    vector<int> pathNodes(int branch) {
        vector<int> path;
        for(int i = 0; i <= L; i++) {
            path.push_back(branch);
            branch = getParent(branch);
        }
        reverse(path.begin(), path.end());
        return path;
    }

    int randomPath(const int& node) {
        int randVal = rand() % 2;
        int child1 = 2 * node + 1;
        int child2 = 2 * node + 2;
        if(child2 > treeSize - 1) {
            return node - (treeSize / 2);
        } else {
            if(randVal == 0) {
                return randomPath(child1);
            } else {
                return randomPath(child2);
            }
        }
    }

    string readBucket(const int& block) {
        
        return data[block];
    }

    void writeBucket(const int& block, const string& new_data) {
       
        data[block] = new_data;
    }

public:

    void get_PosMap(map<int, int>& output){
        output = posMap;
    }

    void set_PosMap(const map<int, int>& input){
        posMap = input;
    }
    map<int, string> access(const int& x,  map<int, string> stash_data,  vector<int> stash) {

  
        // 3-5
        int leafNode = readLeaf(x);
        vector<int>path = pathNodes(leafNode);
        for (int node : path) {
            int blocks = tree[node];
            stash.push_back(blocks);
            for(int i=0; i<Z; i++){
                stash_data[blocks*Z + i] = readBucket(blocks*Z + i);
            }
            
        }
        map<int, string> response = stash_data;
        // 10-15
        for (auto it = path.rbegin(); it != path.rend(); ++it) {
            int node = *it;
            int n = tree[node];
            for(int i=0; i < Z;i++){
                writeBucket(n*Z+i, stash_data[n*Z+i]);
            }
            
            
            for (size_t i = 0; i < stash.size();) {
                if (stash[i] == 0) {
                    stash.erase(stash.begin() + i);
                    for(int j=0; j<Z;j++){
                        stash_data.erase(j);
                    }
                                      
                } else {
                    int current_branch = posMap[stash[i]];
                    vector<int> a = pathNodes(current_branch);
                    if (find(a.begin(), a.end(), node) != a.end()) {
                        tree[node] = stash[i];
                        for(int j=0; j<Z; j++){
                            stash_data.erase(stash[i]*Z+j);

                        }
                        stash.erase(stash.begin() + i);
                       
                    }     
                } 
            ++i;     
            }

        }
        return response; 
    }

    PathOram(const vector<string> RAM) {

        cout << "Starting setup" << endl;
        
        N = RAM.size();

        cout << "RAM size:" << N << endl;
        L = static_cast<int>(ceil(log2(N)) - 1);
        noOfLeafs = pow(2, L) ;


        cout << "number of leafs: " << noOfLeafs << endl;
        
        treeSize = pow(2, L + 1) - 1;
        tree.resize(treeSize, 0);

        
        for(int i = 1; i <= N; i++) {
            tree[i] = i;
        }
      
        random_shuffle(tree.begin(), tree.end());

       
        for(int x = 1; x <= N; x++) {
            posMap[x] = rand() % noOfLeafs;
        }

        for(int i = 0; i < treeSize; i++) {

            if(tree[i] != 0) {
                posMap[tree[i]] = randomPath(i);
            }
        }

        for(int i : tree) {
            int k = rand() % Z;
            for(int j=0; j < Z; j++){
                data[Z*i + j] = "Dummy";
            }
            if (i != 0) data[Z*i + k] = RAM[i-1];
        }

        cout << "Setup Done !" << endl;
    }


};

/*int main() {
   

    
    vector<string>RAM = {"Value1", "Value200", "Value001", "Value2222", "Value1111", "Value2444", "Value1234",  
    "cc", "cc", "cc","cc", "cc", "cc", "cc", "cc", "cc","cc","Value2142", "Value1444", "Valuellll"};
    PathOram inst(RAM);
    map<int, int> stRAM;
    inst.get_PosMap(stRAM);

    int addr =20;
    int x = stRAM[addr];
    int noOfLeafs = pow(2, static_cast<int>(ceil(log2(RAM.size())) - 1));
    stRAM[addr] = rand() % (noOfLeafs - 1);
    inst.set_PosMap(stRAM);
    map<int, string> stash_data;
    vector<int> stash;
    map<int, string> re = inst.access(x, stash_data, stash);
    std::cout << "stash_data:" << std::endl;
    for(const auto& [key, value] : re) {
        std::cout << key << " -> " << value << std::endl;
    }
    

    x = stRAM[addr];
    stRAM[addr] = rand() % (noOfLeafs - 1);
    re = inst.access(x, stash_data, stash);
    std::cout << "stash_data:" << std::endl;
    for(const auto& [key, value] : re) {
        std::cout << key << " -> " << value << std::endl;
    }

    x = stRAM[addr];
    stRAM[addr] = rand() % (noOfLeafs - 1);
    inst.set_PosMap(stRAM);
    re = inst.access(x, stash_data, stash);
    std::cout << "stash_data:" << std::endl;
    for(const auto& [key, value] : re) {
        std::cout << key << " -> " << value << std::endl;
    }
    return 0;

}*/