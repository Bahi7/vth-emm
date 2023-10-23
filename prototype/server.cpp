#include <iostream>
#include <winsock2.h>
#include "LDX.cpp"
#include "PathOram.cpp"
#include <sstream>
#include <string>
#include <vector>
#include <cereal/cereal.hpp>
#include <cereal/archives/binary.hpp>
#include <cstring>

int receiveData(SOCKET clientSocket, char* buffer, int bufferSize) {
    int bytesReceived = recv(clientSocket, buffer, bufferSize - 1, 0);
    if (bytesReceived == SOCKET_ERROR) {
        int error = WSAGetLastError();
        if (error == WSAECONNRESET) {
            std::cerr << "Connection was reset by the client. Error code: " << error << std::endl;
            // Handle the disconnection (e.g., cleanup resources, try reconnecting, etc.)
        } else {
            std::cerr << "Error while receiving data. Error code: " << error << std::endl;
            // Handle other errors as necessary.
        }
        return -1;
    } else if (bytesReceived == 0) {
        std::cerr << "Client closed the connection." << std::endl;
        // Handle client gracefully closing the connection.
        return 0;
    }
    return bytesReceived;
}
bool isClientConnected(SOCKET clientSocket) {
    fd_set readfds;
    FD_ZERO(&readfds);
    FD_SET(clientSocket, &readfds);

    timeval tv;
    tv.tv_sec = 0;  // Set the timeout to zero, so select returns immediately.
    tv.tv_usec = 0;

    int selectResult = select(clientSocket + 1, &readfds, NULL, NULL, &tv);
    if (selectResult > 0) {
        // There's something to read. This could be actual data, or it could be a notification 
        // that the client has closed the connection.
        char buffer[1];
        int recvResult = recv(clientSocket, buffer, 1, MSG_PEEK);
        if (recvResult <= 0) {
            // Either an error occurred or the client closed the connection.
            return false;
        } else {
            // Data is available to read.
            return true;
        }
    } else if (selectResult == 0) {
        // No data to read, but the client is still connected.
        return true;
    } else {
        // An error occurred in select.
        return false;
    }
}



int main() {

   WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "Failed to initialize Winsock." << std::endl;
        return 1;
    }

    SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == INVALID_SOCKET) {
        std::cerr << "Failed to create socket." << std::endl;
        return 1;
    }

    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(12345);
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    if (bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        std::cerr << "Bind failed." << std::endl;
        return 1;
    }

    listen(serverSocket, 1);
    cout << "Waiting for connection..." << endl;

    SOCKET clientSocket = accept(serverSocket, NULL, NULL);
    if (clientSocket == INVALID_SOCKET) {
        cerr << "Accept failed." << std::endl;
        return 1;
    }
    cerr << "Accepted." << std::endl;
    vector<string> RAM;
    char buffer[1024];
    int bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0);

    if (bytesReceived <= 0) {
        if (bytesReceived < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                std::cerr << "No data available to read. Socket is non-blocking." << std::endl;
            } else {
                std::cerr << "Error while receiving data." << std::endl;
            }
        } else {
            std::cerr << "Connection closed by client." << std::endl;
        }
    } else {
        istringstream is(string(buffer, bytesReceived)); // Directly create a string from buffer with the exact size.

        try {
            cereal::BinaryInputArchive archive(is);
            archive(RAM);
            // handle the deserialized RAM as needed
        } catch (const cereal::Exception &e) {
            std::cerr << "Deserialization error: " << e.what() << std::endl;
        } catch (const std::exception &e) {
            std::cerr << "Error: " << e.what() << std::endl;
        }
    }

    cout << "Checking Received RAM..." << endl;
    for (const string& value : RAM) {

            cout << value << endl;
    }

    PathOram oramInstance(RAM);

    map<int, int> stRAM_out;
    map<int, string> stash_data_out;
    vector<int> stash_out;
    map<int, string> response;

    oramInstance.get_PosMap(stRAM_out);

    ostringstream os;
    {
        cereal::BinaryOutputArchive archive(os);
        archive(stRAM_out);
    }
    string serializedVec = os.str();
    cerr << "sending..." << std::endl;
    cout << "Size of PosMap data: " << serializedVec.size() << " bytes" << std::endl;
    send(clientSocket, serializedVec.c_str(), serializedVec.size(), 0);

    // Clearing the buffer
    
    cout << "Query time " << sizeof(buffer) <<endl;
    stRAM_out.clear();
    
    while (true) {
        cout << ">----------------------------<" << endl;
        memset(buffer, 0, sizeof(buffer));
        bytesReceived = receiveData(clientSocket, buffer, sizeof(buffer));
        istringstream is(string(buffer, bytesReceived));
        DataPacket query;
        

        try {
            cereal::BinaryInputArchive archive(is);
            archive(query);
        } catch (const cereal::Exception &e) {
            std::cerr << "Deserialization error: " << e.what() << std::endl;
            break;
           
        } catch (const std::exception &e) {
            std::cerr << "Error: " << e.what() << std::endl;
           
        }

        oramInstance.set_PosMap(query.stRAM);

        memset(buffer, 0, sizeof(buffer));
        // Receiving int (leaf)
        bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0);
        if (bytesReceived <= 0) {
            cerr << "Error receiving data or connection closed by client." << std::endl;
            break;  // Exit the loop
        }

        istringstream is2(string(buffer, bytesReceived));
        int leaf;
        try {
            cereal::BinaryInputArchive archive(is2);
            archive(leaf);
        } catch (const cereal::Exception &e) {
            std::cerr << "answer Deserialization error: " << e.what() << std::endl;
            continue;  // Skip to next iteration
        } catch (const std::exception &e) {
            std::cerr << "Error: " << e.what() << std::endl;
            continue;  // Skip to next iteration
        }
      
        response = oramInstance.access(leaf, query.stash_data, query.stash);

        os.str("");        // Clear the contents
        os.clear();    // Reset the ostringstream
        {
            cereal::BinaryOutputArchive archive(os);
            archive(response);
        }
        string serializedString = os.str();
        send(clientSocket, serializedString.c_str(), serializedString.size(), 0);
        cerr << "Sending response." << endl;
}

    
    closesocket(clientSocket);
    closesocket(serverSocket);
    WSACleanup();
    return 0;


}
