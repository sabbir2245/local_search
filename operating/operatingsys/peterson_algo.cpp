#include <iostream>
#include <thread>
#include <chrono>
#include <atomic>
#include <mutex>
#include <iomanip>

using namespace std;
using namespace chrono;

// Peterson's Algorithm variables
volatile int turn;
volatile bool interested[2] = {false, false};

// Shared resource
volatile int shared_counter = 0;

// For clean output
mutex print_mutex;

void enter_region(int process) {
    int other = 1 - process;
    interested[process] = true;
    turn = process;
    
    // Spinlock - wait until it's our turn
    while (turn == process && interested[other] == true) {
        // Busy wait (spin)
        this_thread::sleep_for(milliseconds(10)); // Small sleep to prevent CPU hogging
    }
}

void leave_region(int process) {
    interested[process] = false;
}

void process_work(int process_id) {
    string process_name = "Process " + to_string(process_id);
    
    for (int iteration = 0; iteration < 3; iteration++) {
        // ENTER CRITICAL SECTION
        enter_region(process_id);
        
        // --- CRITICAL SECTION START ---
        {
            lock_guard<mutex> lock(print_mutex);
            cout << "\n╔═══════════════════════════════════════════╗\n";
            cout << "║ " << process_name << " ENTERED critical section ║\n";
            cout << "╚═══════════════════════════════════════════╝\n";
            
            cout << "  Before: shared_counter = " << shared_counter << endl;
        }
        
        // Simulate work with 2-second delay
        int local_copy = shared_counter;
        this_thread::sleep_for(seconds(2)); // 2 second delay
        shared_counter = local_copy + 1;
        
        {
            lock_guard<mutex> lock(print_mutex);
            cout << "  After:  shared_counter = " << shared_counter << endl;
            cout << "  " << process_name << " leaving...\n\n";
        }
        
        // --- CRITICAL SECTION END ---
        leave_region(process_id);
        
        // Do some non-critical work outside
        this_thread::sleep_for(milliseconds(500));
    }
}

int main() {
    cout << "═══════════════════════════════════════════════════════\n";
    cout << "  PETERSON'S ALGORITHM DEMONSTRATION\n";
    cout << "  Two processes increment a shared counter\n";
    cout << "  Each critical section has a 2-second delay\n";
    cout << "═══════════════════════════════════════════════════════\n\n";
    
    cout << "Initial shared_counter = 0\n";
    cout << "Starting both processes...\n\n";
    
    // Create two threads
    thread process0(process_work, 0);
    thread process1(process_work, 1);
    
    // Wait for both to finish
    process0.join();
    process1.join();
    
    cout << "\n═══════════════════════════════════════════════════════\n";
    cout << "  FINAL RESULT: shared_counter = " << shared_counter << endl;
    cout << "  Expected: 6 (3 iterations × 2 processes)\n";
    cout << "═══════════════════════════════════════════════════════\n";
    
    return 0;
}