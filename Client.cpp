#include "socket.h"
#include <iostream>
#include <string>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>

using namespace Sync;

std::mutex messageMutex;
std::condition_variable messageCondVar;
bool hasMessage = false;
std::string messageToSend;


void ListenForMessages(Socket& socket) {
    try {
        while (true) {
            ByteArray data;
            socket.Read(data);
            if (data.v.size() > 0) {
                std::cout << "\r" + data.ToString() << std::endl;
                std::cout << "> "; // Prompt for next message
                std::cout.flush();
            }
        }
    } catch (...) {
        std::cerr << "Error occurred while listening for messages." << std::endl;
    }
}

void SendMessage(Socket& socket) {
    while (true) {
        std::unique_lock<std::mutex> lock(messageMutex);
        messageCondVar.wait(lock, []{return hasMessage;});

        // Send the message to the server
        ByteArray data(messageToSend);
        socket.Write(data);
        
        hasMessage = false;
        lock.unlock();
    }
}

void UserInputHandler(Socket& socket) {
    while (true) {
        std::string input;
        std::getline(std::cin, input); // Blocks until input
        
        {
            std::lock_guard<std::mutex> lock(messageMutex);
            messageToSend = input;
            hasMessage = true;
        }   
        messageCondVar.notify_one();
    }
}


int main(void) {
    std::string serverAddress = "127.0.0.1";
    int serverPort = 3000;
    
    try {
        Socket socket(serverAddress, serverPort);
        socket.Open();
        std::cout << "Connected to the server." << std::endl;

        std::cout << "Enter your username: ";
        std::string username;
        std::getline(std::cin, username);
        ByteArray usernameData(username);
        socket.Write(usernameData);

        // Threads for listening to messages and sending messages
        std::thread listenThread(ListenForMessages, std::ref(socket));
        std::thread sendThread(SendMessage, std::ref(socket));

        // The main thread handles user input 
        UserInputHandler(socket);
        
        // Wait for the threads to finish (they won't in this case, so you'll need to handle termination properly)
        listenThread.join();
        sendThread.join();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    } catch (...) {
        std::cerr << "An unknown error occurred." << std::endl;
    }

    return 0;
}