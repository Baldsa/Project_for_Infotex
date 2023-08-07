#include <iostream>
#include <string>
#include <cstdlib>
#include <cstring>
#include <arpa/inet.h>
#include <unistd.h>
using namespace std;

void processReceivedData(const string& data) {
    if (data.size() > 2 && data.size() % 32 == 0) {
        cout << "Received valid data: " << data << endl;
    }
    else {
        cout << "Received invalid data: " << data << endl;
    }
}

int main() {
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == -1) {
        cerr << "Error creating socket." << endl;
        return 1;
    }

    sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = INADDR_ANY;
    serverAddress.sin_port = htons(5555); // Порт для связи

    if (bind(serverSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) == -1) {
        cerr << "Error binding socket." << endl;
        close(serverSocket);
        return 1;
    }

    if (listen(serverSocket, 1) == -1) {
        cerr << "Error listening on socket." << endl;
        close(serverSocket);
        return 1;
    }

    cout << "Waiting for data from Program 1..." << endl;

    while (true) {
        sockaddr_in clientAddress;
        socklen_t clientAddressLength = sizeof(clientAddress);

        int clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddress, &clientAddressLength);
        if (clientSocket == -1) {
            cerr << "Error accepting connection." << endl;
            continue;
        }

        char buffer[1024];
        memset(buffer, 0, sizeof(buffer));

        int bytesReceived = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
        if (bytesReceived == -1) {
            cerr << "Error receiving data." << endl;
            close(clientSocket);
            continue;
        }

        string receivedData(buffer);
        processReceivedData(receivedData);

        close(clientSocket);
    }

    close(serverSocket);

    return 0;
}
