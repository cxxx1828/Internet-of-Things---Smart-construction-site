#include <iostream>
#include <fstream>
#include <chrono>
#include <thread>
#include <jsoncpp/json/json.h>
#include <cstdlib>
#include <iomanip>
#include <signal.h>
#include <atomic>

#include "httplib.h"

using namespace std;

// GLOBAL FLAG ZA GRACEFUL SHUTDOWN
std::atomic<bool> keep_running{true}; //flag kontroliše da li petlja radi ili ne
httplib::Server* global_server = nullptr; //pokazuje na HTTP server da ga zaustavimo kad stiže signal

struct EnvironmentState {
    double temperature;
    double heart_rate;
    string emergency_call_active;
    string machine_shutdown_active;
    bool worker_replaced;
    int cycle_count;
};

// SIGNAL HANDLER ZA CTRL+C
void signal_handler(int signal) {
    if (signal == SIGINT || signal == SIGTERM) {
        std::cout << std::endl;
        std::cout << "SHUTDOWN SIGNAL RECEIVED" << std::endl;
        std::cout << "Cleaning up and stopping all processes..." << std::endl;
        
        keep_running = false;
        
        // Stopuj HTTP server
        if (global_server) {
            global_server->stop();
        }
        
        // Obriši JSON fajlove
        std::remove("construction_site.json");
        std::remove("construction_site.json.tmp");
        
        std::cout << "Environment stopped cleanly!" << std::endl;
        std::exit(0);
    }
}

void simulateEnvironment(EnvironmentState& state) {
    int temp_counter = 0;
    int heart_counter = 0;
    state.cycle_count = 0;
   
    while (keep_running) {
        state.cycle_count++;
        
        // TEMPERATURA - redom kroz vrednosti
        double temp_values[] = {34.5, 34.6, 34.7, 34.8, 34.9, 35.1, 35.2, 35.3, 35.4, 35.5,
                                35.6, 35.7, 35.8, 35.9, 36.1, 36.2, 36.3, 36.4, 36.5, 36.6,
                                36.7, 36.8, 36.9, 37.1, 37.2, 37.3, 37.4, 37.5, 37.6, 37.7,
                                37.8, 37.9, 38.1, 38.2, 38.3, 38.4, 38.5, 38.6, 38.7, 38.8,
                                38.9, 39.1, 39.2, 39.3, 39.4, 39.5, 39.6, 39.7, 39.8, 40.0};
        int temp_size = sizeof(temp_values) / sizeof(temp_values[0]);
        
        state.temperature = temp_values[temp_counter % temp_size];
        temp_counter++;
        
        // PULS - redom kroz vrednosti  
        double heart_values[] = {40, 42, 43, 45, 46, 48, 49, 51, 52, 54, 55, 57, 58, 60, 61, 63, 64, 66, 67, 69,
                                70, 72, 73, 75, 77, 78, 80, 81, 83, 84, 86, 88, 89, 91, 92, 94, 95, 97, 98, 100,
                                101, 103, 104, 106, 107, 109, 111, 112, 114, 115, 117, 118, 120, 121, 123, 125, 126, 128,
                                129, 131, 132, 134, 135, 137, 138, 140, 141, 143, 144, 146, 147, 149, 150, 152, 153, 155,
                                156, 158, 159, 161, 162, 164, 165, 167, 168};
        int heart_size = sizeof(heart_values) / sizeof(heart_values[0]);
        
        state.heart_rate = heart_values[heart_counter % heart_size];
        heart_counter++;

        // LOGIKA ALARMA
        state.machine_shutdown_active = "OFF";
        state.emergency_call_active = "OFF";

        bool temp_alarm = (state.temperature >= 38.5 || state.temperature <= 35.0);
        bool heart_alarm = (state.heart_rate >= 105 || state.heart_rate <= 45);

        if (temp_alarm) {
            state.machine_shutdown_active = "ON";
        }

        if (heart_alarm) {
            state.emergency_call_active = "ON";
            state.machine_shutdown_active = "ON";
        }

        // GENERIŠ JSON FAJL - ATOMIC WRITE
        Json::Value root;
        root["temperature"] = state.temperature;
        root["heart_rate"] = state.heart_rate;
        root["machine_shutdown_active"] = state.machine_shutdown_active;
        root["emergency_call_active"] = state.emergency_call_active;

        // SAFE FILE WRITE
        try {
            std::ofstream tempFile("construction_site.json.tmp");
            tempFile << root;
            tempFile.close();
            std::rename("construction_site.json.tmp", "construction_site.json");
        } catch (...) {
            std::cerr << "Warning: Could not write JSON file" << std::endl;
        }
        
        // POBOLJŠAN ISPIS STANJA
        if (keep_running) {
            std::cout << std::endl;
            std::cout << "========================================" << std::endl;
            std::cout << "        ENVIRONMENT CYCLE #" << state.cycle_count << std::endl;
            std::cout << "========================================" << std::endl;
            
            // Temperature status
            std::cout << "Temperature: " << std::fixed << std::setprecision(1) << state.temperature << " °C ";
            if (temp_alarm) {
                std::cout << (state.temperature >= 38.5 ? "HIGH FEVER" : "HYPOTHERMIA");
            } else {
                std::cout << "Normal";
            }
            std::cout << std::endl;
            
            // Heart rate status  
            std::cout << "Heart rate: " << std::fixed << std::setprecision(0) << state.heart_rate << " bpm ";
            if (heart_alarm) {
                std::cout << (state.heart_rate >= 105 ? "TACHYCARDIA" : "BRADYCARDIA");
            } else {
                std::cout << "Normal";
            }
            std::cout << std::endl;
            
            std::cout << "----------------------------------------" << std::endl;
            
            // System status
            std::cout << "Machine shutdown: " << state.machine_shutdown_active;
            if (state.machine_shutdown_active == "ON") {
                std::cout << "MACHINE STOPPED";
            } else {
                std::cout << "Machine running";
            }
            std::cout << std::endl;
            
            std::cout << "Emergency call: " << state.emergency_call_active;
            if (state.emergency_call_active == "ON") {
                std::cout << "EMERGENCY ACTIVE";
            } else {
                std::cout << "No emergency";
            }
            std::cout << std::endl;
            
            std::cout << "========================================" << std::endl;
            
            // Overall system status
            if (heart_alarm) {
                std::cout << "CRITICAL: Heart rate emergency - Worker needs immediate help!" << std::endl;
            } else if (temp_alarm) {
                std::cout << "WARNING: Temperature alarm - Worker should rest!" << std::endl;
            } else {
                std::cout << "NORMAL: All systems operating normally" << std::endl;
            }
            
            std::cout << std::endl;
        }

        // MAIN TIMING - 3 SEKUNDE sa interrupt check
        for (int i = 0; i < 30 && keep_running; ++i) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }
}

