/**
 * POSIX SEMAPHORE CHEAT-SHEET & BOILERPLATE TEMPLATE
 * 
 * Compilation command: 
 *   g++ -pthread student_id.cpp -o solution
 * 
 * Common Header Included:
 *   <pthread.h>    -> pthread_t, pthread_create, pthread_join
 *   <semaphore.h>  -> sem_t, sem_init, sem_wait, sem_post, sem_destroy
 */

#include <iostream>
#include <pthread.h>
#include <semaphore.h>

using namespace std;

// ============================================================================
// GLOBAL STATE & SEMAPHORE DECLARATIONS
// ============================================================================
int N; // Input iteration/line count

// PATTERN A: Mutex / Single Lock State Tracking (e.g., Section A1, B2, C2)
sem_t lock_mutex;
int current_iter = 1;
int countA = 0, countB = 0, countC = 0;

// PATTERN B: Multi-Semaphore Signaling (e.g., Section C1, Producer-Consumer)
sem_t semA, semB, semC, semLine;


// ============================================================================
// THREAD FUNCTIONS
// ============================================================================

/**
 * PATTERN A: Generic Worker Thread using a Single Mutex Lock
 * Use when: Threads print independently and check shared state/counters.
 */
void* genericWorkerPatternA(void* arg) {
    char letter = *(char*)arg;
    
    while (current_iter <= N) {
        // 1. Acquire Critical Section Lock
        sem_wait(&lock_mutex);
        
        // 2. CRITICAL: Re-check exit condition inside the lock!
        // Prevents threads blocked at sem_wait from running extra iterations.
        if (current_iter > N) {
            sem_post(&lock_mutex); // Release lock before exiting to avoid deadlock!
            break;
        }

        // 3. Perform Thread Action (Print / Update counters)
        cout << letter;
        if (letter == 'A') countA++;
        else if (letter == 'B') countB++;
        else if (letter == 'C') countC++;

        // 4. Check Line / Iteration Completion Condition
        // EXAMPLE: Change condition below to match exam requirement
        if (countA >= 1 && countB >= 1 && countC >= 1) {
            cout << " [Iteration " << current_iter << "]" << endl;
            
            // Reset state for next iteration
            countA = 0; countB = 0; countC = 0;
            current_iter++;
        }

        // 5. Release Critical Section Lock
        sem_post(&lock_mutex);
    }
    return NULL;
}


/**
 * PATTERN B: Handshake / Token Coordinator Thread
 * Use when: Strict character quotas exist per line (e.g., 3 A's, 4 B's, 5 C's).
 */
void* coordinatorThreadPatternB(void* arg) {
    for (int line = 1; line <= N; line++) {
        // 1. Wait for total characters to complete for this line
        int TOTAL_CHARS_PER_LINE = 12; // Update according to problem
        for (int j = 0; j < TOTAL_CHARS_PER_LINE; j++) {
            sem_wait(&semLine);
        }

        // 2. Format line ending
        cout << " [Line " << line << "]" << endl;

        // 3. Replenish token permits for worker threads for the next line
        for (int j = 0; j < 3; j++) sem_post(&semA); // Refill A tokens
        for (int j = 0; j < 4; j++) sem_post(&semB); // Refill B tokens
        for (int j = 0; j < 5; j++) sem_post(&semC); // Refill C tokens
    }
    return NULL;
}


// ============================================================================
// MAIN FUNCTION BOILERPLATE
// ============================================================================
int main() {
    // 1. Read Inputs
    if (!(cin >> N)) return 0;

    // 2. Initialize Semaphores
    // sem_init(&sem_variable, pshared=0, initial_value)
    
    // Pattern A Initialization (Binary Mutex Lock starts at 1)
    sem_init(&lock_mutex, 0, 1);

    /* 
    // Pattern B Initialization (Tokens equal allowed count per line)
    sem_init(&semA, 0, 3);
    sem_init(&semB, 0, 4);
    sem_init(&semC, 0, 5);
    sem_init(&semLine, 0, 0); // Accumulator starts at 0
    */

    // 3. Declare Thread Handles & Arguments
    pthread_t t1, t2, t3;
    char pA = 'A', pB = 'B', pC = 'C';

    // 4. Create Threads
    pthread_create(&t1, NULL, genericWorkerPatternA, &pA);
    pthread_create(&t2, NULL, genericWorkerPatternA, &pB);
    pthread_create(&t3, NULL, genericWorkerPatternA, &pC);

    // 5. Join Threads (Wait for all threads to terminate)
    pthread_join(t1, NULL);
    pthread_join(t2, NULL);
    pthread_join(t3, NULL);

    // 6. Destroy Semaphores
    sem_destroy(&lock_mutex);
    /*
    sem_destroy(&semA);
    sem_destroy(&semB);
    sem_destroy(&semC);
    sem_destroy(&semLine);
    */

    return 0;
}