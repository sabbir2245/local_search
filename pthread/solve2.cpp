/*
    CSE 314 - Assignment 3 (IPC): The Shadows of Small Heath

    N operatives are split into groups of M. Each operative:
      1) waits randomly, then visits one of 4 typewriting stations
         (only ONE operative may use a station at a time)
      2) once ALL M members of a group are done, the group's LEADER
         (highest id in the group) writes one entry into a shared
         logbook. Two "staff" threads keep reading the logbook forever.
         Readers can share access, but a writer needs exclusive access.
         (Reader-priority version, as required by the assignment rubric.)

    Build:  g++ -pthread -std=c++17 solve.cpp -o solve.out
    Run:    ./solve.out input.txt output.txt
*/

#include <chrono>
#include <fstream>
#include <iostream>
#include <pthread.h>
#include <random>
#include <semaphore.h>
#include <unistd.h>
#include <vector>

using namespace std;

// ---------------------------------------------------------------------
// Global simulation parameters (read from the input file)
// ---------------------------------------------------------------------
int N;              // number of operatives
int M;              // group size
int x, y;           // relative time units for the two phases
int num_groups;     // = N / M

chrono::high_resolution_clock::time_point start_time;

// ---------------------------------------------------------------------
// Typewriting stations (Task 1)
// ---------------------------------------------------------------------
// There are always exactly 4 stations. Each station is a binary
// semaphore: 1 = free, 0 = occupied. Whoever calls sem_wait() first
// (in arrival order, since Linux wakes waiters in FIFO order) gets in.
// No extra bookkeeping / busy waiting is needed for this part.
const int NUM_STATIONS = 4;
sem_t station_sem[NUM_STATIONS];

// ---------------------------------------------------------------------
// Logbook (Task 2) - Reader-Priority Readers/Writers problem
// ---------------------------------------------------------------------
// logbook_mutex  : protects the "reader_count" variable
// logbook_access : held by a writer while writing, OR by the very
//                  first reader (on behalf of all current readers)
sem_t logbook_mutex;
sem_t logbook_access;
int   reader_count = 0;
int   operations_completed = 0;

// ---------------------------------------------------------------------
// Group barrier: "wait until all M members of my group finished
// phase 1", implemented with a semaphore instead of a polling loop.
// ---------------------------------------------------------------------
pthread_mutex_t barrier_mutex;
vector<int> group_members_done;   // members_done[g] = how many have finished
vector<sem_t> group_barrier_sem;  // one semaphore per group, posted by
                                   // every member, waited on M times by leader

// A leader tells the non-leaders "you may leave" through this
// per-group semaphore.
vector<sem_t> leader_release_sem;

// ---------------------------------------------------------------------
// Output: all threads share stdout/cout, so writes must be serialized
// or lines will interleave and become unreadable.
// ---------------------------------------------------------------------
pthread_mutex_t output_mutex;

void write_output(const string &line) {
    pthread_mutex_lock(&output_mutex);
    cout << line;
    pthread_mutex_unlock(&output_mutex);
}

// ---------------------------------------------------------------------
// Small helpers
// ---------------------------------------------------------------------
long long elapsed_ms() {
    auto now = chrono::high_resolution_clock::now();
    return chrono::duration_cast<chrono::milliseconds>(now - start_time).count();
}

// One random-number generator per call is fine at this scale and
// keeps the code simple to read.
int poisson_random(double lambda) {
    random_device rd;
    mt19937 gen(rd());
    poisson_distribution<int> dist(lambda);
    return dist(gen);
}

// ---------------------------------------------------------------------
// Station access (Task 1)
// ---------------------------------------------------------------------
void enter_station(int op_id, int station_idx) {
    sem_wait(&station_sem[station_idx]);   // blocks (no busy wait) until free
    write_output("Operative " + to_string(op_id) +
                 " has started typewriting at time " + to_string(elapsed_ms()) + "\n");
}

void exit_station(int op_id, int station_idx) {
    write_output("Operative " + to_string(op_id) +
                 " has finished typewriting at time " + to_string(elapsed_ms()) + "\n");
    sem_post(&station_sem[station_idx]);   // wakes exactly the next waiter, if any
}

// ---------------------------------------------------------------------
// Logbook access (Task 2) - reader priority
// ---------------------------------------------------------------------
void start_reading_logbook() {
    sem_wait(&logbook_mutex);
    reader_count++;
    if (reader_count == 1) {
        // first reader locks out any writer
        sem_wait(&logbook_access);
    }
    sem_post(&logbook_mutex);
}

void stop_reading_logbook() {
    sem_wait(&logbook_mutex);
    reader_count--;
    if (reader_count == 0) {
        // last reader lets a writer in
        sem_post(&logbook_access);
    }
    sem_post(&logbook_mutex);
}

void start_writing_logbook() {
    sem_wait(&logbook_access);   // exclusive access
}

void stop_writing_logbook() {
    sem_post(&logbook_access);
}

