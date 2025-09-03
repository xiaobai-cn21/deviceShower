#include <iostream>
#include <cstdlib>
#include <ctime>
#include <unistd.h>
#include <arpa/inet.h>
#include <string>

int main() {
    int server_fd, new_socket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);

    // Create socket file descriptor
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // Forcefully attaching socket to port 8080
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt));

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY; // listen on all interfaces
    address.sin_port = htons(8081);

    // Bind
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    // Listen
    if (listen(server_fd, 3) < 0) {
        perror("listen failed");
        exit(EXIT_FAILURE);
    }

    std::cout << "Server listening on port 8080...\n";

    // Accept a client
    new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen);
    if (new_socket < 0) {
        perror("accept failed");
        exit(EXIT_FAILURE);
    }

    srand(time(NULL));

    while (true) {
        float temperature = (rand() % 4000) / 100.0; // 0.00 to 40.00
        float pressure = 950.0 + (rand() % 1000) / 10.0; // 950.0 to 1050.0

        std::string message = std::to_string(temperature) + "," + std::to_string(pressure) + "\n";

        send(new_socket, message.c_str(), message.size(), 0);
        std::cout << "Sent: " << message;
        sleep(1); // 1 second
    }

    close(new_socket);
    close(server_fd);
    return 0;
}
