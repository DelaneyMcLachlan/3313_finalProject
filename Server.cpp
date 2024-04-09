#include "thread.h"
#include "socketserver.h"
#include <iostream>
#include <stdlib.h>
#include <time.h>
#include <list>
#include <vector>
#include <thread>
#include <mutex>

using namespace Sync;

std::list<Socket> clientSockets; // global list of all connected clients 
std::mutex clientListMutex;

void BroadcastMessage(const std::string& message) { // iterates over the list of clients and sends the message to each one 
    std::lock_guard<std::mutex> lock(clientListMutex);
    for (auto& clientSocket : clientSockets) {
        ByteArray data(message);
        clientSocket.Write(data);
    }
}


void HandleClient(Socket clientSocket) {

    { // adding new client to the list 
        std::lock_guard<std::mutex> lock(clientListMutex);
        clientSockets.push_back(clientSocket);
    }

    try {
        // Read the username first
        ByteArray usernameData;
        clientSocket.Read(usernameData);
        std::string username = usernameData.ToString();
        std::cout << username << " has entered the chat!" << std::endl;

        while (true) {
            ByteArray data;
            clientSocket.Read(data);

            std::string receivedStr = data.ToString();
            if (receivedStr == "done") {
                std::cout << username << " has left the chat." << std::endl;
                break;
            }

            std::cout << username << ": " << receivedStr << std::endl;


            // Here you can handle other commands or messages from the client
            // For now, let's just echo back the message
            
            // moved these lines inside the if / else block

            // check if the message is a broadcast command
            if (receivedStr.substr(0, 10) == "broadcast:")
            {
                // the actual message to broadcast is everything after the "broadcast:" command
                std::string broadcastMessage = receivedStr.substr(10);

                // call the broadcast function with the message
                BroadcastMessage(broadcastMessage);

                // confirmation gets sent back to the client
                ByteArray confirmationData("Your message has been broadcasted.");
                clientSocket.Write(confirmationData);
            }
            else
            {
                ByteArray echoData(receivedStr);
                clientSocket.Write(echoData);
            }
        }
    } catch (...) {
        std::cerr << "Error occurred with a client connection" << std::endl;
    }

    { // removing client from the list 
        std::lock_guard<std::mutex> lock(clientListMutex);
        clientSockets.remove(clientSocket);
    }
    
    clientSocket.Close();
}

int main(void) {
    std::cout << "Server starting..." << std::endl;

    try {
        SocketServer server(3000); 

        while (true) {
            try {
                // Accept a new client connection
                Socket clientSocket = server.Accept();

                // Create a thread to handle the new client
                std::thread clientThread(HandleClient, std::move(clientSocket));
                clientThread.detach(); // Detach the thread to handle the client independently
            } catch (std::string &e) {
                std::cerr << "Accept failed: " << e << std::endl;
                break;
            }
        }
    } catch (std::string &e) {
        std::cerr << "Could not start server: " << e << std::endl;
    }

    return 0;
}
