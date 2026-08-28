#include <chrono>
#include <fstream>
#include <iostream>
#include <pthread.h>
#include <random>
#include <semaphore.h>
#include <unistd.h>

using namespace std;

// --- Globals ---
int N, M, x, y;
int num_groups;
auto start_time = chrono::high_resolution_clock::now();

// --- Station sync (4 stations) ---
sem_t station_sem[4];
pthread_mutex_t station_wait_mtx[4];
int station_waiting[4] = {0, 0, 0, 0};

// --- Logbook (Reader-Writer with writer priority) ---
sem_t logbook_resource;
sem_t logbook_readtry;
sem_t logbook_rmutex;
int logbook_readers = 0;
int operations_completed = 0;

// --- Group barrier ---
sem_t leader_done;
pthread_mutex_t barrier_mtx;
int group_members_done[1000] = {0};

// --- Output ---
pthread_mutex_t output_lock;

// ==================== Utility ====================

long long get_time() {
    auto end_time = chrono::high_resolution_clock::now();
    return chrono::duration_cast<chrono::milliseconds>(end_time - start_time).count();
}

int poisson_random(double lambda, mt19937 &gen) {
    poisson_distribution<int> dist(lambda);
    return dist(gen);
}

void write_output(const string &s) {
    pthread_mutex_lock(&output_lock);
    cout << s;
    pthread_mutex_unlock(&output_lock);
}

// ==================== Station Access ====================

void enter_station(int op_id, int station_idx) {
    pthread_mutex_lock(&station_wait_mtx[station_idx]);
    station_waiting[station_idx]++;
    pthread_mutex_unlock(&station_wait_mtx[station_idx]);

    sem_wait(&station_sem[station_idx]);

    write_output("Operative " + to_string(op_id) +
                 " has started typewriting at time " + to_string(get_time()) + "\n");
}

void exit_station(int op_id, int station_idx) {
    pthread_mutex_lock(&station_wait_mtx[station_idx]);
    int waiters = station_waiting[station_idx];
    pthread_mutex_unlock(&station_wait_mtx[station_idx]);

    write_output("Operative " + to_string(op_id) +
                 " has finished typewriting at time " + to_string(get_time()) + "\n");

    sem_post(&station_sem[station_idx]);

    for (int i = 1; i < waiters; i++)
        sem_post(&station_sem[station_idx]);
}

// ==================== Logbook (Reader-Writer with writer priority) ====================
// Prevents writer starvation: once a writer is waiting, new readers block.

void enter_logbook_read() {
    sem_wait(&logbook_readtry);
    sem_wait(&logbook_rmutex);
    logbook_readers++;
    if (logbook_readers == 1)
        sem_wait(&logbook_resource);
    sem_post(&logbook_rmutex);
    sem_post(&logbook_readtry);
}

void exit_logbook_read() {
    sem_wait(&logbook_rmutex);
    logbook_readers--;
    if (logbook_readers == 0)
        sem_post(&logbook_resource);
    sem_post(&logbook_rmutex);
}

void enter_logbook_write() {
    sem_wait(&logbook_readtry);
    sem_wait(&logbook_resource);
}

void exit_logbook_write() {
    sem_post(&logbook_resource);
    sem_post(&logbook_readtry);
}

// ==================== Operative Thread ====================

