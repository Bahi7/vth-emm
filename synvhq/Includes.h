#pragma once
#define _CRT_SECURE_NO_WARNINGS
#include <memory>
#include <thread>
#include <mutex>
#include <deque>
#include <queue>
#include <optional>
#include <vector>
#include <iostream>
#include <algorithm>
#include <chrono>
#include <cstdint>
#include <unordered_map>
#include <cmath>
#include <map>
#include <algorithm>
#include <ctime>
#include <functional>
#include <optional>
#include <random>
#include <vector>
#include <regex>
#include <fstream>
#include <sstream>
#include <string>
#include <future>
#include <openssl/evp.h>
#include <cereal/archives/binary.hpp>
#include <cereal/types/map.hpp>
#include <cereal/types/vector.hpp>
#include <cereal/types/string.hpp>
#include <atomic>
#include <condition_variable>
#include <openssl/evp.h>
#include <openssl/aes.h>
#include <openssl/rand.h>
#include <boost/lockfree/queue.hpp>

double exponentialWait(double lambda);
struct Timer
{
    std::chrono::time_point<std::chrono::high_resolution_clock> start, end;
    std::chrono::duration<float, std::micro> duration;
    std::string where;

    Timer(const std::string place)
    {
        start = std::chrono::high_resolution_clock::now();
        where = place;
    }

    ~Timer()
    {
        end = std::chrono::high_resolution_clock::now();
        duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

        std::cout << where + " Latency: " << duration.count() << " us\n";
    }
};

const int ivSize = EVP_MAX_IV_LENGTH;

void encrypt(const char* plaintext, int plaintext_len, const unsigned char* key,  unsigned char* ciphertext) {

    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    unsigned char iv[ivSize];
    RAND_bytes(iv, ivSize);
    EVP_EncryptInit_ex(ctx, EVP_aes_128_ctr(), NULL, key, iv);

    int len;
    EVP_EncryptUpdate(ctx, ciphertext + EVP_CIPHER_CTX_iv_length(ctx), &len, reinterpret_cast<const unsigned char*>(plaintext), plaintext_len- ivSize);

    std::memcpy(ciphertext, iv, EVP_CIPHER_CTX_iv_length(ctx));

    EVP_CIPHER_CTX_free(ctx);
   
}

void decrypt(const unsigned char* ciphertext, int ciphertext_len, const unsigned char* key, char* plaintext) {
    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    unsigned char iv[ivSize];
    std::memcpy(iv, ciphertext, ivSize);

    EVP_DecryptInit_ex(ctx, EVP_aes_128_ctr(), NULL, key, iv);

    int len;
    EVP_DecryptUpdate(ctx, reinterpret_cast<unsigned char*>(plaintext), &len, ciphertext + EVP_MAX_IV_LENGTH, ciphertext_len - EVP_MAX_IV_LENGTH);

    EVP_CIPHER_CTX_free(ctx);
  
    
}

struct TaskParams {
    char* plaintext;
    int textSize;
    const unsigned char* key;
    unsigned char* ciphertext;
    TaskParams() = default;
    TaskParams(char* pt, int ts, const unsigned char* k, unsigned char* ct)
        : plaintext(pt), textSize(ts), key(k), ciphertext(ct) {}
};



std::mutex mtx;
std::condition_variable cv;

void decryptionWorker(unsigned char*& ciphertext, int textSize,
    const unsigned char* key, char*& plaintext, std::atomic<int>& tasksRemaining) {
    
    decrypt(ciphertext, textSize, key, plaintext);
   
    if (tasksRemaining.fetch_sub(1) == 1) {
        
        std::lock_guard<std::mutex> lock(mtx);
        cv.notify_one(); // Notify the main thread
    }
}

void encryptionWorker(char*& plaintext, int textSize,
    const unsigned char* key, unsigned char*& ciphertext, std::atomic<int>& tasksRemaining) {

    encrypt(plaintext, textSize, key, ciphertext);
    
    if (tasksRemaining.fetch_sub(1) == 1) {
      
        std::lock_guard<std::mutex> lock(mtx);
        cv.notify_one();
    }
}

class ThreadPool {
public:
    ThreadPool(size_t numThreads) : stopFlag(false) {
        start(numThreads);
    }

    ~ThreadPool() {
        stop();
    }

    void enqueue(TaskParams params) {
        while (!tasks.push(std::move(params))) {
           
        }
    }

    void setFuncType(char val) {
        funcType = val;

    }
    void setTaskR(int numElements) {
        tasksRemaining.store(numElements);
    }
    int getTaskR() {
        return tasksRemaining.load();
    }

    // Method to wake up all worker threads
    void notifyAll() {
        wakeUp.store(true);
        cvW.notify_all();
    }