// ---------------------------------------------------------------------
// Operative thread
// ---------------------------------------------------------------------
void *operative_thread(void *arg) {
    int op_id = *(int *)arg;

    int group_id    = (op_id - 1) / M;          // 0-indexed group number
    int member_idx  = (op_id - 1) % M;          // position inside the group
    bool is_leader  = (member_idx == M - 1);    // highest id in the group leads
    int station_idx = (op_id - 1) % NUM_STATIONS;

    // Random arrival delay so operatives don't all start at once.
    int arrival_delay_ms = poisson_random(5.0);
    usleep(arrival_delay_ms * 1000);

    write_output("Operative " + to_string(op_id) +
                 " has arrived at typewriting station at time " + to_string(elapsed_ms()) + "\n");

    // ---- Phase 1: Document Recreation ----
    enter_station(op_id, station_idx);
    usleep(x * 1000);
    exit_station(op_id, station_idx);

    write_output("Operative " + to_string(op_id) +
                 " has completed document recreation at time " + to_string(elapsed_ms()) + "\n");

    // Tell the group "I'm done with phase 1"
    pthread_mutex_lock(&barrier_mutex);
    group_members_done[group_id]++;
    pthread_mutex_unlock(&barrier_mutex);
    sem_post(&group_barrier_sem[group_id]);

    if (is_leader) {
        // Wait for all M members (including myself) to arrive here.
        // sem_wait blocks instead of polling -> no busy waiting.
        for (int i = 0; i < M; i++)
            sem_wait(&group_barrier_sem[group_id]);

        write_output("Unit " + to_string(group_id + 1) +
                     " has completed document recreation phase at time " + to_string(elapsed_ms()) + "\n");

        // ---- Phase 2: Logbook Entry ----
        start_writing_logbook();
        usleep(y * 1000);
        operations_completed++;
        write_output("Unit " + to_string(group_id + 1) +
                     " has completed intelligence distribution at time " + to_string(elapsed_ms()) + "\n");
        stop_writing_logbook();

        // Let the other M-1 members of the group finish/exit.
        for (int i = 0; i < M - 1; i++)
            sem_post(&leader_release_sem[group_id]);
    } else {
        // Non-leaders just wait for the leader to finish the logbook entry.
        sem_wait(&leader_release_sem[group_id]);
    }

    return nullptr;
}

// ---------------------------------------------------------------------
// Intelligence staff thread (keeps reading the logbook forever)
// ---------------------------------------------------------------------
void *staff_thread(void *arg) {
    int staff_id = *(int *)arg;

    while (true) {
        int delay_ms = poisson_random(5.0);
        usleep(delay_ms * 1000);

        start_reading_logbook();
        write_output("Intelligence Staff " + to_string(staff_id) +
                     " began reviewing logbook at time " + to_string(elapsed_ms()) +
                     ". Operations completed = " + to_string(operations_completed) + "\n");
        usleep(y * 500);   // staff spend some time reading too
        stop_reading_logbook();
    }
    return nullptr;
}

// ---------------------------------------------------------------------
// main
// ---------------------------------------------------------------------
int main(int argc, char *argv[]) {
    if (argc != 3) {
        cerr << "Usage: ./solve.out <input_file> <output_file>" << endl;
        return 1;
    }

    // Redirect cin/cout to the given files.
    ifstream inputFile(argv[1]);
    streambuf *cinBuf = cin.rdbuf();
    cin.rdbuf(inputFile.rdbuf());

    ofstream outputFile(argv[2]);
    streambuf *coutBuf = cout.rdbuf();
    cout.rdbuf(outputFile.rdbuf());

    cin >> N >> M >> x >> y;
    num_groups = N / M;

    start_time = chrono::high_resolution_clock::now();

    // --- init synchronization primitives ---
    for (int i = 0; i < NUM_STATIONS; i++)
        sem_init(&station_sem[i], 0, 1);   // 1 = free

    sem_init(&logbook_mutex, 0, 1);
    sem_init(&logbook_access, 0, 1);

    pthread_mutex_init(&barrier_mutex, NULL);
    pthread_mutex_init(&output_mutex, NULL);

    group_members_done.assign(num_groups, 0);
    group_barrier_sem.resize(num_groups);
    leader_release_sem.resize(num_groups);
    for (int g = 0; g < num_groups; g++) {
        sem_init(&group_barrier_sem[g], 0, 0);
        sem_init(&leader_release_sem[g], 0, 0);
    }

    // --- create operative threads (all at once; random delay happens inside) ---
    vector<pthread_t> op_threads(N);
    vector<int> op_ids(N);
    for (int i = 0; i < N; i++) {
        op_ids[i] = i + 1;
        pthread_create(&op_threads[i], NULL, operative_thread, &op_ids[i]);
    }

    // --- create the 2 intelligence staff threads (run forever) ---
    pthread_t staff_threads[2];
    int staff_ids[2] = {1, 2};
    for (int i = 0; i < 2; i++)
        pthread_create(&staff_threads[i], NULL, staff_thread, &staff_ids[i]);

    // --- wait for all operatives to finish ---
    for (int i = 0; i < N; i++)
        pthread_join(op_threads[i], NULL);

    // Staff threads never finish on their own (they loop forever),
    // so we just stop the program here; that's fine since main()
    // exiting ends the whole process.

    // --- cleanup ---
    for (int i = 0; i < NUM_STATIONS; i++)
        sem_destroy(&station_sem[i]);
    sem_destroy(&logbook_mutex);
    sem_destroy(&logbook_access);
    pthread_mutex_destroy(&barrier_mutex);
    pthread_mutex_destroy(&output_mutex);
    for (int g = 0; g < num_groups; g++) {
        sem_destroy(&group_barrier_sem[g]);
        sem_destroy(&leader_release_sem[g]);
    }

    cin.rdbuf(cinBuf);
    cout.rdbuf(coutBuf);
    return 0;
}