void *operative_thread(void *arg) {
    int op_id = *(int *)arg;
    int group_id = (op_id - 1) / M;
    int member_idx = (op_id - 1) % M;
    bool is_leader = (member_idx == M - 1);
    int station_idx = (op_id - 1) % 4;

    // Random arrival delay (Poisson)
    random_device rd;
    mt19937 gen(rd());
    int arrival_delay = poisson_random(5.0, gen);
    usleep(arrival_delay * 1000);

    write_output("Operative " + to_string(op_id) +
                 " has arrived at typewriting station at time " + to_string(get_time()) + "\n");

    // Phase 1: Document Recreation
    enter_station(op_id, station_idx);
    usleep(x * 1000);
    exit_station(op_id, station_idx);

    write_output("Operative " + to_string(op_id) +
                 " has completed document recreation at time " + to_string(get_time()) + "\n");

    // Barrier: signal completion
    pthread_mutex_lock(&barrier_mtx);
    group_members_done[group_id]++;
    bool all_done = (group_members_done[group_id] == M);
    pthread_mutex_unlock(&barrier_mtx);

    if (is_leader) {
        // Wait until all M members (including self) have finished
        while (true) {
            pthread_mutex_lock(&barrier_mtx);
            int done = group_members_done[group_id];
            pthread_mutex_unlock(&barrier_mtx);
            if (done >= M) break;
            usleep(100);
        }

        // Phase 2: Logbook Entry (writer)
        write_output("Unit " + to_string(group_id + 1) +
                     " has completed document recreation phase at time " + to_string(get_time()) + "\n");

        enter_logbook_write();
        operations_completed++;
        write_output("Unit " + to_string(group_id + 1) +
                     " has completed intelligence distribution at time " + to_string(get_time()) + "\n");
        exit_logbook_write();

        // Signal remaining members to exit
        for (int i = 0; i < M - 1; i++)
            sem_post(&leader_done);
    } else {
        // Non-leader: signal leader and wait for exit signal
        sem_post(&leader_done);
        sem_wait(&leader_done);
    }

    return NULL;
}

// ==================== Intelligence Staff Thread ====================

void *staff_thread(void *arg) {
    int staff_id = *(int *)arg;
    random_device rd;
    mt19937 gen(rd());

    while (true) {
        int delay = poisson_random(5.0, gen);
        usleep(delay * 1000);

        enter_logbook_read();
        write_output("Intelligence Staff " + to_string(staff_id) +
                     " began reviewing logbook at time " + to_string(get_time()) +
                     ". Operations completed = " + to_string(operations_completed) + "\n");
        usleep(y * 500);
        exit_logbook_read();
    }
    return NULL;
}

// ==================== Main ====================

int main(int argc, char *argv[]) {
    if (argc != 3) {
        cerr << "Usage: ./a.out <input_file> <output_file>" << endl;
        return 1;
    }

    // File I/O redirection
    ifstream inputFile(argv[1]);
    streambuf *cinBuf = cin.rdbuf();
    cin.rdbuf(inputFile.rdbuf());

    ofstream outputFile(argv[2]);
    streambuf *coutBuf = cout.rdbuf();
    cout.rdbuf(outputFile.rdbuf());

    cin >> N >> M >> x >> y;

    num_groups = N / M;
    start_time = chrono::high_resolution_clock::now();

    // Initialize station semaphores
    for (int i = 0; i < 4; i++) {
        sem_init(&station_sem[i], 0, 1);
        pthread_mutex_init(&station_wait_mtx[i], NULL);
    }

    // Initialize logbook sync
    sem_init(&logbook_resource, 0, 1);
    sem_init(&logbook_readtry, 0, 1);
    sem_init(&logbook_rmutex, 0, 1);

    // Initialize group barrier
    sem_init(&leader_done, 0, 0);
    pthread_mutex_init(&barrier_mtx, NULL);

    pthread_mutex_init(&output_lock, NULL);

    // Create operative threads
    pthread_t op_threads[N];
    int op_ids[N];
    for (int i = 0; i < N; i++) {
        op_ids[i] = i + 1;
        pthread_create(&op_threads[i], NULL, operative_thread, &op_ids[i]);
    }

    // Create 2 intelligence staff threads
    pthread_t staff_threads[2];
    int staff_ids[2] = {1, 2};
    for (int i = 0; i < 2; i++)
        pthread_create(&staff_threads[i], NULL, staff_thread, &staff_ids[i]);

    // Join operative threads
    for (int i = 0; i < N; i++)
        pthread_join(op_threads[i], NULL);

    // Cleanup
    for (int i = 0; i < 4; i++) {
        sem_destroy(&station_sem[i]);
        pthread_mutex_destroy(&station_wait_mtx[i]);
    }
    sem_destroy(&logbook_resource);
    sem_destroy(&logbook_readtry);
    sem_destroy(&logbook_rmutex);
    sem_destroy(&leader_done);
    pthread_mutex_destroy(&barrier_mtx);
    pthread_mutex_destroy(&output_lock);

    cin.rdbuf(cinBuf);
    cout.rdbuf(coutBuf);

    return 0;
}