    // Method to reset the wake-up flag
    void resetWakeUp() {
        wakeUp.store(false);
    }

private:
    std::vector<std::thread> workers;
    char funcType;
    std::atomic<bool> stopFlag;
    std::atomic<int> tasksRemaining;
    boost::lockfree::queue<TaskParams, boost::lockfree::capacity<256>> tasks;
    std::mutex cvMutex;
    std::condition_variable cvW;
    std::atomic<bool> wakeUp{ false };

    void start(size_t numThreads) {
        for (size_t i = 0; i < numThreads; ++i) {
            workers.emplace_back([this] {
                while (!stopFlag.load()) {
                    TaskParams params;
                    if (tasks.pop(params)) {
                        
                        if(funcType == 'E') encryptionWorker(params.plaintext, params.textSize, params.key, params.ciphertext, tasksRemaining);
                        else decryptionWorker(params.ciphertext,  params.textSize, params.key, params.plaintext, tasksRemaining);
                    }
                    else {
                        std::unique_lock<std::mutex> lock(this->cvMutex);
                        this->cvW.wait(lock, [this] { return wakeUp.load(); });
                    }
                }
                });
        }
    }
    
    void stop() {
        stopFlag.store(true);
        for (std::thread& worker : workers) {
            worker.join();
        }
    }
};



void encryptAll(ThreadPool& pool, std::unique_ptr<char* []>& plaintexts, int numElements, int textSize, const unsigned char* key, std::unique_ptr<unsigned char* []>& ciphertexts) {
    std::atomic<int> tasksRemaining(numElements);

   
    pool.setFuncType('E');
    pool.setTaskR(numElements);
    pool.notifyAll();
    for (int i = 0; i < numElements; ++i) {
        TaskParams params = { plaintexts[i], textSize, key, ciphertexts[i] };
        pool.enqueue(params);
    }

    // Wait for all tasks to complete
    std::unique_lock<std::mutex> lock(mtx);
    cv.wait(lock, [&]() { return pool.getTaskR() == 0; });
    pool.resetWakeUp();
    
}


void decryptAll(ThreadPool& pool, std::unique_ptr<unsigned char* []>& ciphertexts,
    int numElements, int textSize, const unsigned char* key,
    std::unique_ptr<char* []>& plaintexts) {

    pool.setTaskR(numElements);

    pool.setFuncType('D');
    
    pool.notifyAll();
    for (int i = 0; i < numElements; ++i) {
       
        TaskParams params = { plaintexts[i], textSize, key, ciphertexts[i] };
        pool.enqueue(params);
    }
  
    std::unique_lock<std::mutex> lock(mtx);
    cv.wait(lock, [&]() { return pool.getTaskR() == 0; });
    pool.resetWakeUp();
  
}

   

//Sequential Decryption / Encryption

//void encryptAll(std::unique_ptr<char* []>& plaintexts, int numelements, int textsize, const unsigned char* key, std::unique_ptr<unsigned char* []>& ciphertexts) {
//    for (int i = 0; i < numelements; ++i) {
//        if (plaintexts[i] != nullptr) {
//            encrypt(plaintexts[i], textsize, key, ciphertexts[i]);
//        }
//        else {
//            ciphertexts[i] = nullptr;
//        }
//    }
//}

//void decryptAll(std::unique_ptr<unsigned char* []>& ciphertexts, int numElements, int textSize, const unsigned char* key, std::unique_ptr<char* []>& plaintexts) {
//    for (int i = 0; i < numElements; ++i) {
//        if (ciphertexts[i] != nullptr) {
//            decrypt(ciphertexts[i], textSize, key, plaintexts[i]);
//        }
//        else {
//            plaintexts[i] = nullptr;
//        }
//    }
//}



// Function to derive a key from a password using PBKDF2
void deriveKeyFromPassword(const std::string& password, const unsigned char* salt, unsigned char* key, size_t keyLength) {
    const int iterations = 10000; // Number of iterations (can be adjusted)

    PKCS5_PBKDF2_HMAC(password.c_str(), password.length(),
        salt, sizeof(salt), iterations, EVP_sha256(),
        keyLength, key);
}



void preciseSleep(double seconds) {

    using namespace std;
    using namespace std::chrono;

    static double estimate = 5e-3;
    static double mean = 5e-3;
    static double m2 = 0;
    static int64_t count = 1;

    while (seconds > estimate) {
        auto start = high_resolution_clock::now();
        this_thread::sleep_for(milliseconds(1));
        auto end = high_resolution_clock::now();

        double observed = (end - start).count() / 1e9;
        seconds -= observed;

        ++count;
        double delta = observed - mean;
        mean += delta / count;
        m2 += delta * (observed - mean);
        double stddev = sqrt(m2 / (count - 1));
        estimate = mean + stddev;
    }

    // spin lock
    auto start = high_resolution_clock::now();
    while ((high_resolution_clock::now() - start).count() / 1e9 < seconds);
}
#define WINVER 0x0A00
#define _WIN32_WINNT 0x0A00

#define ASIO_STANDALONE
#include <asio.hpp>
#include <asio/ts/buffer.hpp>
#include <asio/ts/internet.hpp>