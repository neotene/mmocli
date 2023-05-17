#include <iostream>

#include "utils.hpp"

int main(int argc, char**argv, char**)
{
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <email> <password>" << std::endl;
        return EXIT_FAILURE;
    }

    std::string email = argv[1];
    std::string password = argv[2];

    try {
        std::string response = send_register(email, password);
        std::cout << response << std::endl;
    } catch (std::exception const& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }
}
