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

void HandleClient(Socket clientSocket) {
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
            ByteArray echoData(receivedStr);
            clientSocket.Write(echoData);
        }
    } catch (...) {
        std::cerr << "Error occurred with a client connection" << std::endl;
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