void startHttpServer(EnvironmentState& state) {
    httplib::Server svr;
    global_server = &svr;

    svr.Get("/environment", [&state](const httplib::Request& req, httplib::Response& res) {
        std::ifstream file("construction_site.json");
        if (!file.is_open()) {
            res.status = 500;
            res.set_content("Error: cannot open JSON file", "text/plain");
            return;
        }
        
        std::stringstream buffer;
        buffer << file.rdbuf();
        file.close();
        res.set_content(buffer.str(), "application/json");
    });

    svr.Post("/update_relay_state", [&state](const httplib::Request& req, httplib::Response& res) {
        std::string response_msg = "";
        
        if (req.has_param("emergency_call_module")) {
            state.emergency_call_active = req.get_param_value("emergency_call_module");
            response_msg += "Emergency call updated to " + state.emergency_call_active + ". ";
        }
        if (req.has_param("shutdown_relay")) {
            state.machine_shutdown_active = req.get_param_value("shutdown_relay");
            response_msg += "Shutdown relay updated to " + state.machine_shutdown_active + ". ";
        }
        if (response_msg.empty()) {
            response_msg = "Missing parameters.";
        }
        
        res.set_content(response_msg, "text/plain");
    });

    std::cout << "HTTP Server starting on port 8080..." << std::endl;
    std::cout << "Environment simulation will begin in 3 seconds..." << std::endl;
    std::cout << "Press Ctrl+C to stop gracefully" << std::endl;
    std::cout << std::endl;
    
    svr.listen("0.0.0.0", 8080); 
}

int main() {
    // REGISTRUJ SIGNAL HANDLERS
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    
    // OBRIŠI STARE JSON FAJLOVE
    std::remove("construction_site.json");
    std::remove("construction_site.json.tmp");
    
    EnvironmentState environmentState;
    environmentState.temperature = 36.5; 
    environmentState.heart_rate = 75;
    environmentState.emergency_call_active = "OFF";
    environmentState.machine_shutdown_active = "OFF";
    environmentState.worker_replaced = false;
    environmentState.cycle_count = 0;

    std::thread simulation_thread(simulateEnvironment, std::ref(environmentState));
    startHttpServer(environmentState);
    
    simulation_thread.join();
    
    // CLEANUP
    std::remove("construction_site.json");
    std::remove("construction_site.json.tmp");
    
    return 0;
}
