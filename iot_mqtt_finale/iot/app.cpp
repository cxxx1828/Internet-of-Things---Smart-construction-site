#include <iostream>
#include "httplib.h" 

int main() {
    httplib::Client client("http://localhost:8080");
    
    auto res = client.Get("/environment");
    
    if (res && res->status == 200) {
        
        std::cout << std::endl;
        std::cout << "Current environment status:\n" << res->body << std::endl;
        std::cout << std::endl;
    } else {
        std::cout << std::endl;
        std::cerr << "Error: Failed to retrieve status.\n";
        std::cout << std::endl;
        return 1;
    }
    return 0;
}

