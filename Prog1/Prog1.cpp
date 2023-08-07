#include <iostream>
#include <string>
#include <cctype>
#include <algorithm>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <cstdlib>
#include <cstring>
#include <arpa/inet.h>
#include <unistd.h>
using namespace std;

class DataProcessor {
private:
    mutex bufferMutex;
    condition_variable cv;
    string sharedBuffer;
    bool newDataAvailable;
public:
    DataProcessor() : newDataAvailable(false) {}

    void processInput() {
        while (true) {
            string user_s;
            cin >> user_s;

            if (user_s.size() > 64) {
                cout << "String length exceeds 64 characters." << endl;
                continue;
            }

            bool validInput = true;
            for (char c : user_s) {
                if (!isdigit(c)) {
                    validInput = false;
                    break;
                }
            }

            if (!validInput) {
                cout << "Enter a valid string containing only digits." << endl;
                continue;
            }
            int sum = 0;
            sort(user_s.rbegin(), user_s.rend()); // Сортировка по убыванию
            for (int i = 0; i < user_s.size(); ++i) {
                if ((user_s[i] - '0') % 2 == 0) {
                    user_s.replace(i, 1, "KB");
                    i += 1;
                }
                else {
                    sum += (user_s[i] - '0');
                }
            }
            cout << "Data : " << user_s << endl 
                 << "Sum : " << sum << endl;
            lock_guard<mutex> lock(bufferMutex);
            sharedBuffer = user_s;
            newDataAvailable = true;
            cv.notify_all(); // Оповещение о новых данных
        }
    }

    void processBuffer() {
        while (true) {
            unique_lock<mutex> lock(bufferMutex);

            // Ожидание новых данных
            cv.wait(lock, [this] { return newDataAvailable; });

            string data = sharedBuffer;
            newDataAvailable = false;
            lock.unlock(); // Освобождение мьютекса

            // Анализ данных и отправка через сокет
            processAndSendData(data);
        }
    }

    void processAndSendData(const string& data) {
            sendData(data);
    }

    void sendData(const string& data) {
        int clientSocket = socket(AF_INET, SOCK_STREAM, 0);
        if (clientSocket == -1) {
            cerr << "Error creating socket." << endl;
            return;
        }

        sockaddr_in serverAddress;
        serverAddress.sin_family = AF_INET;
        serverAddress.sin_addr.s_addr = inet_addr("127.0.0.1"); // Адрес сервера (адрес локального хоста)
        serverAddress.sin_port = htons(5555); // Порт для связи

        if (connect(clientSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) == -1) {
            cerr << "Error connecting to server." << endl;
            close(clientSocket);
            return;
        }

        int bytesSent = send(clientSocket, data.c_str(), data.size(), 0);
        if (bytesSent == -1) {
            cerr << "Error sending data." << endl;
        }

        close(clientSocket);
    }


};

int main() {
    DataProcessor processor;

    thread inputThread(&DataProcessor::processInput, &processor);
    thread bufferThread(&DataProcessor::processBuffer, &processor);

    inputThread.join();
    bufferThread.join();

    return 0;
}


















