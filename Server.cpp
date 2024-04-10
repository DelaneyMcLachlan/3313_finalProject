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

void BroadcastMessage(const std::string& senderUsername, const std::string& message) {
    std::lock_guard<std::mutex> lock(clientListMutex);
    std::string fullMessage = senderUsername + ": " + message + "\n";
    ByteArray data(fullMessage);
    for (auto& clientSocket : clientSockets) {
        clientSocket.Write(data);
    }
}


void HandleClient(Socket clientSocket) {
    // Adding new client to the list 
    {
        std::lock_guard<std::mutex> lock(clientListMutex);
        clientSockets.push_back(clientSocket);
    }

    std::string username;

    try {
        // Read the username first
        ByteArray usernameData;
        clientSocket.Read(usernameData);
        username = usernameData.ToString();
        std::cout << username << " has entered the chat!" << std::endl;

        // Broadcast the 'entered chat' notification
        BroadcastMessage("Server", username + " has entered the chat!");

        while (true) {
            ByteArray data;
            clientSocket.Read(data);

            std::string receivedStr = data.ToString();
            if (receivedStr == "done") {
                std::cout << username << " has left the chat." << std::endl;
                // Broadcast the 'left chat' notification
                BroadcastMessage("Server", username + " has left the chat.");
                break;
            }

            // Broadcast the received message to all clients
            BroadcastMessage(username, receivedStr);
        }
    } catch (...) {
        std::cerr << "Error occurred with a client connection" << std::endl;
        if (!username.empty()) {
            // Broadcast the 'error' notification
            BroadcastMessage("Server", username + " has been disconnected due to an error.");
        }
    }

    // Removing client from the list 
    {
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
