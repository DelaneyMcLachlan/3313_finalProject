#include "socket.h"
#include <iostream>
#include <string>

using namespace Sync;


int main(void) {
    std::string serverAddress = "127.0.0.1";
    int serverPort = 3000;

    try {
        Socket socket(serverAddress, serverPort);
        socket.Open();
        std::cout << "Connected to the server." << std::endl;

        // Get and send the username
        std::cout << "Enter your username: ";
        std::string username;
        std::getline(std::cin, username);
        ByteArray usernameData(username);
        socket.Write(usernameData);

        while (true) {
            // Get user input
            std::cout << "Enter a message: ";
            std::string message;
            std::getline(std::cin, message);

            // Check for exit condition
            if (message == "done") {
                break;
            }

            // Send the message to the server
            ByteArray data(message);
            socket.Write(data);

            // Read the server's response (if any)
            ByteArray responseData;
            socket.Read(responseData);
            std::cout << "Server response: " << responseData.ToString() << std::endl;
        }

        socket.Close();
        std::cout << "Connection closed." << std::endl;
    } catch (std::string &e) {
        std::cerr << "Error: " << e << std::endl;
    }

    return 0;
}
